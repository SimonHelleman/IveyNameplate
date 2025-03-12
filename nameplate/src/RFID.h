#pragma once
#include <stdint.h>

namespace nameplate
{

class RFID
{
public:
    virtual ~RFID() = default;
    virtual uint32_t GetId() = 0;
};

}