#pragma once

#include <memory>
#include <deque>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "Message.h"
#include "ServerConnection.h"

namespace nameplate
{

class Client
{
public:
    Client(const char* serverIP, unsigned int serverPort);
    Client(const Client&) = delete;
    ~Client();

    Client operator=(const Client&) = delete;

    void Disconnect();

    bool IsConnected();

    std::deque<Message>& Messages()
    {
        return m_incomingMessageQueue;
    }

private:
    asio::io_context m_context;
    std::thread m_clientThread;
    
    // socket gets moved to connection upon success
    asio::ip::tcp::socket m_tempSocket;

    std::unique_ptr<ServerConnection> m_server;

    std::deque<Message> m_incomingMessageQueue;

};

}
