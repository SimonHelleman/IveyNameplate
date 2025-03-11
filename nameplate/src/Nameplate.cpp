#include "Nameplate.h"
namespace nameplate
{


Nameplate::Nameplate(const PlatformConfig<TCPNetworkConfig>& config)
    : m_frontDisplay(PlatformFactory::CreateDisplay(config.displayWidth, config.displayHeight, "nameplate_front")),
    m_rearDisplay(PlatformFactory::CreateDisplay(config.displayWidth, config.displayHeight, "nameplate_rear")),
    m_network(PlatformFactory::CreateNetwork<TCPNetworkConfig>(config.networkConfig))
{
    m_network->SubscribeToPacket(PacketType::SetClientId, [this](const Message& msg) {
        TestHandler(msg);
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

        const unsigned int centerX = m_frontDisplay->Width() / 2;
        const unsigned int centerY = m_frontDisplay->Height() / 2;
        constexpr unsigned int NAME_FONT_SIZE = 400;
        const char* name = "Kyle";
        const char* course = "BUS 1299E";

        m_frontDisplay->DrawText(centerX, centerY, NAME_FONT_SIZE, {}, name);

        m_rearDisplay->DrawText(centerX, centerY, NAME_FONT_SIZE / 2, {}, course);
        m_rearDisplay->DrawText(centerX, centerY + 100, NAME_FONT_SIZE / 3, {}, name);
        
        m_frontDisplay->Show();
        m_rearDisplay->Show();
    }
    
}

}
