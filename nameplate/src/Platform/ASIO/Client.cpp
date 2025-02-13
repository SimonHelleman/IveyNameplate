#include <exception>
#include <string>
#include "Client.h"

namespace nameplate
{

Client::Client(const char* serverIP, unsigned int serverPort)
    : m_tempSocket(m_context)
{
    try
    {
        asio::ip::tcp::resolver resolver(m_context);
        auto endpoints = resolver.resolve(serverIP, std::to_string(serverPort));



    }
    catch (const std::exception& e)
    {
        // TODO throw custom except
    }
}

void Client::Disconnect()
{

}

bool Client::IsConnected()
{
    if (m_server)
    {
        return m_server->IsConnected();
    }

    return false;
}


}