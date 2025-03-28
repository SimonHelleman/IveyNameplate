#pragma once
#define ASIO_STANDALONE
#include <asio.hpp>
#include "../../RFID.h"

namespace nameplate
{

class SerialRFID : public RFID
{
public:
    SerialRFID(const char* serialPort, const unsigned int baudRate);
    uint32_t GetId() override;
private:
    const char* m_serialPortName;
    const unsigned int m_baudRate;
    asio::io_service m_service;
    asio::serial_port m_serial;
};

}
