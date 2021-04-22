
#include "src/thread/Thread.cpp"
#include "src/thread/Thread.h"

class ThreadTest : public ::testing::Test
{
public:
    thread::OneTimeThread* one_time_thread{nullptr};
    thread::PeriodicThread* periodic_thread{nullptr};

protected:
    ThreadTest()
    {
        one_time_thread = new thread::OneTimeThread{};
        periodic_thread = new thread::PeriodicThread{};
    }
    ~ThreadTest()
    {
        if (nullptr != one_time_thread)
        {
            delete one_time_thread;
            one_time_thread = nullptr;
        }
        if (nullptr != periodic_thread)
        {
            delete periodic_thread;
            periodic_thread = nullptr;
        }
    }
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
    }
};
