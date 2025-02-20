#include "ServerConnection.h"

namespace nameplate
{


ServerConnection::ServerConnection(asio::io_context& context, asio::ip::tcp::socket socket, std::deque<Message>& msgInQueue)
    :  m_context(context), m_socket(std::move(socket)), m_incomingMessageQueue(msgInQueue)
{

}

}