#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdexcept>
#include "EccelRFID.h"


namespace nameplate
{

EccelRFID::EccelRFID(const char* serialPort)
    : m_serial(0), m_serialBuf(UART_BUFFER_SZ)
{
    m_serial = open(serialPort, O_RDWR | O_NOCTTY | O_NDELAY);

    if (m_serial == -1)
    {
        throw std::runtime_error("[EccelRDIF.cpp] Failed to open serial port");
    }

    struct termios options;
    tcgetattr(m_serial, &options);

    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    // 8N1 Mode (8 data bits, No parity, 1 stop bit)
    options.c_cflag &= ~PARENB; // No parity
    options.c_cflag &= ~CSTOPB; // 1 stop bit
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;     // 8 data bits

    // No hardware flow control
    options.c_cflag &= ~CRTSCTS;
    options.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore control lines

    // No software flow control
    options.c_iflag &= ~(IXON | IXOFF | IXANY);

    // Raw input mode (disable special processing)
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    tcsetattr(m_serial, TCSANOW, &options);
}

EccelRFID::~EccelRFID()
{
    close(m_serial)
}

uint32_t EccelRFID::GetId() const
{
}

int EccelRFID::ReadUART() const
{
    memset(m_serialBuf.get(), 0, UART_BUFFER_SZ * sizeof(char));

    const int bytesRead = read(m_serial, m_serialBuf.get(), UART_BUFFER_SZ - 1);

    return bytesRead;
}

void EccelRFID::WriteUART(const uint8_t* data, size_t dataLen)
{
    write(m_serial, data, dataLen) const;
}

}