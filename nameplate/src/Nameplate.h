#pragma once
#include <memory>

#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>

#include "PlatformFactory.h"

namespace nameplate {

class Nameplate
{
public:
    // gonna need to add additional constructors for different types of config
    // Can't think of a better solution for now
    Nameplate(const PlatformConfig<TCPNetworkConfig>& config);

    void Run();

    void TestHandler(const Message& msg);
    
private:
    const std::unique_ptr<Display> m_frontDisplay;
    const std::unique_ptr<Display> m_rearDisplay;
    const std::unique_ptr<Network> m_network;
};

}
