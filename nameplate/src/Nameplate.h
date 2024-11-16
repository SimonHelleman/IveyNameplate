#pragma once
#include "Display.h"

namespace nameplate {

class Nameplate
{
public:
    Nameplate(Display& front, Display& rear);

    void Update();

private:
    Display& m_frontDisplay;
    Display& m_rearDisplay;
};

}
