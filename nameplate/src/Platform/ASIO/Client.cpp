#include <exception>
#include <string>
#include "Client.h"

namespace nameplate
{

Client::Client(const char* serverIP, unsigned int serverPort)
    : m_tempSocket(m_context)
{
    try
    {
        asio::ip::tcp::resolver resolver(m_context);
        const auto endpoints = resolver.resolve(serverIP, std::to_string(serverPort));

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

    m_server.release();
}

bool Client::IsConnected()
{
    if (m_server)
    {
        return m_server->IsConnected();
    }

    return false;
}


}