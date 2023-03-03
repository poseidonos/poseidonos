#include <gtest/gtest.h>
#include "src/lib/signal_mask.h"
#include <unistd.h>

namespace pos
{

TEST(SignalMask, MaskSignal_Simple)
{
    sigset_t oldset;
    SignalMask::MaskSignal(&oldset);
    SignalMask::RestoreSignal(&oldset);
    SignalMask::MaskSignal(SIGSEGV, &oldset);
    SignalMask::RestoreSignal(&oldset);
}

volatile uint32_t test_func_flag = 0;
volatile bool done = false;
void change_test_func_flag(int sig)
{
    test_func_flag = sig;
    done = true;
}

TEST(SignalMask, MaskSignal_Pending)
{
    signal(SIGSEGV, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGQUIT, change_test_func_flag);
    test_func_flag = 0;
    done = false;
    sigset_t oldset;
    SignalMask::MaskQuitSignal(&oldset);
    raise(SIGQUIT);
    usleep(500000);
    EXPECT_EQ(test_func_flag, 0);
    SignalMask::RestoreSignal(&oldset);
    usleep(500000);
    while(done == false)
    {
        usleep(1);
    }
    EXPECT_EQ(test_func_flag, SIGQUIT);
}

TEST(SignalMask, Nested_MaskSignal_And_Pending)
{
    signal(SIGSEGV, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGQUIT, change_test_func_flag);
    test_func_flag = 0;
    done = false;
    sigset_t oldset, oldset2;
    SignalMask::MaskQuitSignal(&oldset);
    raise(SIGQUIT);
    usleep(500000);
    EXPECT_EQ(test_func_flag, 0);
    SignalMask::MaskQuitSignal(&oldset2);
    usleep(500000);
    EXPECT_EQ(test_func_flag, 0);
    SignalMask::RestoreSignal(&oldset2);
    usleep(500000);
    EXPECT_EQ(test_func_flag, 0);
    SignalMask::RestoreSignal(&oldset);
    usleep(500000);
    while(done == false)
    {
        usleep(1);
    }
    EXPECT_EQ(test_func_flag, SIGQUIT);
}
}  // namespace pos
