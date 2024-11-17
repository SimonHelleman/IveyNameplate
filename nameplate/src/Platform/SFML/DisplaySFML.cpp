#include <stdlib.h>
#include "DisplaySFML.h"

namespace nameplate
{

DisplaySFML::DisplaySFML(unsigned int width, unsigned int height, const char* name, const Config& config)
    : Display(width, height, name), m_window({ width, height }, name), m_config(config)
{
    m_window.setFramerateLimit(config.updateRate);
}


void DisplaySFML::HandleEvents()
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

// x, y is position of origin of text. 
void DisplaySFML::DrawText(unsigned int posX, unsigned int posY, unsigned int size, RGBA color, const char* text)
{
    const sf::Color fill(color.r, color.g, color.b, color.a);
    
    sf::Text txt;
    txt.setString(text);
    txt.setFont(m_config.textFont);
    txt.setCharacterSize(size);
    txt.setFillColor(fill);

    const sf::FloatRect rect = txt.getLocalBounds();
    txt.setOrigin(rect.left + rect.width / 2.0f, rect.top + rect.height / 2.0f);
    txt.setPosition(posX, posY);

    m_window.draw(txt);
}

void DisplaySFML::Clear(RGB color)
{
    m_window.clear({ color.r, color.g, color.b });
}

void DisplaySFML::Show()
{
    m_window.display();
}

}