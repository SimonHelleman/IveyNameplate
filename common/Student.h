#pragma once
#include <stdint.h>
#include <string>

namespace nameplate
{
struct Student
{
    uint32_t id;
    std::string firstName;
    std::string lastName;
};
}