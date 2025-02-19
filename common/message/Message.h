#pragma once
#include <stdint.h>
#include <exception>
#include <vector>
#include <string>

namespace nameplate
{

enum class PacketType : uint16_t
{
    Null = 0
};

struct Header
{
    Header(uint32_t clientId, PacketType type, uint16_t size)
        : clientId(clientId), packetType(type), payloadSize(size)
    {
    }

    uint32_t clientId;
    PacketType packetType;
    uint16_t payloadSize;
};
static_assert(sizeof(Header) == 8);

class Message
{
public:
    Message(PacketType type)
        : m_header(0, type, 0)
    {
    }

    uint32_t Size() const
    {
        return static_cast<uint16_t>(m_payload.size()) + sizeof(Header);
    }

    uint16_t PayloadSize() const
    {
        return static_cast<uint16_t>(m_payload.size());
    }

    const Header& Head() const
    {
        return m_header;
    }

    void ResizePayload(uint16_t size)
    {
        m_payload.resize(size);
    }

    void* PayloadData()
    {
        return m_payload.data();
    }

public:

    void Push(const void* data, uint16_t size);
    void Pop(void* buffer, size_t bufferSz, uint16_t size);

public:

    std::string AsString() const;

    friend std::ostream& operator<<(std::ostream& os, const Message& msg)
    {
        os << msg.AsString();
        return os;
    }


private:
    Header m_header;
    std::vector<uint8_t> m_payload;
};


class InsufficientBufferSizeException : public std::exception
{
public:

    InsufficientBufferSizeException(size_t bufferSize, uint16_t dataSize);

    const char* what() const noexcept override
    {
        return m_what.c_str();
    }

    size_t BufferSize() const
    {
        return m_bufSize;
    }

    uint16_t DataSize() const
    {
        return m_dataSize;
    }

private:
    size_t m_bufSize;
    uint16_t m_dataSize;
    std::string m_what;
};


class RequestExceedsAvilableDataException : public std::exception
{
public:

    RequestExceedsAvilableDataException(uint16_t requestSize, uint16_t payloadSize);

    const char* what() const noexcept override
    {
        return m_what.c_str();
    }

    uint16_t RequestSize() const
    {
        return m_requestSize;
    }

    uint16_t PayloadSize() const
    {
        return m_payloadSize;
    }

private:
    uint16_t m_requestSize;
    uint16_t m_payloadSize;
    std::string m_what;
};

}