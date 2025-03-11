#pragma once
#include <memory>
#include "Display.h"
#include "Network.h"

namespace nameplate
{

template<typename NetConfig>
struct PlatformConfig
{
    const unsigned int displayWidth;
    const unsigned int displayHeight;
    const NetConfig networkConfig;

    PlatformConfig(unsigned int width, unsigned int height, const NetConfig& config)
        : displayWidth(width), displayHeight(height), networkConfig(config) {}
};

template<typename NetConfig>
PlatformConfig<NetConfig> GetPlatformConfig();

class PlatformFactory
{
public:
    static std::unique_ptr<Display> CreateDisplay(const unsigned int width, const unsigned int height, const char* name);
    
    template<typename T>
    static std::unique_ptr<Network> CreateNetwork(const T& config);
};
}
