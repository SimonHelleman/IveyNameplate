#pragma once
#include "Message.h"

namespace nameplate
{
class Network
{
    virtual void OnConnect() = 0;
    virtual void OnDisconnect() = 0;
    virtual void SendToServer(const Message& message) = 0;
    virtual void HandleMessage() = 0;
};
}