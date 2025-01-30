#pragma once
#include <memory>
#include "../../RFID.h"

namespace nameplate
{    
class EccelRFID : public RFID
{
public:
    EccelRFID(const char* serialPort);
    ~EccelRFID();

    uint32_t GetId() const override;

private:

    int ReadUART() const;
    void WriteUART(const char* data) const;


private:
    const char* m_port;
    int m_serial;
    std::unique_ptr<char[]> m_serialBuf;

private:
    constexpr size_t UART_BUFFER_SZ = 1024;
};
}