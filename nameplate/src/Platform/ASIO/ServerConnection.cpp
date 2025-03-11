#include "ServerConnection.h"
#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>

namespace nameplate
{


ServerConnection::ServerConnection(asio::io_context& context, asio::ip::tcp::socket socket, std::deque<Message>& msgInQueue)
    :  m_context(context), m_socket(std::move(socket)), m_ipAddr(), 
    m_incomingMessageQueue(msgInQueue), m_incomingMsg(PacketType::Null, 0)
{
}

void ServerConnection::Connect(const asio::ip::tcp::resolver::results_type& endpoint)
{
    asio::async_connect(m_socket, endpoint, [this](std::error_code error, asio::ip::tcp::endpoint end) {
        if (!error)
        {
            m_ipAddr = m_socket.remote_endpoint().address();
            AsyncReceiveHeader();
        }
    });
}

bool ServerConnection::IsConnected() const
{
    return m_socket.is_open();
}

void ServerConnection::SendMessage(const Message& msg)
{
    asio::post(m_context, [this, msg]() {
        const bool isEmpty = m_outgoingMessageQueue.empty();
        m_outgoingMessageQueue.push_back(msg);
        // Only if we aren't already sending messages at the moment
        if (isEmpty)
        {
            AsyncSendHeader();
        }
    });
}

void ServerConnection::Disconnect()
{
    if (m_socket.is_open())
    {
        m_socket.close();
    }
}

void ServerConnection::AsyncSendHeader()
{
    asio::async_write(m_socket, asio::buffer(&m_outgoingMessageQueue.front().Head(), sizeof(Header)), [this](asio::error_code error, size_t) {
        if (error)
        {
            ERROR("[Connection] an error occurred when sending a message header.");
            Disconnect();
            return;            
        }

        if (m_outgoingMessageQueue.front().PayloadSize() > 0)
        {
            LOG_DEBUG("[Connection] header sent");
            AsyncSendPayload();
            return;
        }

        m_outgoingMessageQueue.pop_front();

        if (!m_outgoingMessageQueue.empty())
        {
            AsyncSendHeader();
        }
    });
}

void ServerConnection::AsyncSendPayload()
{
    asio::async_write(m_socket, asio::buffer(m_outgoingMessageQueue.front().PayloadData(), m_outgoingMessageQueue.front().PayloadSize()), [this](asio::error_code error, size_t) {
        if (error)
        {
            ERROR("[Connection] an error occurred when sending a message payload.");
            Disconnect();
            return;            
        }

        m_outgoingMessageQueue.pop_front();
        LOG_DEBUG("[Connection] payload sent");

        if (!m_outgoingMessageQueue.empty())
        {
            AsyncSendHeader();
        }
    });
}

void ServerConnection::AsyncReceiveHeader()
{
    asio::async_read(m_socket, asio::buffer(&m_incomingMsg.Head(), sizeof(Header)), [this](asio::error_code error, size_t) {
        if (error)
        {
            if (error == asio::error::eof)
            {
                INFO("[Connection] client disconnected.");
            }
            else
            {
                ERROR("[Connection] an error occurred when reading a message header. With error message: " + error.message());
            }

            Disconnect();
            return;
        }
        else if (m_incomingMsg.Head().packetType == PacketType::Null)
        {
            ERROR("[Connection] received a NULL message.");
            Disconnect();
            return;
        }

        // Print the header
        LOG_DEBUG("[Connection] Header: { client_id: " + std::to_string(m_incomingMsg.Head().clientId) +
                  ", packet_type: " + std::to_string(static_cast<uint16_t>(m_incomingMsg.Head().packetType)) +
                  ", payload_size: " + std::to_string(m_incomingMsg.Head().payloadSize) + " }");

        m_incomingMsg.ResizePayload(m_incomingMsg.Head().payloadSize);
        LOG_DEBUG("[Connection] Received header");
        AsyncReceivePayload();
    });
}

void ServerConnection::AsyncReceivePayload()
{
    asio::async_read(m_socket, asio::buffer(m_incomingMsg.PayloadData(), m_incomingMsg.PayloadSize()), [this](asio::error_code error, size_t) {
        if (error)
        {
            if (error == asio::error::eof)
            {
                INFO("[Connection] client disconnected.");
            }
            else
            {
                ERROR("[Connection] an error occurred when reading a message payload. With error message: " + error.message());
            }

            Disconnect();
            return;
        }

        m_incomingMessageQueue.emplace_back(std::move(m_incomingMsg));
        m_incomingMsg = Message(PacketType::Null, 0); // reset the object
        LOG_DEBUG("[Connection] Received payload");
        
        AsyncReceiveHeader();
    });
}



}