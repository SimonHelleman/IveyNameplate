#pragma once
#include <functional>
#include <memory>
#include <thread>
#include <unordered_map>

#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>

#include "PlatformFactory.h"
#include "Student.h"

namespace nameplate {

class Nameplate
{
public:
    enum class State
    {
        Idle,
        Name,
        Poll,
        CreateUser,
        CreateGuest
    };

public:
    // gonna need to add additional constructors for different types of config
    // Can't think of a better solution for now
    Nameplate(const PlatformConfig<TCPNetworkConfig>& config);

    void Run();

    void TestHandler(const Message& msg);

private:
    struct StateHandler
    {
        std::function<void()> init;
        std::function<void()> periodic;
    };


private:
    void IdleStateInit();
    void IdleStatePeriodic();

    void NameStateInit();
    void NameStatePeriodic();
    
private:
    const std::unique_ptr<Display> m_frontDisplay;
    const std::unique_ptr<Display> m_rearDisplay;
    const std::unique_ptr<Network> m_network;
    const std::unique_ptr<RFID> m_card;

    bool m_stateTransition;
    State m_currentState;

    bool m_readId;
    uint32_t m_currentId;

    Student m_currentStudent;

    std::unique_ptr<std::thread> m_cardThread;

    const std::unordered_map<State, StateHandler> m_stateHandler = {
        { State::Idle, { std::bind(&Nameplate::IdleStateInit, this), std::bind(&Nameplate::IdleStatePeriodic, this) } },
        { State::Name, { std::bind(&Nameplate::NameStateInit, this), std::bind(&Nameplate::NameStatePeriodic, this) } }
    };
};

}
