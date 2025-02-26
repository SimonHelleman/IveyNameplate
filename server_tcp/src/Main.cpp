#include <iostream>
#include <thread>
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
    database.DoesStudentExist(123);
    database.RecordAttendance(123, std::chrono::system_clock::now());

    std::thread consoleListen(ConsoleThread);

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "hello world";
    });


    auto tmp = app.port(18080).multithreaded().run_async();

    nameplate::Server s(25565);
    s.Start();

    while (!g_shouldEnd)
    {
        s.HandleMessages();
    }

    if (consoleListen.joinable())
    {
        consoleListen.join();
    }

    s.Stop();

    return 0;
}