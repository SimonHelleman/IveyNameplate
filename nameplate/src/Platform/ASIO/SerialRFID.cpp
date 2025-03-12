#include <stdlib.h>
#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>
#include "../../PlatformFactory.h"
#include "SerialRFID.h"

namespace nameplate
{

std::unique_ptr<RFID> PlatformFactory::CreateRFID(const char* serialPort, const unsigned int baudRate)
{
    return std::make_unique<SerialRFID>(serialPort, baudRate);
}

SerialRFID::SerialRFID(const char* serialPort, const unsigned int baudRate)
    : m_service(), m_serial(m_service, serialPort)
{
    m_serial.set_option(asio::serial_port_base::baud_rate(baudRate));
}

uint32_t SerialRFID::GetId()
{
    constexpr size_t BUF_SZ = 256;
    constexpr size_t ID_SZ = 8;
    char buffer[BUF_SZ];
    const size_t bytesRead = m_serial.read_some(asio::buffer(buffer, ID_SZ));

    buffer[bytesRead] = '\0';

    LOG_DEBUG("[SerialRFID] " + std::string(buffer));

    return static_cast<uint32_t>(strtoul(buffer, NULL, 16));
}


}
