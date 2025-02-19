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

class ServerConnection : public std::enable_shared_from_this<ServerConnection>
{
public:
    ServerConnection(asio::io_context& context, asio::ip::tcp::socket socket, std::deque<Message>& msgInQueue);
    ServerConnection(const ServerConnection&) = delete;
    ~ServerConnection();

    ServerConnection operator=(const ServerConnection&) = delete;

    void Connect(const asio::ip::tcp::resolver::results_type& endpoint);
    bool IsConnected() const;

    void SendMessage(const Message& msg); // WINAPI clash??
    void Disconnect();


private:
    asio::ip::tcp::socket m_socket;
    asio::io_context& m_context;

    std::deque<Message>& m_incomingMessageQueue;
    std::deque<Message> m_outgoingMessageQueue;
};
}