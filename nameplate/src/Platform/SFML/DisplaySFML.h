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
    
    void DrawText(unsigned int posX, unsigned int posY, unsigned int size, RGBA color, const std::string& text) override;
    void FillRectangle(
        const float posX, const float posY, const float width, const float height,
        const RGBA fill, const RGBA outlineColor, const float outlineThickness
    ) override;

    void Clear(RGB color) override;
    void Show() override;

    std::pair<int, int> GetTouchPos() const override;
    bool IsTouched() const override;


private:
    sf::RenderWindow m_window;
    const Config m_config;
};

}