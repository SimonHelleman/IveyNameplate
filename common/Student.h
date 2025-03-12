#pragma once
#include <stdint.h>
#include <string>

namespace nameplate
{
struct Student
{
    uint32_t id;
    std::string lastName;
    std::string firstName;
};
}