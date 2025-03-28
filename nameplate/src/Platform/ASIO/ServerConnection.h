#pragma once
#include <deque>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "Message.h"

namespace nameplate
{

class ServerConnection
{
public:
    ServerConnection(asio::io_context& context, asio::ip::tcp::socket socket, std::deque<Message>& msgInQueue);
    ServerConnection(const ServerConnection&) = delete;
    ~ServerConnection() = default;

    ServerConnection operator=(const ServerConnection&) = delete;

public:
    void Connect(const asio::ip::tcp::resolver::results_type& endpoint);
    bool IsConnected() const;

    void SendMessage(const Message& msg); // WINAPI clash??
    void Disconnect();

private:

    std::string IpAddress() const
    {
        return m_ipAddr.to_string();
    }

private:
    void AsyncSendHeader();
    void AsyncSendPayload();
    
    void AsyncReceiveHeader();
    void AsyncReceivePayload();


private:
    asio::ip::tcp::socket m_socket;
    asio::io_context& m_context;
    asio::ip::address m_ipAddr;

    Message m_incomingMsg;

    std::deque<Message>& m_incomingMessageQueue;
    std::deque<Message> m_outgoingMessageQueue;
};
}