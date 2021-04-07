#include "gtest/gtest.h"
#include "src/state/ibof_state_event.h"
#include "src/state/ibof_state_manager.h"
#include "src/logger/logger.h"
#include <string>
#include <iostream>
#include <condition_variable>

using namespace ibofos;
using namespace std;

static condition_variable cv;
static mutex m;

static StateContext currState = {"default", State::OFFLINE};
static StateContext expState = {"default", State::OFFLINE};

class IBoFStateTester : public testing::Test, StateEvent
{

protected:
    static void SetUpTestCase()
    {
        cout << "setup" << endl;
    }

    static void TearDownTestCase()
    {
    }

    virtual void SetUp()
    {
        StateManager::Instance()->AddSubscriber(this);
    }
    virtual void TearDown()
    {
        StateManager::Instance()->RemoveSubscriber(this);
        StateManager::Instance()->ResetContext();
    }

    void StateChanged(StateContext prev, StateContext next)
    {
        if (currState != next)
        {
            IBOF_TRACE_WARN(1111, "STATECHANGED {} -> {}", prev.ToString(), next.ToString());
            currState = next;
            cv.notify_all();

            sleep(0.5);
        }
    }
};

TEST_F(IBoFStateTester, NormalOnState)
{
    //OFFLINE -> DIAGNOSIS -> NORMAL
    //START DIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //ENDDIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::NORMAL));
        StateManager::Instance()->RemoveContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    StateManager::Instance()->ResetContext();

    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [] { return currState.GetState() == State::OFFLINE; });
    
    int res = 1;
    EXPECT_EQ(1, res);
}

TEST_F(IBoFStateTester, PauseOnState)
{
    //OFFLINE -> DIAGNOSIS -> PAUSE -> NORMAL
    string uid = "testid";
    //START DIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //PAUSE
    {
        StateManager::Instance()->SubmitContext(StateContext("jornal_mgr", State::PAUSE));
    }

    //ENDPAUSE
    {
        StateManager::Instance()->RemoveContext(StateContext("jornal_mgr", State::PAUSE));
    }

    //ENDDIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::NORMAL));
        StateManager::Instance()->RemoveContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    StateManager::Instance()->ResetContext();

    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [] { return currState.GetState() == State::OFFLINE; });

    int res = 1;
    EXPECT_EQ(1, res);
}

TEST_F(IBoFStateTester, StopOnState)
{
    //OFFLINE -> DIAGNOSIS -> STOP -> NORMAL
    string uid = "testid";
    //START DIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //STOP
    {
        StateManager::Instance()->SubmitContext(StateContext("array", State::STOP));
    }

    //ENDSTOP
    {
        StateManager::Instance()->RemoveContext(StateContext("array", State::STOP));
    }

    //ENDDIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::NORMAL));
        StateManager::Instance()->RemoveContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    StateManager::Instance()->ResetContext();

    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [] { return currState.GetState() == State::OFFLINE; });

    int res = 1;
    EXPECT_EQ(1, res);
}

TEST_F(IBoFStateTester, NormalToPause)
{
    //OFFLINE -> DIAGNOSIS -> NORMAL -> PAUSE
    string uid = "testid";
    //START DIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //ENDDIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::NORMAL));
        StateManager::Instance()->RemoveContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //NOW NORMAL
    //PAUSE
    {
        StateManager::Instance()->SubmitContext(StateContext("rebuilder", State::PAUSE));
    }

    //ENDPAUSE
    {
        StateManager::Instance()->RemoveContext(StateContext("rebuilder", State::PAUSE));
    }

    StateManager::Instance()->ResetContext();

    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [] { return currState.GetState() == State::OFFLINE; });

    int res = 1;
    EXPECT_EQ(1, res);
}

TEST_F(IBoFStateTester, NormalToBusy)
{
    //OFFLINE -> DIAGNOSIS -> NORMAL -> PAUSE
    string uid = "testid";
    //START DIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //ENDDIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::NORMAL));
        StateManager::Instance()->RemoveContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //NOW NORMAL
    //BUSY
    {
        StateManager::Instance()->SubmitContext(StateContext("rebuilder", State::BUSY));
    }

    //ENDBUSY
    {
        StateManager::Instance()->RemoveContext(StateContext("rebuilder", State::BUSY));
    }

    StateManager::Instance()->ResetContext();

    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [] { return currState.GetState() == State::OFFLINE; });

    int res = 1;
    EXPECT_EQ(1, res);
}

TEST_F(IBoFStateTester, BusyToPause)
{
    //OFFLINE -> DIAGNOSIS -> NORMAL -> BUSY -> PAUSE
    string uid = "testid";
    //START DIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //ENDDIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::NORMAL));
        StateManager::Instance()->RemoveContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //NOW NORMAL
    //BUSY
    {
        StateManager::Instance()->SubmitContext(StateContext("rebuilder", State::BUSY));
    }

    //PAUSE
    {
        StateManager::Instance()->SubmitContext(StateContext("rebuilder", State::PAUSE));
    }

    //ENDPAUSE
    {
        StateManager::Instance()->RemoveContext(StateContext("rebuilder", State::PAUSE));
    }

    //ENDBUSY
    {
        StateManager::Instance()->RemoveContext(StateContext("rebuilder", State::BUSY));
    }

    StateManager::Instance()->ResetContext();

    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [] { return currState.GetState() == State::OFFLINE; });

    int res = 1;
    EXPECT_EQ(1, res);
}

TEST_F(IBoFStateTester, PauseDuplicated)
{
    //OFFLINE -> DIAGNOSIS -> NORMAL -> BUSY -> PAUSE -> BUSY
    string uid = "testid";
    //START DIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //ENDDIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::NORMAL));
        StateManager::Instance()->RemoveContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //NOW NORMAL

    //BUSY
    {
        StateManager::Instance()->SubmitContext(StateContext("gc", State::BUSY));
    }

    //PAUSE1
    {
        StateManager::Instance()->SubmitContext(StateContext("rebuilder1", State::PAUSE));
    }

    //PAUSE2
    {
        StateManager::Instance()->SubmitContext(StateContext("rebuilder2", State::PAUSE));
    }

    //ENDPAUSE1
    {
        StateManager::Instance()->RemoveContext(StateContext("rebuilder1", State::PAUSE));
    }

    //ENDPAUSE2
    {
        StateManager::Instance()->RemoveContext(StateContext("rebuilder2", State::PAUSE));
    }

    //ENDBUSY
    {
        StateManager::Instance()->RemoveContext(StateContext("gc", State::BUSY));
    }

    StateManager::Instance()->ResetContext();

    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [] { return currState.GetState() == State::OFFLINE; });

    int res = 1;
    EXPECT_EQ(1, res);
}

TEST_F(IBoFStateTester, TestAndSetTrue)
{
    //OFFLINE -> DIAGNOSIS -> NORMAL ->PAUSE -> OFFLINE
    string uid = "testid";
    //START DIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //ENDDIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::NORMAL));
        StateManager::Instance()->RemoveContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //TRY TO PAUSE TO PREPARE EXIT
    {
        expState = StateContext("unmount_handler", State::PAUSE);
        bool ret = StateManager::Instance()->TestAndSet(expState);
        if (ret == true)
        {
            std::unique_lock<std::mutex> lock(m);
            cv.wait(lock, [] { return expState == currState; });            
        }

        StateManager::Instance()->ResetContext();

        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [] { return currState.GetState() == State::OFFLINE; });

        EXPECT_EQ(true, ret);
    }
}

TEST_F(IBoFStateTester, TestAndSetFalse)
{
    //OFFLINE -> DIAGNOSIS -> NORMAL ->PAUSE -> X -> OFFLINE
    string uid = "testid";
    //START DIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //ENDDIAGNOSIS
    {
        StateManager::Instance()->SubmitContext(StateContext("main_handler", State::NORMAL));
        StateManager::Instance()->RemoveContext(StateContext("main_handler", State::DIAGNOSIS));
    }

    //PAUSE1
    {
        StateManager::Instance()->SubmitContext(StateContext("rebuilder1", State::PAUSE));
    }

    //TRY TO PAUSE TO PREPARE EXIT
    {
        bool ret = StateManager::Instance()->TestAndSet(StateContext("main_handler", State::PAUSE));
        StateManager::Instance()->ResetContext();

        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [] { return currState.GetState() == State::OFFLINE; });

        EXPECT_EQ(false, ret);
    }
}