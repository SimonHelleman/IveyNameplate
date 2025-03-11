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

    const TCPNetworkConfig NETWORK_CONFIG = { "127.0.0.1", 25565 };
    return PlatformConfig<TCPNetworkConfig>(WINDOW_WIDTH, WINDOW_HEIGHT, NETWORK_CONFIG);
}
#endif

}