#include "Nameplate.h"

namespace nameplate
{

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
    m_keyboard(config.displayWidth / 20, config.displayWidth / 20, config.displayWidth / 200, 2),
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

        m_frontDisplay->Clear({ 255, 255, 255 });
        m_rearDisplay->Clear( { 255, 255, 255 });

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
    m_rearDisplay->DrawText(centerX, centerY, 20, {}, "Tap Student ID card to begin");
    m_keyboard.Draw(*m_rearDisplay, 0, 0, { 255, 255, 255 }, { });

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

    m_frontDisplay->DrawText(centerX, centerY, NAME_FONT_SIZE, {}, m_currentStudent.firstName.c_str());

    m_rearDisplay->DrawText(centerX, centerY, NAME_FONT_SIZE / 3, {}, fullName.c_str());
}

}
