#pragma once
#include <vector>

namespace nameplate
{

class Serializable
{
public:
    virtual std::vector<uint8_t> AsByteVector() = 0;
};

}