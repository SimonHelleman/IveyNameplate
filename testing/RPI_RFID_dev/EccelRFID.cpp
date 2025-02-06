#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdexcept>
#include "EccelRFID.h"



#include <stdio.h>


static void Hexdump(const void* data, size_t size)
{
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);

    for (size_t i = 0; i < size; ++i)
    {
        printf("%02x  ", bytes[i]);

        if (i % 16 == 0 && i > 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}


static const uint16_t s_tableCCITTCRC[256] = {  
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5,  
    0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b,  
    0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210,  
    0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,  
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c,  
    0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,  
    0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b,  
    0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,  
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6,  
    0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738,  
    0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5,  
    0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,  
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969,  
    0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96,  
    0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc,  
    0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,  
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03,  
    0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,  
    0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6,  
    0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,  
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,  
    0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb,  
    0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1,  
    0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,  
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c,  
    0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2,  
    0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb,  
    0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,  
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447,  
    0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,  
    0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2,  
    0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,  
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,  
    0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827,  
    0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c,  
    0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0,  
    0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d,  
    0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07,  
    0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,  
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba,  
    0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,  
    0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

static uint16_t GetCCITTCRC(const void* data, uint32_t size)
{
    const uint8_t* bytes = (const uint8_t*)data;

    if (size == 0)
    {
        return 0;
    }

    uint16_t crc = 0xffff;
    for (uint32_t i = 0; i < size; ++i)
    {
        uint16_t temp = (uint16_t)((crc >> 8) ^ bytes[i]) & 0x00ff;
        crc = s_tableCCITTCRC[temp] ^ (crc << 8);
    }

    return crc;
} 


namespace nameplate
{

EccelRFID::EccelRFID(const char* serialPort)
    : m_serial(0), m_serialBuf(std::make_unique<uint8_t[]>(UART_BUFFER_SZ)), m_serialBufferLen(0), m_responsePacket()
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

// TODO go back to size_t since throwing the except
size_t EccelRFID::ReadUART()
{
    memset(m_serialBuf.get(), 0, UART_BUFFER_SZ * sizeof(char));

    const ssize_t bytesRead = read(m_serial, m_serialBuf.get(), UART_BUFFER_SZ - 1);

    if (bytesRead == -1)
    {
        throw UARTError(UARTError::Type::Read);
    }

    m_serialBufferLen = bytesRead;

    return static_cast<size_t>(bytesRead);
}

size_t EccelRFID::WriteUART(const void* data, size_t dataLen) const 
{
    const ssize_t bytesWritten = write(m_serial, data, dataLen);

    if (bytesWritten == -1)
    {
        throw UARTError(UARTError::Type::Write);
    }

    return static_cast<size_t>(bytesWritten);
}

EccelRFID::Packet EccelRFID::SendCommand(Command command, const uint8_t* params, uint16_t paramsSize)
{
    std::vector<uint8_t> dataPayload;

    dataPayload.push_back(static_cast<uint8_t>(command));
    
    for (int i = 0; i < paramsSize; ++i)
    {
        dataPayload.push_back(params[i]);
    }
    
  
    const uint16_t crc = GetCCITTCRC(dataPayload.data(), static_cast<uint32_t>(dataPayload.size()));
    const uint8_t* crcBytes = reinterpret_cast<const uint8_t*>(&crc); // TODO do this the proper way even though this works

    dataPayload.push_back(crcBytes[0]);
    dataPayload.push_back(crcBytes[1]);

    const Packet packet(dataPayload.data(), static_cast<uint16_t>(dataPayload.size()));
    const uint8_t* packetPayload = packet.AsBytes().data();
    const size_t packetPayloadSz = packet.AsBytes().size();



    try
    {
        printf("writing: \n");
        Hexdump(packetPayload, packetPayloadSz);
        const size_t bytesWritten = WriteUART(packetPayload, packetPayloadSz);
        printf("%ld written\n", bytesWritten);
        ReadUART();
    }
    catch (const UARTError& error)
    {
        throw std::runtime_error(error.what());
    }

    std::vector<uint8_t> responseData(m_serialBufferLen);

    for (size_t i = 0; i < m_serialBufferLen; ++i)
    {
        responseData.push_back(m_serialBuf[i]);
    }
    printf("received all:\n");
    Hexdump(responseData.data(), responseData.size());

    const Packet response(std::move(responseData));

    //const std::vector<uint8_t> tmp(4);

    //const Packet response(tmp);
    return response;
}

EccelRFID::Packet::Packet(const void* data, uint16_t len)
{
    const uint16_t crc = GetCCITTCRC(data, len);

    const uint8_t lenHi = static_cast<uint8_t>((len >> 8) & 0xff);
    const uint8_t lenLo = static_cast<uint8_t>(len & 0xff);

    const uint8_t crcHi = static_cast<uint8_t>((crc >> 8) & 0xff);
    const uint8_t crcLo = static_cast<uint8_t>(len & 0xff);

    m_packet.push_back(0x02);
    m_packet.push_back(lenLo);
    m_packet.push_back(lenHi);
    m_packet.push_back(crcLo);
    m_packet.push_back(crcHi);


    const uint8_t* payload = (const uint8_t*)data;
    for (int i = 0; i < len; ++i)
    {
        m_packet.push_back(payload[i]);
    }
}

EccelRFID::Packet::Packet(const std::vector<uint8_t>& packetData)
    : m_packet(packetData)
{
}

EccelRFID::Packet::Packet(std::vector<uint8_t>&& packetData)
    : m_packet(packetData)
{
}

std::vector<uint8_t> EccelRFID::Packet::Data() const
{
    const uint8_t lenLow = m_packet[2];
    const uint8_t lenHigh = m_packet[3];

    const uint16_t len = (lenHigh << 8) | lenLow;
    std::vector<uint8_t> ret(len);

    for (int i = 0; i < len; ++i)
    {
        ret[i] = m_packet[i + 5];
    }

    return ret;
}
}

#include <iostream>

using namespace nameplate;


int main()
{
    EccelRFID rfid("/dev/ttyAMA0");

    auto packet = rfid.SendCommand(EccelRFID::Command::Read, nullptr, 0);
    auto bytes = packet.AsBytes();

    std::cout << "received data:\n";
    Hexdump(bytes.data(), bytes.size());

    /*

    const uint8_t wr[] = { 0x58, 0x02, 0x01, 0x00, 0xab };
    auto packet = rfid.SendCommand(EccelRFID::Command::Write, wr, 5);
    auto bytes = packet.AsBytes();

    std::cout << "received data:\n";
    Hexdump(bytes.data(), bytes.size());

    const uint8_t rd[] = { 0x58, 0x02 };
    packet = rfid.SendCommand(EccelRFID::Command::Read, rd, 2);
    bytes = packet.AsBytes();

    std::cout << "received data\n";
    Hexdump(bytes.data(), bytes.size());

    */

    std::cout << "clean exit\n";
    return 0;
}
