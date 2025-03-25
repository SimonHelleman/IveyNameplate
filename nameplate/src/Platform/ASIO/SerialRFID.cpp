#include <stdlib.h>
#include "../../PlatformFactory.h"
#include "SerialRFID.h"

#ifdef NAMEPLATE_PLATFORM_LINUX
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif

#undef ERROR // WinAPI smh
#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>

namespace nameplate
{

#ifdef NAMEPLATE_PLATFORM_LINUX
void ClearNativeSerialBuffer(asio::serial_port& serial)
{
    const int fd = serial.native_handle();
    // Flush both input and output buffers
    if (tcflush(fd, TCIOFLUSH) == -1)
    {
        ERROR("[SerialRFID_Linux] tcflush failed");
    }
    else
    {
        LOG_DEBUG("[SerialRFID_Linux] buffers cleared successfully");
    }
}
#endif

std::unique_ptr<RFID> PlatformFactory::CreateRFID(const char* serialPort, const unsigned int baudRate)
{
    try
    {
        auto obj = std::make_unique<SerialRFID>(serialPort, baudRate);
        return obj;
    }
    catch (const std::exception& e)
    {
        ERROR("[SerialRFID] Could not connect to RFID Reader");
    }

    return std::unique_ptr<RFID>();
}

SerialRFID::SerialRFID(const char* serialPort, const unsigned int baudRate)
    : m_serialPortName(serialPort), m_baudRate(baudRate), m_service(), m_serial(m_service, serialPort)
{
    m_serial.set_option(asio::serial_port_base::baud_rate(baudRate));
}

uint32_t SerialRFID::GetId()
{
    if (!m_serial.is_open())
    {
        m_serial.open(m_serialPortName);
        m_serial.set_option(asio::serial_port_base::baud_rate(m_baudRate));
    }

#ifdef NAMEPLATE_PLATFORM_LINUX
    ClearNativeSerialBuffer(m_serial);
#endif

    constexpr size_t BUF_SZ = 256;
    constexpr size_t ID_SZ = 8;
    char buffer[BUF_SZ];
    const size_t bytesRead = m_serial.read_some(asio::buffer(buffer, ID_SZ));

    buffer[bytesRead] = '\0';

    LOG_DEBUG("[SerialRFID] " + std::string(buffer));

    m_serial.close();
    return static_cast<uint32_t>(strtoul(buffer, NULL, 16));
}

}
