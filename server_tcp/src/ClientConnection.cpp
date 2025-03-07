
#include "ClientConnection.h"

#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>

namespace nameplate
{
ClientConnection::ClientConnection(asio::io_context& context, asio::ip::tcp::socket socket, unsigned int id, std::deque<Message>& msgInQueue, std::function<void(const ClientConnection&)> disconHandle)
    : m_context(context), m_socket(std::move(socket)), m_ipAddr(m_socket.remote_endpoint().address()), m_id(id), m_incomingMessageQueue(msgInQueue), m_incomingMsg(PacketType::Null, 0), m_disconnectHandler(disconHandle)
{
}

void ClientConnection::Start()
{
    AsyncReceiveHeader();
}

bool ClientConnection::IsConnected() const
{
    return m_socket.is_open();
}

void ClientConnection::SendMessage(const Message& msg)
{
    // Do this on the server thread
    asio::post(m_context, [self = shared_from_this(), msg]() {
        const bool isEmpty = self->m_outgoingMessageQueue.empty();
        self->m_outgoingMessageQueue.push_back(msg);
        // Only if we aren't already sending messages at the moment
        if (isEmpty)
        {
            self->AsyncSendHeader();
        }
    });
}

void ClientConnection::Disconnect()
{
    if (m_socket.is_open())
    {
        m_socket.close();
    }

    LOG_DEBUG_FL("[Connection_" + std::to_string(m_id) + "] call disconnect handler");
    m_disconnectHandler(*this);
}

void ClientConnection::AsyncSendHeader()
{
    asio::async_write(m_socket, asio::buffer(&m_outgoingMessageQueue.front().Head(), sizeof(Header)), [self = shared_from_this()](asio::error_code error, size_t) {
        if (error)
        {
            ERROR("[Connection_" + std::to_string(self->Id()) + "] an error occurred when sending a message header.");
            self->Disconnect();
            return;            
        }

        if (self->m_outgoingMessageQueue.front().PayloadSize() > 0)
        {
            LOG_DEBUG("[Connection_" + std::to_string(self->Id()) + "] header sent");
            self->AsyncSendPayload();
            return;
        }

        self->m_outgoingMessageQueue.pop_front();

        if (!self->m_outgoingMessageQueue.empty())
        {
            self->AsyncSendHeader();
        }
    });
}

void ClientConnection::AsyncSendPayload()
{
    asio::async_write(m_socket, asio::buffer(m_outgoingMessageQueue.front().PayloadData(), m_outgoingMessageQueue.front().PayloadSize()), [self = shared_from_this()](asio::error_code error, size_t) {
        if (error)
        {
            ERROR("[Connection_" + std::to_string(self->Id()) + "] an error occurred when sending a message payload.");
            self->Disconnect();
            return;            
        }

        self->m_outgoingMessageQueue.pop_front();
        LOG_DEBUG("[Connection_" + std::to_string(self->Id()) + "] payload sent");

        if (!self->m_outgoingMessageQueue.empty())
        {
            self->AsyncSendHeader();
        }
    });
}

void ClientConnection::AsyncReceiveHeader()
{
    asio::async_read(m_socket, asio::buffer(&m_incomingMsg.Head(), sizeof(Header)), [self = shared_from_this()](asio::error_code error, size_t) {
        if (error)
        {
            if (error == asio::error::eof)
            {
                INFO("[Connection_" + std::to_string(self->Id()) + "] client disconnected.");
            }
            else
            {
                ERROR("[Connection_" + std::to_string(self->Id()) + "] an error occurred when reading a message header. With error message: " + error.message());
            }

            self->Disconnect();
            return;
        }
        else if (self->m_incomingMsg.Head().packetType == PacketType::Null)
        {
            ERROR("[Connection_" + std::to_string(self->Id()) + "] received a NULL message.");
            self->Disconnect();
            return;
        }

        // Print the header
        LOG_DEBUG("[Connection_" + std::to_string(self->Id()) + "] Header: { client_id: " + std::to_string(self->m_incomingMsg.Head().clientId) +
                  ", packet_type: " + std::to_string(static_cast<uint16_t>(self->m_incomingMsg.Head().packetType)) +
                  ", payload_size: " + std::to_string(self->m_incomingMsg.Head().payloadSize) + " }");

        self->m_incomingMsg.ResizePayload(self->m_incomingMsg.Head().payloadSize);
        LOG_DEBUG("[Connection_" + std::to_string(self->Id()) + "] Received header");
        self->AsyncReceivePayload(); // Nesting lambdas appears to be harder than I thought
    });
}

void ClientConnection::AsyncReceivePayload()
{
    asio::async_read(m_socket, asio::buffer(m_incomingMsg.PayloadData(), m_incomingMsg.PayloadSize()), [self = shared_from_this()](asio::error_code error, size_t) {
        if (error)
        {
            if (error == asio::error::eof)
            {
                INFO("[Connection_" + std::to_string(self->Id()) + "] client disconnected.");
            }
            else
            {
                ERROR("[Connection_" + std::to_string(self->Id()) + "] an error occurred when reading a message payload. With error message: " + error.message());
            }

            self->Disconnect();
            return;
        }

        self->m_incomingMessageQueue.emplace_back(std::move(self->m_incomingMsg));
        self->m_incomingMsg = Message(PacketType::Null, 0); // reset the object
        LOG_DEBUG("[Connection_" + std::to_string(self->Id()) + "] Received payload");
        
        self->AsyncReceiveHeader();
    });
}
}