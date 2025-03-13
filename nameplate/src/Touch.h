#pragma once
#include <utility>

namespace nameplate
{
class Touch
{
public:
    virtual ~Touch() = default;
    virtual std::pair<int, int> GetTouchPos() const = 0;
    virtual bool IsTouched() const = 0;
};

}
