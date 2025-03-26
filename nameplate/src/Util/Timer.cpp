#include "Timer.h"

namespace nameplate
{

Timer::Timer()
    : m_lastTime(std::chrono::steady_clock::now())
{
}

float Timer::Mark()
{
    const auto past = m_lastTime;
    m_lastTime = std::chrono::steady_clock::now();
    const std::chrono::duration<float> elapsedTime = m_lastTime - past; 
    return elapsedTime.count();
}

float Timer::Elapsed() const
{
    const auto now = std::chrono::steady_clock::now();
    const std::chrono::duration<float> elapsedTime = now - m_lastTime;
    return elapsedTime.count();
}

}