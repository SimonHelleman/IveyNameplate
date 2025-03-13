#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "../../Display.h"

namespace nameplate
{
class DisplaySFML : public Display
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
    void Clear(RGB color) override;
    void Show() override;


private:
    sf::RenderWindow m_window;
    const Config m_config;
};

}