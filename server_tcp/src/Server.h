#pragma once
#include <stdint.h>
#include <memory>
#include <vector>
#include <deque>
#include <thread>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

// ASIO includes windows API which is annoying
#ifdef SendMessage
#undef SendMessage
#endif

#include "DatabaseConnection.h"
#include "ClientConnection.h"

namespace nameplate
{
class Server
{
public:
    Server(uint16_t port, DatabaseConnection& database);

    Server(const Server&) = delete;

    Server& operator=(const Server&) = delete;
public:

    void Start();
    void Stop();

    void AsyncWaitForConnection();

    void SendMessage(ClientConnection& client, const Message& message);
    void SendMessage(const uint32_t clientId, const Message& message);

    void OnConnect(ClientConnection& client);
    void OnDisconnect(const ClientConnection& client);

    void HandleMessages();

private:
    void HandleStudentId(Message& msg);
    void HandleStudentInfo(Message& msg);

private:
    uint16_t m_port;
    unsigned int m_numConnections = 0;

    std::vector<std::shared_ptr<ClientConnection>> m_connections;
    
    std::deque<Message> m_incomingMessageQueue;

    asio::io_context m_context;
    asio::ip::tcp::acceptor m_connectionAcceptor;
    std::thread m_serverThread;

    DatabaseConnection& m_database;
};

}