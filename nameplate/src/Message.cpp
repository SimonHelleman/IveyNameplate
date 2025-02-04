#include <sstream>
#include "Message.h"

namespace nameplate
{

void Message::Push(const void* data, uint16_t size)
{
    const size_t oldSize = m_payload.size();

    m_payload.resize(oldSize + size);

    memcpy(m_payload.data() + oldSize, data, size);

    m_header.payloadSize = m_payload.size();
}

void Message::Pop(void* buffer, size_t bufferSz, uint16_t size)
{
    if (size > bufferSz)
    {
        throw InsufficientBufferSizeException(bufferSz, size);
    }

    if (size > m_payload.size())
    {
        throw RequestExceedsAvilableDataException(size, static_cast<uint16_t>(m_payload.size()));
    }

    const size_t startPos = m_payload.size() - size;

    memcpy(buffer, m_payload.data() + startPos, size);

    m_payload.resize(m_payload.size() - size);

    m_header.payloadSize = m_payload.size();
}

std::string Message::AsString() const
{
    std::stringstream ss;

    constexpr size_t BUF_SZ = 16;
    char buf[BUF_SZ];

    ss << "************************ Message ************************\n";

    snprintf(buf, BUF_SZ, "%02d", (uint8_t)m_header.packetType);
    ss << "ID: " << buf << "                                 ";
    snprintf(buf, BUF_SZ, "%04d", m_header.payloadSize);
    ss << "Payload Size: " << buf << '\n';
    ss << "---------------------------------------------------------\n";

    for (int i = 0; i < m_payload.size(); ++i)
    {
        if (i % 16 == 0)
        {
            if (i > 0) ss << "\n";
            snprintf(buf, BUF_SZ, "%08x: ", i);
            ss << buf;
        }

        snprintf(buf, BUF_SZ, "%02x ", m_payload[i]);
        ss << buf;
    }

    ss << "\n*********************************************************\n";
    return ss.str();
}


InsufficientBufferSizeException::InsufficientBufferSizeException(size_t bufferSize, uint16_t dataSize)
    : m_bufSize(bufferSize), m_dataSize(dataSize), m_what()
{
    m_what = "[InsufficientBufferSizeException] the specified buffer size (";
    m_what += bufferSize;
    m_what += ") is insufficient to contain the requested ";
    m_what += dataSize;
    m_what += " bytes of data";
}

RequestExceedsAvilableDataException::RequestExceedsAvilableDataException(uint16_t requestSize, uint16_t payloadSize)
    : m_requestSize(requestSize), m_payloadSize(payloadSize), m_what()
{
    m_what = "[RequestExceedsAvailableDataException] the requested amount of data (";
    m_what += requestSize;
    m_what += ") is greater than what can be provided (";
    m_what += payloadSize;
    m_what += ")";
}


}