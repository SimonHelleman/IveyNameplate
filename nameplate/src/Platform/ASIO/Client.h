#pragma once

#include <memory>
#include <deque>
#include <vector>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "../../Network.h"
#include "Message.h"
#include "ServerConnection.h"

namespace nameplate
{

class Client : public Network
{
public:
    Client(const TCPNetworkConfig& config);
    Client(const Client&) = delete;
    ~Client() = default;

    Client operator=(const Client&) = delete;

    void Disconnect();

    bool IsConnected() const;

    void SendToServer(const Message& msg) override;

    void SubscribeToPacket(const PacketType packet, std::function<void(const Message&)> eventHandler) override;

    void HandleMessages() override;

private:
    struct Callback
    {
        Callback(const PacketType packet, const std::function<void(const Message& msg)> callback)
            : packet(packet), eventHandler(callback)
        {
        }

        const PacketType packet;
        const std::function<void(const Message& msg)> eventHandler;
    };

private:
    asio::io_context m_context;
    std::thread m_clientThread;
    
    // socket gets moved to connection upon success
    asio::ip::tcp::socket m_tempSocket;

    std::unique_ptr<ServerConnection> m_server;

    std::deque<Message> m_incomingMessageQueue;
    uint32_t m_id;
    std::vector<Callback> m_callbacks;

};

}
