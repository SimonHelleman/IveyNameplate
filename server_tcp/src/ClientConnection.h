#pragma once
#include <memory>
#include <deque>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "Message.h"

namespace nameplate
{

class ClientConnection : public std::enable_shared_from_this<ClientConnection>
{
public:
    ClientConnection(asio::io_context& context, asio::ip::tcp::socket socket, unsigned int id, std::deque<Message>& msgInQueue);
    ClientConnection(const ClientConnection&) = delete;
    ~ClientConnection();

    ClientConnection operator=(const ClientConnection&) = delete;

public:
    bool IsConnected() const;
    void Start();

    void SendMessage(const Message& msg);
    void Disconnect();

    unsigned int Id() const
    {
        return m_id;
    }

private:
    asio::ip::tcp::socket m_socket;
    asio::io_context& m_context;
    unsigned int m_id;

    Message m_incomingMsg;
     
    std::deque<Message>& m_incomingMessageQueue;
    std::deque<Message> m_outgoingMessageQueue;
};
}