#include "Util/MiscUtil.h"
#include "Nameplate.h"

namespace nameplate
{

static constexpr RGBA WHITE32 = { 255, 255, 255 };
static constexpr RGBA BLACK32 = { };
static constexpr RGB WHITE24 = { 255, 255, 255 };
static constexpr RGB BLACK24 = { };

static constexpr int BUTTON_WIDTH = 125;
static constexpr int BUTTON_HEIGHT = 50;

constexpr unsigned int NAME_FONT_SIZE = 400;


void RFIDThread(RFID* rfid, uint32_t& id, bool& end)
{
    end = false;
    id = rfid->GetId(); // blocks
    end = true;
}

Nameplate::Nameplate(const PlatformConfig<TCPNetworkConfig>& config)
    : m_frontDisplay(PlatformFactory::CreateDisplay(config.displayWidth, config.displayHeight, "nameplate_front")),
    m_rearDisplay(PlatformFactory::CreateDisplay(config.displayWidth, config.displayHeight, "nameplate_rear")),
    m_network(PlatformFactory::CreateNetwork<TCPNetworkConfig>(config.networkConfig)),
    m_card(PlatformFactory::CreateRFID(config.serialPort, config.serialBaudRate)),
    m_touch(dynamic_cast<Touch*>(m_rearDisplay.get())), // Yet another area where my attempts to keep this ultra potrable kinda fails
    m_keyboard(2, config.displayHeight - (VirtualKeyboard::LAYOUT_ROWS * (config.displayWidth / 20)) - config.displayWidth / 200 - 12, 
        config.displayWidth / 20, config.displayWidth / 20, config.displayWidth / 200, 2
    ),
    m_currentState(State::Idle), m_stateTransition(true), m_readId(false), m_currentId(0), m_currentStudent(),
    m_currentQuietMode(QuietMode::Off), m_anonymousMode(AnonymousMode::Off),
    m_reactionSelected(Reaction::None), m_reactionSent(false), m_raiseHandTime(), m_frontForeground(BLACK24), m_frontBackground(WHITE24),
    m_numPollOptions(0), m_selectedPollOption(-1),
    m_cardThread()
{
    m_network->SubscribeToPacket(PacketType::StudentInfo, [this](Message& msg) {        
        uint16_t firstNameSize;
        msg.Pop(&firstNameSize, sizeof(firstNameSize), sizeof(firstNameSize));

        uint16_t lastNameSize;
        msg.Pop(&lastNameSize, sizeof(lastNameSize), sizeof(lastNameSize));

        // Pop strings into a vector
        std::vector<uint8_t> buf(firstNameSize + lastNameSize);
        msg.Pop(buf.data(), buf.size(), firstNameSize + lastNameSize);

        const char* lastName = reinterpret_cast<const char*>(&buf[0]);
        const char* firstName = reinterpret_cast<const char*>(&buf[lastNameSize]);

        uint32_t studentId;
        msg.Pop(&studentId, sizeof(uint32_t), sizeof(studentId));

        m_currentStudent = Student(studentId, lastName, firstName);

        m_currentState = State::Name;
        m_stateTransition = true;
    });


    m_network->SubscribeToPacket(PacketType::StudentNotFound, [this](Message& msg) {
        m_currentState = State::CreateStudentLastName;
        m_stateTransition = true;
    });

    m_network->SubscribeToPacket(PacketType::StartPoll, [this](Message& msg) {
        if (m_currentState == State::Name || m_currentState == State::Poll)
        {
            msg.Pop(&m_numPollOptions, sizeof(m_numPollOptions), sizeof(m_numPollOptions));
            m_currentState = State::Poll;
            m_stateTransition = true;
        }
    });

    m_network->SubscribeToPacket(PacketType::EndPoll, [this](Message& msg) {
        if (m_currentState == State::Poll)
        {
            m_currentState = State::Name;
            m_stateTransition = true;
        }
    });

    m_network->SubscribeToPacket(PacketType::QuiteMode, [this](Message& msg) {
        QuietMode mode;
        msg.Pop(&mode, sizeof(mode), sizeof(mode));
        m_currentQuietMode = mode;
        INFO("[Nameplate] Quiet mode set to: " + std::to_string(static_cast<int>(mode)));

        if (m_currentQuietMode != QuietMode::Off && m_reactionSelected != Reaction::None)
        {
            ClearReaction();
        }
    });

    m_network->SubscribeToPacket(PacketType::AnonymousMode, [this](Message& msg) {
        AnonymousMode mode;
        msg.Pop(&mode, sizeof(mode), sizeof(mode));
        m_anonymousMode = mode;
        INFO("[Nameplate] Anonymous mode set to: " + std::to_string(static_cast<int>(mode)));
    });

    m_network->SubscribeToPacket(PacketType::ClearReaction, [this](Message& msg) {
        INFO("[Nameplate] Network request to clear reactions");
        ClearReaction();
    });

}

void Nameplate::Run()
{
    while (true)
    {
        m_frontDisplay->HandleEvents();
        m_rearDisplay->HandleEvents();

        m_network->HandleMessages();

        m_frontDisplay->Clear(m_frontBackground);
        m_rearDisplay->Clear(WHITE24);

        if (m_stateTransition)
        {
            m_stateHandler.at(m_currentState).init();
            m_stateTransition = false;
        }

        m_stateHandler.at(m_currentState).periodic();

        
        m_frontDisplay->Show();
        m_rearDisplay->Show();
    }
    
}

void Nameplate::IdleStateInit()
{
    if (m_card)
    {
        m_cardThread = std::make_unique<std::thread>(RFIDThread, m_card.get(), std::ref(m_currentId), std::ref(m_readId));
    }
}

void Nameplate::IdleStatePeriodic()
{
    const unsigned int centerX = m_frontDisplay->Width() / 2;
    const unsigned int centerY = m_frontDisplay->Height() / 2;
    
    m_rearDisplay->DrawText(centerX, centerY, 20, BLACK32, "Tap Student ID card to begin");

    if (m_readId)
    {
        if (m_cardThread->joinable())
        {
            m_cardThread->join();
            m_readId = false;
        }


        // Send Student ID message
        Message studentIdMessage(PacketType::StudentId, m_network->ClientId());
        studentIdMessage.Push(&m_currentId, sizeof(uint32_t));

        m_network->SendToServer(studentIdMessage);

        LOG_DEBUG("[Nameplate] RFID: " + std::to_string(m_currentId));
    }
}

void Nameplate::NameStateInit()
{
    INFO("[Namplate] Name state init");
}

void Nameplate::NameStatePeriodic()
{
    const unsigned int centerX = m_frontDisplay->Width() / 2;
    const unsigned int centerY = m_frontDisplay->Height() / 2;

    const std::string fullName = std::string(m_currentStudent.firstName) + ' ' + std::string(m_currentStudent.lastName);

    m_frontDisplay->DrawText(centerX, centerY, NAME_FONT_SIZE, m_frontForeground, m_currentStudent.firstName);
    m_rearDisplay->DrawText(centerX, centerY, NAME_FONT_SIZE / 3, BLACK32, fullName);

    const float signOutBtnX = m_rearDisplay->Width() - BUTTON_WIDTH - 50;
    const float buttonY = m_rearDisplay->Height() - BUTTON_HEIGHT - 60;

    m_rearDisplay->FillRectangle(
        signOutBtnX, buttonY,
        BUTTON_WIDTH, BUTTON_HEIGHT, WHITE32, BLACK32, 2
    );
    m_rearDisplay->DrawText(signOutBtnX + (BUTTON_WIDTH / 2), buttonY + (BUTTON_HEIGHT / 2), 20, BLACK32, "Sign Out");

    const auto touchPos = m_touch->GetTouchPos();
    const bool overlap = RectOverlapTest(signOutBtnX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, touchPos.first, touchPos.second, 1, 1);

    if (overlap && m_touch->IsTouched())
    {
        // Clear existing reaction
        if (m_reactionSelected != Reaction::None)
        {
            ClearReaction();
        }
        
        Message msg(PacketType::LeaveClass, m_network->ClientId());
        msg.Push(&m_currentStudent.id, sizeof(m_currentStudent.id));
        m_network->SendToServer(msg);

        m_currentState = State::Idle;
        m_stateTransition = true;
    }

    const int thumbsUpX = 50;
    const int thumbsDownX = thumbsUpX + 200;
    const int raiseHandX = thumbsDownX + 200;

    const int reactionY = m_rearDisplay->Height() - 200;

    const int thumbsUpWidth = m_rearDisplay->ReactionWidth(Reaction::ThumbsUp);
    const int thumbsUpHeight = m_rearDisplay->ReactionHeight(Reaction::ThumbsUp);
    const bool thumbsUpOverlap = RectOverlapTest(thumbsUpX, reactionY, thumbsUpWidth, thumbsUpHeight, touchPos.first, touchPos.second, 1, 1);

    const int thumbsDownWidth = m_rearDisplay->ReactionWidth(Reaction::ThumbsDown);
    const int thumbsDownHeight = m_rearDisplay->ReactionHeight(Reaction::ThumbsDown);
    const bool thumbsDownOverlap = RectOverlapTest(thumbsDownX, reactionY, thumbsDownWidth, thumbsDownHeight, touchPos.first, touchPos.second, 1, 1);

    const int raiseHandWidth = m_rearDisplay->ReactionWidth(Reaction::RaiseHand);
    const int raiseHandHeight = m_rearDisplay->ReactionHeight(Reaction::RaiseHand);
    const bool raiseHandOverlap = RectOverlapTest(raiseHandX, reactionY, raiseHandWidth, raiseHandHeight, touchPos.first, touchPos.second, 1, 1);


    if (m_reactionSelected == Reaction::ThumbsUp || (thumbsUpOverlap && m_touch->IsTouched()) && m_currentQuietMode == QuietMode::Off)
    {
        if (!m_reactionSent)
        {
            LOG_DEBUG("[Nameplate] thumbs up reaction sent");
            Message msg(PacketType::SetReaction, m_network->ClientId());
            const Reaction r = Reaction::ThumbsUp;
            msg.Push(&r, sizeof(r));
            m_network->SendToServer(msg);

            m_reactionSent = true;
        }

        m_rearDisplay->FillRectangle(thumbsUpX, reactionY, thumbsUpWidth, thumbsUpHeight, WHITE32, BLACK32, 2);
        m_reactionSelected = Reaction::ThumbsUp;
        m_frontDisplay->DrawReaction(m_rearDisplay->Width() - 150, 25, Reaction::ThumbsUp);
    }

    if (m_reactionSelected == Reaction::ThumbsDown || (thumbsDownOverlap && m_touch->IsTouched()) && m_currentQuietMode == QuietMode::Off)
    {
        if (!m_reactionSent)
        {
            LOG_DEBUG("[Nameplate] thumbs down reeaction sent");
            Message msg(PacketType::SetReaction, m_network->ClientId());
            const Reaction r = Reaction::ThumbsDown;
            msg.Push(&r, sizeof(r));
            m_network->SendToServer(msg);

            m_reactionSent = true;
        }

        m_rearDisplay->FillRectangle(thumbsDownX, reactionY, thumbsDownWidth, thumbsDownHeight, WHITE32, BLACK32, 2);
        m_reactionSelected = Reaction::ThumbsDown;
        
        m_frontDisplay->DrawReaction(m_rearDisplay->Width() - 150, 25, Reaction::ThumbsDown);
    }

    if (m_reactionSelected == Reaction::RaiseHand || (raiseHandOverlap && m_touch->IsTouched()) && m_currentQuietMode == QuietMode::Off)
    {
        if (!m_reactionSent)
        {
            LOG_DEBUG("[Nameplate] raise hand reeaction sent");
            Message msg(PacketType::SetReaction, m_network->ClientId());
            const Reaction r = Reaction::RaiseHand;
            msg.Push(&r, sizeof(r));
            m_network->SendToServer(msg);

            m_raiseHandTime.Mark();
            m_reactionSent = true;
        }

        m_rearDisplay->FillRectangle(raiseHandX, reactionY, raiseHandWidth, raiseHandHeight, WHITE32, BLACK32, 2);
        m_reactionSelected = Reaction::RaiseHand;

        if (m_raiseHandTime.Elapsed() > 1.0f)
        {
            m_raiseHandTime.Mark();
            if (m_frontForeground == WHITE24)
            {
                m_frontForeground = BLACK24;
                m_frontBackground = WHITE24;
            }
            else if (m_frontForeground == BLACK24)
            {
                m_frontForeground = WHITE24;
                m_frontBackground = BLACK24;
            }
        }

    }

    if (m_currentQuietMode == QuietMode::Off)
    {
        m_rearDisplay->DrawReaction(thumbsUpX, reactionY, Reaction::ThumbsUp);
        m_rearDisplay->DrawReaction(thumbsDownX, reactionY, Reaction::ThumbsDown);
        m_rearDisplay->DrawReaction(raiseHandX, reactionY, Reaction::RaiseHand);
    }

    const int clearReactionsBtnX = signOutBtnX - 1000;

    if (m_reactionSelected != Reaction::None)
    {
        m_rearDisplay->FillRectangle(
            clearReactionsBtnX, buttonY,
            BUTTON_WIDTH, BUTTON_HEIGHT, WHITE32, BLACK32, 2
        );
        const bool clearRxnOverlap = RectOverlapTest(clearReactionsBtnX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, touchPos.first, touchPos.second, 1, 1);

        if (clearRxnOverlap && m_touch->IsTouched())
        {
            ClearReaction();
        }

        m_rearDisplay->DrawText(clearReactionsBtnX + (BUTTON_WIDTH / 2), buttonY + (BUTTON_HEIGHT / 2), 20, BLACK32, "Clear");
    }
}

void Nameplate::CreateStudentLastNameInit()
{
    INFO("[Nameplate] Create student last name init");
    m_keyboard.ClearText();
}

void Nameplate::CreateStudentLastNamePeriodic()
{
    m_keyboard.Draw(*m_rearDisplay, WHITE32, BLACK32);
    m_keyboard.Update(*m_touch);

    m_rearDisplay->DrawText(100, 50, 20, BLACK32, "Last Name:");

    m_rearDisplay->DrawText(m_rearDisplay->Width() / 2, m_rearDisplay->Height() / 2, 20, BLACK32, m_keyboard.Text());

    const float buttonX = m_rearDisplay->Width() - BUTTON_WIDTH - 10;
    const float buttonY = m_rearDisplay->Height() - BUTTON_HEIGHT - 10;

    m_rearDisplay->FillRectangle(
        buttonX, buttonY,
        BUTTON_WIDTH, BUTTON_HEIGHT, WHITE32, BLACK32, 2
    );


    m_rearDisplay->DrawText(buttonX + (BUTTON_WIDTH / 2), buttonY + (BUTTON_HEIGHT / 2), 20, BLACK32, "Ok");

    const auto touchPos = m_touch->GetTouchPos();

    const bool overlap = RectOverlapTest(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, touchPos.first, touchPos.second, 1, 1);
    if (overlap && m_touch->IsTouched() && !m_keyboard.Text().empty())
    {
        m_inputLastName = m_keyboard.Text();
        m_currentState = State::CreateStudentFirstName;
        m_stateTransition = true;
    }
}

void Nameplate::CreateStudentFirstNameInit()
{
    INFO("[Nameplate] Create student first name init");
    m_keyboard.ClearText();
}

void Nameplate::CreateStudentFirstNamePeriodic()
{
    m_keyboard.Draw(*m_rearDisplay, WHITE32, BLACK32);
    m_keyboard.Update(*m_touch);

    m_rearDisplay->DrawText(100, 50, 20, BLACK32, "First Name:");

    m_rearDisplay->DrawText(m_rearDisplay->Width() / 2, m_rearDisplay->Height() / 2, 20, BLACK32, m_keyboard.Text());

    const float buttonX = m_rearDisplay->Width() - BUTTON_WIDTH - 10;
    const float buttonY = m_rearDisplay->Height() - BUTTON_HEIGHT - 10;

    m_rearDisplay->FillRectangle(
        buttonX, buttonY,
        BUTTON_WIDTH, BUTTON_HEIGHT, WHITE32, BLACK32, 2
    );


    m_rearDisplay->DrawText(buttonX + (BUTTON_WIDTH / 2), buttonY + (BUTTON_HEIGHT / 2), 20, BLACK32, "Ok");

    const auto touchPos = m_touch->GetTouchPos();

    const bool overlap = RectOverlapTest(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, touchPos.first, touchPos.second, 1, 1);
    if (overlap && m_touch->IsTouched() && !m_keyboard.Text().empty())
    {
        m_inputFirstName = m_keyboard.Text();

        Message resp(PacketType::StudentInfo, m_network->ClientId());
        // 4 bytes of length info for the packet (2 per string)
        const uint16_t lastNameBufSz = static_cast<uint16_t>(m_inputLastName.length() + 1);
        const uint16_t firstNameBufSz = static_cast<uint16_t>(m_inputFirstName.length() + 1);

        resp.Push(&m_currentId, sizeof(m_currentId));
        resp.Push(m_inputLastName.c_str(), lastNameBufSz);
        resp.Push(m_inputFirstName.c_str(), firstNameBufSz);
        resp.Push(&lastNameBufSz, sizeof(lastNameBufSz));
        resp.Push(&firstNameBufSz, sizeof(firstNameBufSz));

        m_network->SendToServer(resp);
    }
}

void Nameplate::PollStateInit()
{
    INFO("[Nameplate] poll state init");
    m_selectedPollOption = -1;
}

void Nameplate::PollStatePeriodic()
{
    constexpr unsigned int LETTER_FONT_SIZE = 106;

    constexpr RGBA RED_FILL = RGBA(0xce, 0x37, 0x46);
    constexpr RGBA BLUE_FILL = RGBA(0x35, 0x69, 0xc4);
    constexpr RGBA YELLOW_FILL = RGBA(0xce, 0x9f, 0x40);
    constexpr RGBA GREEN_FILL = RGBA(0x48, 0x86, 0x2f);


    const unsigned int width = m_rearDisplay->Width();
    const unsigned int height = m_rearDisplay->Height();

    switch (m_selectedPollOption)
    {
    case 1:
        m_rearDisplay->FillRectangle(0.0f, 0.0f, width, height, RED_FILL, BLACK32, 0);
        m_rearDisplay->DrawText(width / 2, height / 2, LETTER_FONT_SIZE, WHITE32, "A");

        if (m_anonymousMode == AnonymousMode::Off)
        {
            m_frontDisplay->FillRectangle(0.0f, 0.0f, width, height, RED_FILL, BLACK32, 0);
            m_frontDisplay->DrawText(width / 2, height / 2, NAME_FONT_SIZE, WHITE32, m_currentStudent.firstName);
            m_frontDisplay->DrawText(width - 50, height - 50, LETTER_FONT_SIZE, WHITE32, "A");
            return;
        }
        
        m_frontDisplay->DrawText(width / 2, height / 2, NAME_FONT_SIZE, BLACK32, m_currentStudent.firstName);
        return;
    case 2:
        m_rearDisplay->FillRectangle(0.0f, 0.0f, width, height, BLUE_FILL, BLACK32, 0);
        m_rearDisplay->DrawText(width / 2, height / 2, LETTER_FONT_SIZE, WHITE32, "B");

        if (m_anonymousMode == AnonymousMode::Off)
        {
            m_frontDisplay->FillRectangle(0.0f, 0.0f, width, height, BLUE_FILL, BLACK32, 0);
            m_frontDisplay->DrawText(width / 2, height / 2, NAME_FONT_SIZE, WHITE32, m_currentStudent.firstName);
            m_frontDisplay->DrawText(width - 50, height - 50, LETTER_FONT_SIZE, WHITE32, "B");
            return;
        }

        m_frontDisplay->DrawText(width / 2, height / 2, NAME_FONT_SIZE, BLACK32, m_currentStudent.firstName);
        return;
    case 3:
        m_rearDisplay->FillRectangle(0.0f, 0.0f, width, height, YELLOW_FILL, BLACK32, 0);
        m_rearDisplay->DrawText(width / 2, height / 2, LETTER_FONT_SIZE, WHITE32, "C");

        if (m_anonymousMode == AnonymousMode::Off)
        {
            m_frontDisplay->FillRectangle(0.0f, 0.0f, width, height, YELLOW_FILL, BLACK32, 0);
            m_frontDisplay->DrawText(width / 2, height / 2, NAME_FONT_SIZE, WHITE32, m_currentStudent.firstName);
            m_frontDisplay->DrawText(width - 50, height - 50, LETTER_FONT_SIZE, WHITE32, "C");
            return;
        }

        m_frontDisplay->DrawText(width / 2, height / 2, NAME_FONT_SIZE, BLACK32, m_currentStudent.firstName);
        return;
    case 4:
        m_rearDisplay->FillRectangle(0.0f, 0.0f, width, height, GREEN_FILL, BLACK32, 0);
        m_rearDisplay->DrawText(width / 2, height / 2, LETTER_FONT_SIZE, WHITE32, "D");

        if (m_anonymousMode == AnonymousMode::Off)
        {
            m_frontDisplay->FillRectangle(0.0f, 0.0f, width, height, GREEN_FILL, BLACK32, 0);
            m_frontDisplay->DrawText(width / 2, height / 2, NAME_FONT_SIZE, WHITE32, m_currentStudent.firstName);
            m_frontDisplay->DrawText(width - 50, height - 50, LETTER_FONT_SIZE, WHITE32, "D");
            return;
        }

        m_frontDisplay->DrawText(width / 2, height / 2, NAME_FONT_SIZE, BLACK32, m_currentStudent.firstName);
        return;
    }

    m_frontDisplay->DrawText(width / 2, height / 2, NAME_FONT_SIZE, BLACK32, m_currentStudent.firstName);

    const auto touchPos = m_touch->GetTouchPos();
    if (m_touch->IsTouched() && RectOverlapTest(0.0f, 0.0f, width / 2, height / 2, touchPos.first, touchPos.second, 1, 1))
    {
        m_selectedPollOption = 1;
    }

    m_rearDisplay->FillRectangle(
        0.0f, 0.0f, width / 2, height / 2, 
        RED_FILL, BLACK32, 0
    );

    m_rearDisplay->DrawText(
        width / 4, height / 4,
        LETTER_FONT_SIZE, WHITE32, "A"
    );

    if (m_numPollOptions >= 2)
    {
        m_rearDisplay->FillRectangle(
            width / 2, 0.0f, width / 2, height / 2,
            BLUE_FILL, BLACK32, 0
        );

        m_rearDisplay->DrawText(
            width - (width / 4), height / 4,
            LETTER_FONT_SIZE, WHITE32, "B"
        );

        if (m_touch->IsTouched() && RectOverlapTest(width / 2, 0.0f, width / 2, height / 2, touchPos.first, touchPos.second, 1, 1))
        {
            m_selectedPollOption = 2;
        }
    }

    if (m_numPollOptions >= 3)
    {
        m_rearDisplay->FillRectangle(
            0.0f, height / 2, width / 2, height / 2,
            YELLOW_FILL, BLACK32, 0
        );

        m_rearDisplay->DrawText(
            width / 4, height - (height / 4),
            LETTER_FONT_SIZE, WHITE32, "C"
        );

        if (m_touch->IsTouched() && RectOverlapTest(0.0f, height / 2, width / 2, height / 2, touchPos.first, touchPos.second, 1, 1))
        {
            m_selectedPollOption = 3;
        }
    }

    if (m_numPollOptions == 4)
    {
        m_rearDisplay->FillRectangle(
            width / 2, height / 2, width / 2, height / 2,
            GREEN_FILL, BLACK32, 0
        );

        m_rearDisplay->DrawText(
            width - (width / 4), height - (height / 4),
            LETTER_FONT_SIZE, WHITE32, "D"
        );

        if (m_touch->IsTouched() && RectOverlapTest(width / 2, height / 2, width / 2, height / 2, touchPos.first, touchPos.second, 1, 1))
        {
            m_selectedPollOption = 4;
        }
    }

    if (m_selectedPollOption > 0)
    {
        Message resp(PacketType::PollResponse, m_network->ClientId());
        resp.Push(&m_selectedPollOption, sizeof(m_selectedPollOption));
        m_network->SendToServer(resp);
    }
}

void Nameplate::ClearReaction()
{
    const Reaction r = m_reactionSelected;
    m_reactionSelected = Reaction::None;

    Message msg(PacketType::ClearReaction, m_network->ClientId());
    msg.Push(&r, sizeof(r));
    m_network->SendToServer(msg);

    m_reactionSent = false;
    LOG_DEBUG("[Nameplate] reeaction cleared");
    m_frontForeground = BLACK24;
    m_frontBackground = WHITE24;
}

}
