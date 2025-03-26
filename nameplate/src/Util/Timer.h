#pragma once
#include <chrono>

namespace nameplate
{

class Timer
{
public:
    Timer();
    float Mark();
    float Elapsed() const;
private:
    std::chrono::steady_clock::time_point m_lastTime;
};


}