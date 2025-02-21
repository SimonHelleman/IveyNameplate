#pragma once
#include <memory>
#include <deque>
#include <functional>

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
    ClientConnection(asio::io_context& context, asio::ip::tcp::socket socket, unsigned int id, std::deque<Message>& msgInQueue, std::function<void(const ClientConnection&)> disconHandle);
    ClientConnection(const ClientConnection&) = delete;
    ~ClientConnection() = default;

    ClientConnection operator=(const ClientConnection&) = delete;

public:
    bool IsConnected() const;
    void Start();

    void SendMessage(const Message& msg);
    void Disconnect();

public:

    unsigned int Id() const
    {
        return m_id;
    }

    asio::ip::address IpAddress() const
    {
        return m_socket.remote_endpoint().address();
    }

private:
    void AsyncSendHeader();
    void AsyncSendPayload();

    void AsyncReceiveHeader();
    void AsyncReceivePayload();

private:
    asio::ip::tcp::socket m_socket;
    asio::io_context& m_context;
    unsigned int m_id;

    Message m_incomingMsg;
     
    std::deque<Message>& m_incomingMessageQueue;
    std::deque<Message> m_outgoingMessageQueue;

    std::function<void(const ClientConnection&)> m_disconnectHandler;
};
}