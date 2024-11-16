#include "Nameplate.h"

namespace nameplate
{

Nameplate::Nameplate(Display& front, Display& rear)
    : m_frontDisplay(front), m_rearDisplay(rear)
{
}

void Nameplate::Update()
{
    while (true)
    {
        m_frontDisplay.HandleEvents();
        m_rearDisplay.HandleEvents();
        m_frontDisplay.Clear({ 255, 255, 255 });
        m_rearDisplay.Clear( { 255, 255, 255 });

        m_frontDisplay.DrawText(20, 20, 48, {}, "Simon Helleman");
        
        m_frontDisplay.Show();
        m_rearDisplay.Show();
    }
}
    
}
