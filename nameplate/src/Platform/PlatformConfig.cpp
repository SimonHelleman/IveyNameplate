#include "../PlatformFactory.h"

#ifdef NAMEPLATE_PLATFORM_ASIO
#include "ASIO/Client.h"
#endif

namespace nameplate
{

#ifdef NAMEPLATE_PLATFORM_ASIO
template<>
PlatformConfig<TCPNetworkConfig> GetPlatformConfig()
{
#ifdef NAMEPLATE_PLATFORM_SFML
    constexpr unsigned int WINDOW_WIDTH = 1920;
    constexpr unsigned int WINDOW_HEIGHT = 720;
#endif

#ifdef NAMEPLATE_PLATFORM_WINDOWS
    const char* serialPort = "COM3"; // This is machine and USB port specific make runtime if possible
    constexpr unsigned int SERIAL_BAUD_RATE = 9600;
#endif

    const TCPNetworkConfig NETWORK_CONFIG = { "127.0.0.1", 25565 };
    return PlatformConfig<TCPNetworkConfig>(WINDOW_WIDTH, WINDOW_HEIGHT, NETWORK_CONFIG, serialPort, SERIAL_BAUD_RATE);
}
#endif

}