#include <iostream>
#include <thread>
#include <chrono>
#include <crow.h>

#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>

#include "Server.h"
#include "DatabaseConnection.h"
#include "Secret.h"

bool g_shouldEnd = false;

void ConsoleThread()
{
    while (!g_shouldEnd)
    {
        std::string consoleIn;
        std::cin >> consoleIn;

        if (consoleIn == "stop")
        {
            g_shouldEnd = true;
        }
    }
}

int main()
{
    std::cout << "hello world\n";


    auto& logger = scrolls::Logger::Get();
    logger.AddOutput(scrolls::LogLevel::Debug | scrolls::LogLevel::Info, scrolls::Logger::StandardOutput);
    logger.AddOutput(scrolls::LogLevel::Warning | scrolls::LogLevel::Error, scrolls::Logger::StandardErrorOutput);

    nameplate::DatabaseConnection database("nameplate_db", "postgres", DB_PASSWORD);
    
    std::thread consoleListen(ConsoleThread);

    crow::SimpleApp webAPI;

    CROW_ROUTE(webAPI, "/")([]() {
        return "hello world";
    });

    constexpr uint16_t WEB_API_PORT = 18080;
    constexpr uint16_t SERVER_PORT = 25565;

    auto tmp = webAPI.port(WEB_API_PORT).multithreaded().run_async();

    nameplate::Server s(SERVER_PORT, database);
    s.Start();

    while (!g_shouldEnd)
    {
        s.HandleMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (consoleListen.joinable())
    {
        consoleListen.join();
    }

    s.Stop();
    webAPI.stop();

    return 0;
}