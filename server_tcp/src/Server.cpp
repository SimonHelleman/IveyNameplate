#include <utility>
#include <functional>
#include <algorithm>
#include "Server.h"

namespace nameplate
{

Server::Server(unsigned int port)
    : m_port(port), m_connectionAcceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
    
}

void Server::Start()
{
    AsyncWaitForConnection();

    m_serverThread = std::thread([this]() {
        m_context.run();
    });
}

void Server::Stop()
{
    m_context.stop();

    if (m_serverThread.joinable())
    {
        m_serverThread.join();
    }
}

void Server::AsyncWaitForConnection()
{
    m_connectionAcceptor.async_accept([this](asio::error_code error, asio::ip::tcp::socket socket) {
        if (error)
        {
            // TODO

            AsyncWaitForConnection();
            return;
        }
        
        const std::function<void(const ClientConnection&)> disconHandle = std::bind(&Server::OnDisconnect, this, std::placeholders::_1);
        
        const auto connection = std::make_shared<ClientConnection>(
            m_context, std::move(socket), ++m_numCommections, m_incomingMessageQueue, disconHandle
        );

        m_connections.push_back(connection);
        connection->Start();

        OnConnect(*connection);
        
        AsyncWaitForConnection();
    });
}

void Server::SendMessage(ClientConnection& client, const Message& message)
{
    if (!client.IsConnected())
    {
        OnDisconnect(client);
        return;
    }

    client.SendMessage(message);
}

void Server::OnConnect(ClientConnection& client)
{
    // Send their Id
    Message message(PacketType::SetClientId, client.Id());
    SendMessage(client, message);
}

void Server::OnDisconnect(const ClientConnection& client)
{
    const auto it = std::find_if(m_connections.begin(), m_connections.end(), [&client](const std::shared_ptr<ClientConnection>& ptr) {
        return ptr.get() == &client;
    });

    if (it != m_connections.end())
    {
        m_connections.erase(it);
    }

    // TODO throw error or smth
}

void Server::HandleMessages()
{
    
}

}
