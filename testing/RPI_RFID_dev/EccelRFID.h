#pragma once
#include <memory>
#include <string>
#include <vector>
#include <exception>

namespace nameplate
{
class UARTError : public std::exception
{
public:
    enum class Type
    {
        Read,
        Write
    };
public:
    UARTError(Type type)
    {
        m_what = "[UART]There was an error ";
        m_what += type == Type::Read ? "reading from " : "writing to ";
        m_what += "UART.";
    }


    const char* what() const noexcept override
    {
        return m_what.c_str();
    }
private:
    std::string m_what;
};


// Datasheet: https://www.eccel.co.uk/wp-content/downloads/RFID-B1-User-Manual.pdf
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

    enum class Response: uint8_t
    {
        ACK = 0x0,
        InvalidCommand = 0x01,
        InvalidCommandParam = 0x02,
        ProtocolError = 0x03,
        MemoryError = 0x04,
        SystemError = 0x05,
        ModuleTimeout = 0x06,
        Overflow = 0x07,
        AsyncPacket = 0x08,
        Busy = 0x09,
        SystemStart = 0x0a
    };

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

    class Packet
    {
    public:
        Packet(const void* data, uint16_t len);
        Packet(const std::vector<uint8_t>& packetData);
        Packet(std::vector<uint8_t>&& packetData);


        std::vector<uint8_t> Data() const;
        
        const std::vector<uint8_t>& AsBytes() const
        {
            return m_packet;
        }

    private:
        std::vector<uint8_t> m_packet;
    };


//private:
public:
    size_t ReadUART();
    size_t WriteUART(const void* data, size_t dataLen) const;

    Packet SendCommand(Command command, const uint8_t* params, uint16_t paramsSize);

private:
    int m_serial;
    std::unique_ptr<uint8_t[]> m_serialBuf;
    size_t m_serialBufferLen;
    std::unique_ptr<Packet> m_responsePacket;

private:
    static constexpr size_t UART_BUFFER_SZ = 1024;
};
}
