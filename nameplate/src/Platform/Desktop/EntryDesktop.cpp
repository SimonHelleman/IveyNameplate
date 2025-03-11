#ifdef NAMEPLATE_PLATFORM_DESKTOP
#include <iostream>
#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>
#include "../../Nameplate.h"
#include "../../Network.h"

int main()
{
    std::cout << "hello world\n";

    auto& logger = scrolls::Logger::Get();
    logger.AddOutput(scrolls::LogLevel::Debug | scrolls::LogLevel::Info, scrolls::Logger::StandardOutput);
    logger.AddOutput(scrolls::LogLevel::Warning | scrolls::LogLevel::Error, scrolls::Logger::StandardErrorOutput);
    
    const auto platformConfig = nameplate::GetPlatformConfig<nameplate::TCPNetworkConfig>();
    nameplate::Nameplate app(platformConfig);
    app.Run();

    return 0;
}
#endif