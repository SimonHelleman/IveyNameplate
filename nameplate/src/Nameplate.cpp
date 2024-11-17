#include "Nameplate.h"

namespace nameplate
{

Nameplate::Nameplate(Display& front, Display& rear)
    : m_frontDisplay(front), m_rearDisplay(rear)
{
}

void Nameplate::Run()
{
    while (true)
    {
        m_frontDisplay.HandleEvents();
        m_rearDisplay.HandleEvents();
        m_frontDisplay.Clear({ 255, 255, 255 });
        m_rearDisplay.Clear( { 255, 255, 255 });

        const unsigned int centerX = m_frontDisplay.Width() / 2;
        const unsigned int centerY = m_frontDisplay.Height() / 2;
        constexpr unsigned int NAME_FONT_SIZE = 216;
        const char* name = "Kyle Maclean";
        const char* course = "BUS 1299E";

        m_frontDisplay.DrawText(centerX, centerY, NAME_FONT_SIZE, {}, name);

        m_rearDisplay.DrawText(centerX, centerY, NAME_FONT_SIZE / 2, {}, course);
        m_rearDisplay.DrawText(centerX, centerY + 100, NAME_FONT_SIZE / 3, {}, name);
        
        m_frontDisplay.Show();
        m_rearDisplay.Show();
    }
}
    
}
