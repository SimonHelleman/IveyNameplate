#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdexcept>
#include "EccelRFID.h"


namespace nameplate
{

EccelRFID::EccelRFID(const char* serialPort)
    : m_serial(0), m_serialBuf(std::make_unique<uint8_t[]>(UART_BUFFER_SZ))
{
    m_serial = open(serialPort, O_RDWR | O_NOCTTY);

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
    close(m_serial);
}

uint32_t EccelRFID::GetId() const
{
    return 0;
}

ssize_t EccelRFID::ReadUART() const
{
    memset(m_serialBuf.get(), 0, UART_BUFFER_SZ * sizeof(char));

    const ssize_t bytesRead = read(m_serial, m_serialBuf.get(), UART_BUFFER_SZ - 1);

    return bytesRead;
}

ssize_t EccelRFID::WriteUART(const void* data, size_t dataLen) const 
{
    const ssize_t bytesWritten = write(m_serial, data, dataLen);
    return bytesWritten;
}

EccelRFID::Packet::Packet(const void* data, uint16_t len)
{
    const uint16_t crc = 0x00;

    const uint8_t* lenBytes = (const uint8_t*)len;
    const uint8_t* crcBytes = (const uint8_t*)crc;

    m_packet.push_back(0x02);
    m_packet.push_back(lenBytes[0]);
    m_packet.push_back(lenBytes[1]);
    m_packet.push_back(crcBytes[0]);
    m_packet.push_back(crcBytes[1]);

    const uint8_t* payload = (const uint8_t*)data;
    for (int i = 0; i < len; ++i)
    {
        m_packet.push_back(payload[i]);
    }
}

EccelRFID::Packet::Packet(const std::vector<uint8_t>& packetData)
{
    
}

}

#include <iostream>

using namespace nameplate;


int main()
{
    const EccelRFID rfid("/dev/ttyAMA0");

    const char* greeting = "hello world\n";
    const size_t len = strlen(greeting) + 1;

    const ssize_t cntWrite = rfid.WriteUART(greeting, len);
    const ssize_t cntRead = rfid.ReadUART();


    std::cout << cntWrite << " bytes written\n";
    std::cout << cntRead << " bytes read\n";



    const char* data = (const char*)rfid.GetBuffer();

    std::cout << data;

    std::cout << "end\n";
    return 0;
}
