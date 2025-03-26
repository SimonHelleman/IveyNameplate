#pragma once
#include <functional>
#include <memory>
#include <thread>
#include <unordered_map>

#define SCROLLS_USE_LOGGER_MACROS
#include <Scrolls.h>

#include "PlatformFactory.h"
#include "Touch.h"
#include "Student.h"
#include "VirtualKeyboard.h"

namespace nameplate {

class Nameplate
{
public:
    enum class State
    {
        Idle,
        Name,
        Poll,
        CreateStudentLastName,
        CreateStudentFirstName,
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

    // lazy substates... just treat them as full states
    void CreateStudentLastNameInit();
    void CreateStudentLastNamePeriodic();

    void CreateStudentFirstNameInit();
    void CreateStudentFirstNamePeriodic();

    void PollStateInit();
    void PollStatePeriodic();
    
private:
    const std::unique_ptr<Display> m_frontDisplay;
    const std::unique_ptr<Display> m_rearDisplay;
    const std::unique_ptr<Network> m_network;
    const std::unique_ptr<RFID> m_card;
    const Touch* m_touch;

    VirtualKeyboard m_keyboard;
    std::string m_inputLastName;
    std::string m_inputFirstName;

    bool m_stateTransition;
    State m_currentState;

    bool m_readId;
    uint32_t m_currentId;

    Student m_currentStudent;

    bool m_reactionSelected = false;
    bool m_reactionSent = false;

    int m_numPollOptions;
    int m_selectedPollOption;


    std::unique_ptr<std::thread> m_cardThread;

    const std::unordered_map<State, StateHandler> m_stateHandler = {
        { State::Idle, { std::bind(&Nameplate::IdleStateInit, this), std::bind(&Nameplate::IdleStatePeriodic, this) } },
        { State::Name, { std::bind(&Nameplate::NameStateInit, this), std::bind(&Nameplate::NameStatePeriodic, this) } },
        { State::CreateStudentLastName, { std::bind(&Nameplate::CreateStudentLastNameInit, this), std::bind(&Nameplate::CreateStudentLastNamePeriodic, this) } },
        { State::CreateStudentFirstName, { std::bind(&Nameplate::CreateStudentFirstNameInit, this), std::bind(&Nameplate::CreateStudentFirstNamePeriodic, this) } },
        { State::Poll, { std::bind(&Nameplate::PollStateInit, this), std::bind(&Nameplate::PollStatePeriodic, this) } },
    };
};

}
