#pragma once
#include <stdint.h>
#include <string>

namespace nameplate
{
struct Student
{
    Student() = default;

    Student(uint32_t id, const std::string& lastName, const std::string& firstName)
        : id(id), lastName(lastName), firstName(firstName)
    {}

    uint32_t id;
    std::string lastName;
    std::string firstName;
};
}