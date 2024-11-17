#include <iostream>
#include "DisplaySFML.h"
#include "../../Nameplate.h"

int main()
{
    std::cout << "hello world\n";

    sf::Font font;
    if (!font.loadFromFile("resources/Roboto-Regular.ttf"))
    {
        std::cerr << "font loading error\n";
        return -1;
    }

    nameplate::DisplaySFML::Config config(20, font);
    nameplate::DisplaySFML front(1920, 1080, "nameplate_front", config);
    nameplate::DisplaySFML rear(1920, 1080, "nameplate_rear", config);

    nameplate::Nameplate nameplate(front, rear);

    nameplate.Run();
    return 0;
}