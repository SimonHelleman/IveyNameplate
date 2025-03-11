#include <utility>
#include <functional>
#include <algorithm>
#include "Server.h"

#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>

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

    INFO("[Server] server started on port " + std::to_string(m_port));
}

void Server::Stop()
{
    m_context.stop();

    if (m_serverThread.joinable())
    {
        m_serverThread.join();
    }

    INFO("[Server] server stopped");
}

void Server::AsyncWaitForConnection()
{
    m_connectionAcceptor.async_accept([this](asio::error_code error, asio::ip::tcp::socket socket) {
        if (error)
        {
            ERROR_FL("[Server] async_accept failed with erorr: " + error.message());

            AsyncWaitForConnection();
            return;
        }

        LOG_DEBUG("[Server] New connection from " + socket.remote_endpoint().address().to_string());
        
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
    INFO("[Server] " + client.IpAddress() + " connected to server with id " + std::to_string(client.Id()));
    // Send their Id
    Message message(PacketType::SetClientId, client.Id());
    SendMessage(client, message);
}

void Server::OnDisconnect(const ClientConnection& client)
{
    INFO("[Server] " + client.IpAddress() + " (connection_" + std::to_string(client.Id()) + ") disconnected from server");
    const auto it = std::find_if(m_connections.begin(), m_connections.end(), [&client](const std::shared_ptr<ClientConnection>& ptr) {
        return ptr.get() == &client;
    });

    if (it != m_connections.end())
    {
        m_connections.erase(it);
        return;
    }

    ERROR("[Server] Could not remove connection_" + std::to_string(client.Id()) + " (" + client.IpAddress() + ')');
}

void Server::HandleMessages()
{
    while (!m_incomingMessageQueue.empty())
    {
        const Message& msg = m_incomingMessageQueue.front();
        LOG_DEBUG("[Server] message:\n" + msg.AsString());

        switch (msg.GetPacketType())
        {
        default:
            ERROR_FL("[Server] message is an unknown type and can not be handled");
        }

        m_incomingMessageQueue.pop_front();
    }
}

}
