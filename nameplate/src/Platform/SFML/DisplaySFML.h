#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "../../Display.h"
#include "../../Touch.h"

namespace nameplate
{

// Multiinheritance... I know... bad, but Touch is a pure interface
// so this is actually something you can do in Java
class DisplaySFML : public Display, public Touch
{
public:
    struct Config
    {
        Config(unsigned int updateRate, const sf::Font& font)
            : updateRate(updateRate), textFont(font)
        {
        }
        unsigned int updateRate;
        const sf::Font textFont;
    };

public:
    DisplaySFML(unsigned int width, unsigned int height, const char* name, const Config& config);

    void HandleEvents() override;
   
    // Why did I use int and thne switch to float??? idk
    void DrawText(unsigned int posX, unsigned int posY, unsigned int size, RGBA color, const std::string& text) override;
    void FillRectangle(
        const float posX, const float posY, const float width, const float height,
        const RGBA fill, const RGBA outlineColor, const float outlineThickness
    ) override;

    void DrawReaction(
        const unsigned int posX, const unsigned int posY,
        const Reaction r
    ) override;

    void Clear(RGB color) override;
    void Show() override;

    std::pair<int, int> GetTouchPos() const override;
    bool IsTouched() const override;


    // Better solution for htis needed
    const unsigned int ReactionWidth(const Reaction r) const override
    {
        switch (r)
        {
        case Reaction::ThumbsUp:
            return m_thumbsUp.getGlobalBounds().width;
        case Reaction::ThumbsDown:
            return m_thumbsDown.getGlobalBounds().width;
        }
    }

    // Better solution for htis needed
    const unsigned int ReactionHeight(const Reaction r) const override
    {
        switch (r)
        {
        case Reaction::ThumbsUp:
            return m_thumbsUp.getGlobalBounds().height;
        case Reaction::ThumbsDown:
            return m_thumbsDown.getGlobalBounds().height;
        }
    }


private:
    sf::RenderWindow m_window;
    sf::Texture m_thumbsUpTex;
    sf::Texture m_thumbsDownTex;
    sf::Sprite m_thumbsUp;
    sf::Sprite m_thumbsDown;
    const Config m_config;
};

}