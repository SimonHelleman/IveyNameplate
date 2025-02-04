#pragma once
#include <memory>
#include <vector>

namespace nameplate
{    
class EccelRFID
{
public:
    enum class Command: uint8_t
    {
        Dummy = 0x00,
        Write = 0x01,
        Read = 0x02,
        EnterSleep = 0x03,
        Reset = 0x04,
        SetBaud = 0x05,
        SetDataType = 0x06,
        SetHeaderType = 0x07,
        SetIOState = 0x08,
        ReadIOState = 0x09,
        SetIOInterrupt = 0x0a,
        MeasureVoltage = 0x0b,
        MeasureDieTemp = 0x0c,
        SetIDACCurrent = 0x0d,
        EnableComparator = 0x0f,
        EnablePWM = 0x10,
        SetAESLimit = 0x11,
        SetAESKey = 0x12,
        ReadAESInitVector = 0x13,
        ReadAESKey = 0x14
    };

    enum class Register: uint16_t
    {
        Result = 0x0000,
        Command = 0x0001,
        CommandParams = 0x0002,
        TagUID = 0x0014,
        TagType = 0x001e,
        TagUIDSize = 0x001f,
        DataBuffer = 0x0020,
        Password = 0x0120,
        AESInitVec0 = 0x0128,
        AESInitVec1 = 0x0138,
        AESKey0 = 0x0148,
        AESKey1 = 0x0158,
        UserMemoryStart = 0x0258
    }; // And more for passwords.... add if needed

public:
    EccelRFID(const char* serialPort);
    ~EccelRFID();

    uint32_t GetId() const;

    // temp
    uint8_t* GetBuffer() const
    {
        return m_serialBuf.get();
    }

//private:
public:
    ssize_t ReadUART() const;
    ssize_t WriteUART(const void* data, size_t dataLen) const;

private:

    class Packet
    {
    public:
        Packet(const void* data, uint16_t len);
        Packet(const std::vector<uint8_t> packetData);

        std::vector<uint8_t> Data() const;
        
        const std::vector<uint8_t>& AsBytes() const
        {
            return m_packet;
        }

    private:
        std::vector<uint8_t> m_packet;
    };
    //static std::vector<uint8_t> MakePacket(const void* data, uint16_t len);

private:
    int m_serial;
    std::unique_ptr<uint8_t[]> m_serialBuf;

private:
    static constexpr size_t UART_BUFFER_SZ = 1024;
};
}
