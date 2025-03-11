#include <exception>
#include <string>

#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>
#include "../../PlatformFactory.h"
#include "Client.h"

namespace nameplate
{

template<>
std::unique_ptr<Network> PlatformFactory::CreateNetwork(const TCPNetworkConfig& config)
{
    return std::make_unique<Client>(config);
}

Client::Client(const TCPNetworkConfig& config)
    : m_tempSocket(m_context)
{
    try
    {
        asio::ip::tcp::resolver resolver(m_context);
        const auto endpoints = resolver.resolve(config.serverIP, std::to_string(config.serverPort));

        m_server = std::make_unique<ServerConnection>(m_context, asio::ip::tcp::socket(m_context), m_incomingMessageQueue);

        m_server->Connect(endpoints);


        m_clientThread = std::thread([this]() {
            m_context.run();
        });
    }
    catch (const std::exception& e)
    {
        // TODO throw custom except
    }
}

void Client::Disconnect()
{
    if (IsConnected())
    {
        m_server->Disconnect();
    }

    m_context.stop();

    if (m_clientThread.joinable())
    {
        m_clientThread.join();
    }

    m_server.reset();
}

bool Client::IsConnected() const
{
    if (m_server)
    {
        return m_server->IsConnected();
    }

    return false;
}

void Client::SendToServer(const Message& msg)
{
    if (IsConnected())
    {
        m_server->SendMessage(msg);
    }
}

void Client::SubscribeToPacket(const PacketType packet, std::function<void(const Message&)> eventHandler)
{
    m_callbacks.emplace_back(packet, eventHandler);
}

void Client::HandleMessages()
{
    while (!m_incomingMessageQueue.empty())
    {
        const Message& msg = m_incomingMessageQueue.front();
        LOG_DEBUG("[Client] message:\n" + msg.AsString());

        if (msg.GetPacketType() == PacketType::SetClientId)
        {
            m_id = msg.ClientId();
        }

        for (const auto& c : m_callbacks)
        {
            if (msg.GetPacketType() == c.packet)
            {
                c.eventHandler(msg);
            }
        }

        m_incomingMessageQueue.pop_front();
    }
}


}