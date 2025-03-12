#pragma once
#include <stdint.h>
#include <string>
#include <functional>
#include "Message.h"

namespace nameplate
{

struct TCPNetworkConfig
{
    const std::string serverIP;
    const unsigned int serverPort;
};

class Network
{
public:
    virtual ~Network() = default;

    virtual void SendToServer(const Message& message) = 0;
    virtual void SubscribeToPacket(const PacketType packet, std::function<void(const Message&)> eventHandler) = 0;
    virtual void HandleMessages() = 0;

    virtual uint32_t ClientId() const = 0;
};
}