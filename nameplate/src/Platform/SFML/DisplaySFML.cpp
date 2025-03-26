#ifdef NAMEPLATE_PLATFORM_SFML
#include <stdlib.h>
#include <filesystem>
#include <stdexcept>

#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>
#include "../../PlatformFactory.h"
#include "DisplaySFML.h"

namespace nameplate
{

std::unique_ptr<Display> PlatformFactory::CreateDisplay(const unsigned int width, const unsigned int height, const char* name)
{
    sf::Font font;
    if (!font.loadFromFile("resources/Roboto-Regular.ttf"))
    {
        const std::filesystem::path fontPath("resources/Roboto-Regular.ttf");
        ERROR("Could not load font from: " + fontPath.string());
        return std::unique_ptr<Display>();
    }

    constexpr unsigned int UPDATE_RATE_FPS = 20;
    DisplaySFML::Config config(UPDATE_RATE_FPS, font);

    return std::make_unique<DisplaySFML>(width, height, name, config);
}

DisplaySFML::DisplaySFML(unsigned int width, unsigned int height, const char* name, const Config& config)
    : Display(width, height, name), m_window({ width, height }, name), m_config(config)
{
    m_window.setFramerateLimit(config.updateRate);

    if (!m_thumbsUpTex.loadFromFile("resources/thumbs_up.jpg"))
    {
        throw std::runtime_error("Failed to open thumbs_up.jpg");
    }

    if (!m_thumbsDownTex.loadFromFile("resources/thumbs_down.jpg"))
    {
        throw std::runtime_error("Failed to open thumbs_down.jpg");
    }

    if (!m_raiseHandTex.loadFromFile("resources/raise_hand.jpg"))
    {
        throw std::runtime_error("Failed to open raise_hand.jpg");
    }
    
    m_thumbsUpTex.setSmooth(true);
    m_thumbsDownTex.setSmooth(true);
    m_raiseHandTex.setSmooth(true);

    m_thumbsUp.setTexture(m_thumbsUpTex);
    m_thumbsDown.setTexture(m_thumbsDownTex);
    m_raiseHand.setTexture(m_raiseHandTex);

    m_thumbsUp.setScale(150.0f / m_thumbsUpTex.getSize().x, 150.0f / m_thumbsUpTex.getSize().y);
    m_thumbsDown.setScale(150.0f / m_thumbsDownTex.getSize().x, 150.0f / m_thumbsDownTex.getSize().y);
    m_raiseHand.setScale(150.0f / m_raiseHandTex.getSize().x, 150.0f / m_raiseHandTex.getSize().y);
}


void DisplaySFML::HandleEvents()
{
    // End the app if a user closes the window
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
void DisplaySFML::DrawText(unsigned int posX, unsigned int posY, unsigned int size, RGBA color, const std::string& text)
{
    const sf::Color fill(color.r, color.g, color.b, color.a);
    
    // Conifure text
    sf::Text txt;
    txt.setString(text);
    txt.setFont(m_config.textFont);
    txt.setStyle(sf::Text::Bold);
    txt.setCharacterSize(size);
    txt.setFillColor(fill);

    // Find origin
    const sf::FloatRect rect = txt.getLocalBounds();
    txt.setOrigin(rect.left + rect.width / 2.0f, rect.top + rect.height / 2.0f);
    txt.setPosition(posX, posY);

    m_window.draw(txt);
}

void DisplaySFML::FillRectangle(
    const float posX, const float posY, const float width, const float height, 
    const RGBA fill, const RGBA outlineColor, const float outlineThickness
)
{
    sf::RectangleShape rect(sf::Vector2f(width, height));
    rect.setPosition(posX, posY);
    rect.setFillColor(sf::Color(fill.r, fill.g, fill.b, fill.a));
    rect.setOutlineColor(sf::Color(outlineColor.r, outlineColor.g, outlineColor.b, outlineColor.a));
    rect.setOutlineThickness(outlineThickness);

    m_window.draw(rect);
}

void DisplaySFML::DrawReaction(
    const unsigned int posX, const unsigned int posY,
    const Reaction r
)
{
    switch (r)
    {
    case Reaction::ThumbsUp:
        m_thumbsUp.setPosition(posX, posY);
        m_window.draw(m_thumbsUp);
        return;
    case Reaction::ThumbsDown:
        m_thumbsDown.setPosition(posX, posY);
        m_window.draw(m_thumbsDown);
        return;
    case Reaction::RaiseHand:
        m_raiseHand.setPosition(posX, posY);
        m_window.draw(m_raiseHand);
        return;
    }
}

void DisplaySFML::Clear(RGB color)
{
    m_window.clear({ color.r, color.g, color.b });
}

void DisplaySFML::Show()
{
    // Push image to display
    m_window.display();
}

std::pair<int, int> DisplaySFML::GetTouchPos() const
{
    const sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
    return std::pair<int, int>(mousePos.x, mousePos.y);
}

bool DisplaySFML::IsTouched() const
{
    return sf::Mouse::isButtonPressed(sf::Mouse::Left);
}

}

#endif