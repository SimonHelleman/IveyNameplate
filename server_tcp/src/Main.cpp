#include <string>
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

    constexpr uint16_t WEB_API_PORT = 18080;
    constexpr uint16_t SERVER_PORT = 25565;

    nameplate::Server s(SERVER_PORT, database);
    s.Start();

    crow::SimpleApp webAPI;
    auto tmp = webAPI.port(WEB_API_PORT).multithreaded().run_async();

    CROW_ROUTE(webAPI, "/")([]() {
        return "hello world";
    });

    CROW_ROUTE(webAPI, "/nameplate/startpoll")([&](const crow::request& req) {
        const std::string param = req.url_params.get("num_options");

        try
        {
            const int numOptions = std::stoi(param);

            if (numOptions < 1 || numOptions > 4)
            {
                throw std::out_of_range("domain: [1, 4]");
            }

            nameplate::Message msg(nameplate::PacketType::StartPoll);
            msg.Push(&numOptions, sizeof(numOptions));
            s.BroadcastMessage(msg);

        }
        catch (const std::invalid_argument& e)
        {
            ERROR("[WebAPI] Could not convert to int on route /nameplate/startpoll");
            return crow::response(400, "Could not conver parameter to int");
        }
        catch (const std::out_of_range& e)
        {
            ERROR("[WebAPI] Parameter out of range on route /nameplate/startpoll");
            return crow::response(400, "parameter out of range");
        }

        return crow::response(200);
    });

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