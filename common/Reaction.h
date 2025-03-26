#pragma once
#include <stdint.h>

enum class Reaction : uint8_t
{
    None = 0,
    ThumbsUp,
    ThumbsDown,
    RaiseHand
};