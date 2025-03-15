#include "Nameplate.h"

namespace nameplate
{

static const RGBA s_white32 = { 255, 255, 255 };
static const RGBA s_black32 = { };
static const RGB s_white24 = { 255, 255, 255 };
static const RGB s_black24 = { };

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
    m_touch(dynamic_cast<Touch*>(m_rearDisplay.get())), // Yet another area where my attempts to keep this ultra potrable fails
    m_keyboard(0, 0, config.displayWidth / 20, config.displayWidth / 20, config.displayWidth / 200, 2),
    m_currentState(State::Idle), m_stateTransition(true), m_readId(false), m_currentId(0), m_currentStudent(),
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

}

void Nameplate::TestHandler(const Message& msg)
{
    LOG_DEBUG("[Nameplate] Received clientId msg");
}


void Nameplate::Run()
{
    while (true)
    {
        m_frontDisplay->HandleEvents();
        m_rearDisplay->HandleEvents();

        m_network->HandleMessages();

        m_frontDisplay->Clear(s_white24);
        m_rearDisplay->Clear(s_white24);

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
    m_cardThread = std::make_unique<std::thread>(RFIDThread, m_card.get(), std::ref(m_currentId), std::ref(m_readId));
}

void Nameplate::IdleStatePeriodic()
{
    const unsigned int centerX = m_frontDisplay->Width() / 2;
    const unsigned int centerY = m_frontDisplay->Height() / 2;
    
    m_rearDisplay->DrawText(centerX, centerY, 20, s_black32, "Tap Student ID card to begin");
    m_keyboard.Draw(*m_rearDisplay, s_white32, s_black32);

    m_keyboard.Update(*m_touch);

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
    LOG_DEBUG("[Namplate] Name state init");
}

void Nameplate::NameStatePeriodic()
{
    const unsigned int centerX = m_frontDisplay->Width() / 2;
    const unsigned int centerY = m_frontDisplay->Height() / 2;
    constexpr unsigned int NAME_FONT_SIZE = 400;

    std::string fullName = std::string(m_currentStudent.firstName) + ' ' + std::string(m_currentStudent.lastName);

    m_frontDisplay->DrawText(centerX, centerY, NAME_FONT_SIZE, s_black32, m_currentStudent.firstName.c_str());

    m_rearDisplay->DrawText(centerX, centerY, NAME_FONT_SIZE / 3, s_black32, fullName.c_str());
}

}
