#include <iostream>
#include "SFMLDisplay.h"
#include "../../Nameplate.h"

int main()
{
    std::cout << "hello world\n";

    sf::Font font;
    if (!font.loadFromFile("resources/Roboto-Regular.ttf"))
    {
        return -1;
    }

    nameplate::SFMLDisplay::Config config(20, font);
    nameplate::SFMLDisplay front(1920, 1080, "nameplate_front", config);
    nameplate::SFMLDisplay rear(1920, 1080, "nameplate_rear", config);

    nameplate::Nameplate nameplate(front, rear);

    nameplate.Update();
/*
    sf::RenderWindow window = sf::RenderWindow({ 1920u, 1080u }, "Hello");
    window.setFramerateLimit(30);

    sf::Font font;
    sf::Text txt;

    txt.setFont(font);
    txt.setString("Simon Helleman");
    txt.setCharacterSize(48);
    txt.setFillColor(sf::Color::Black);


    std::cout << std::filesystem::current_path() << '\n';


    while (window.isOpen())
    {
        for (sf::Event event; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.clear(sf::Color::White);
        window.draw(txt);
    

        window.display();
    }
    return 0;
*/
}