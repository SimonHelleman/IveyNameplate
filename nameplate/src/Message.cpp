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