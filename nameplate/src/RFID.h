#pragma once
#include <stdint.h>

namespace nameplate
{

class RFID
{
public:
    virtual uint32_t GetId() const = 0;
};

}