#pragma once

#include <memory>
#include <vector>
#include <deque>
#include <thread>
#include "ClientConnection.h"

namespace nameplate
{
class Server
{
public:
    Server(unsigned int port);

    Server(const Server&) = delete;

    Server& operator=(const Server&) = delete;
public:

    void Start();
    void Stop();

    void AsyncWaitForConnection();

    void SendMessage(ClientConnection& client, const Message& message);

    void OnConnect(ClientConnection& client);
    void OnDisconnect(const ClientConnection& client);

    void HandleMessages();

private:
    unsigned int m_port;
    unsigned int m_numCommections = 0;

    std::vector<std::shared_ptr<ClientConnection>> m_connections;
    
    std::deque<Message> m_incomingMessageQueue;

    asio::io_context m_context;
    asio::ip::tcp::acceptor m_connectionAcceptor;
    std::thread m_serverThread;
};

}