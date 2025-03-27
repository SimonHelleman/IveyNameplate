#pragma once
#include <stdint.h>

namespace nameplate
{

enum class QuietMode : uint8_t
{
    On,
    Off,
    Partial
};

enum class AnonymousMode : uint8_t
{
    On,
    Off
};

}