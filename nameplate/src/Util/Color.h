#pragma once
#include <stdint.h>

namespace nameplate {

struct RGB
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    constexpr RGB()
        : r{}, g{}, b{}
    {
    }

    constexpr RGB(uint8_t r, uint8_t g, uint8_t b)
        : r(r), g(g), b(b)
    {
    }

    constexpr bool operator==(const RGB& other)
    {
        return r == other.r && g == other.g && b == other.b;
    }

    constexpr bool operator!=(const RGB& other)
    {
        return !(r == other.r && g == other.g && b == other.b);
    }
};

static_assert(sizeof(RGB) == 3);

struct RGBA
{
    union
    {
        struct
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
        uint32_t val;
    };

    constexpr RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff)
        : r(r), g(g), b(b), a(a)
    {
    }

    constexpr RGBA(RGB color)
        : r(color.r), g(color.g), b(color.b), a(0xff)
    {
    }
    
    constexpr RGBA(uint32_t val = 0xff000000)
        : val(val)
    {
    }
    
    constexpr bool operator==(const RGBA& other) const
    {
        return val == other.val;
    }
    
    constexpr bool operator!=(const RGBA& other) const
    {
        return val != other.val;
    }
};

static_assert(sizeof(RGBA) == 4);
}