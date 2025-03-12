#pragma once
#include <memory>
#include "Display.h"
#include "Network.h"
#include "RFID.h"

namespace nameplate
{

template<typename NetConfig>
struct PlatformConfig
{
    const unsigned int displayWidth;
    const unsigned int displayHeight;
    const NetConfig networkConfig;
    const char* serialPort;
    const unsigned int serialBaudRate;

    PlatformConfig(
        const unsigned int width, const unsigned int height,
        const NetConfig& config,
        const char* serialPort, const unsigned int serialBaudRate 
    ) : displayWidth(width), displayHeight(height), networkConfig(config), 
        serialPort(serialPort), serialBaudRate(serialBaudRate)
    {
    }
};

template<typename NetConfig>
PlatformConfig<NetConfig> GetPlatformConfig();

class PlatformFactory
{
public:
    static std::unique_ptr<Display> CreateDisplay(const unsigned int width, const unsigned int height, const char* name);
    
    template<typename T>
    static std::unique_ptr<Network> CreateNetwork(const T& config);

    static std::unique_ptr<RFID> CreateRFID(const char* serialPort, const unsigned int baudRate);
};
}
