#include <stdlib.h>
#include "SFMLDisplay.h"

namespace nameplate
{

SFMLDisplay::SFMLDisplay(unsigned int width, unsigned int height, const char* name, const Config& config)
    : Display(width, height, name), m_window({ width, height }, name), m_config(config)
{
    m_window.setFramerateLimit(config.updateRate);
}


void SFMLDisplay::HandleEvents()
{
    // TODO figure a way to handle window close properly
    if (!m_window.isOpen())
    {
        exit(0);
    }

    for (sf::Event e; m_window.pollEvent(e);)
    {
        switch (e.type)
        {
        case sf::Event::Closed:
            m_window.close();
            break;
        }
    }
}

void SFMLDisplay::DrawText(unsigned int posX, unsigned int posY, unsigned int size, RGBA color, const char* text)
{
    sf::Text txt;

    txt.setFont(m_config.textFont);
    txt.setString(text);
    sf::Color fill(color.r, color.g, color.b, color.a);
    txt.setFillColor(fill);
    txt.setCharacterSize(size);
    txt.setPosition(posX, posY);

    m_window.draw(txt);
}

void SFMLDisplay::Clear(RGB color)
{
    m_window.clear({ color.r, color.g, color.b });
}

void SFMLDisplay::Show()
{
    m_window.display();
}

}