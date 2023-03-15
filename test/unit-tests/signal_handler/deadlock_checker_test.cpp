#include <gtest/gtest.h>
#include <signal.h>
#include <unistd.h>
#include "src/signal_handler/deadlock_checker.h"

namespace pos
{

std::atomic<bool> testSignalHandlerFlag;
void SignalHandlerFake(int sig)
{
    testSignalHandlerFlag = true;
}

void UsleepForTest(uint64_t us)
{
    uint64_t SEC_PER_NANOSEC = 1000000000ULL;
    static struct timespec startTS, endTS;
    clock_gettime(CLOCK_MONOTONIC_RAW, &startTS);
    while(1)
    {
        clock_gettime(CLOCK_MONOTONIC_RAW, &endTS);
        uint64_t timeDiff = SEC_PER_NANOSEC * (endTS.tv_sec -  startTS.tv_sec) + (endTS.tv_nsec - startTS.tv_nsec);
        if (timeDiff > us * 1000)
        {
            break;
        }
    }
}


TEST(DeadlockChecker, AllocFreeSuccess)
{
    // If AllocHeartBeatCount is incurred, but do not increase heart beat count,
    // signal handler happend..?
    signal(SIGUSR1, SignalHandlerFake);
    // another test can run deadLockChecker from unvmf_io_handler
    DeadLockCheckerSingleton::Instance()->DeRegister();
    DeadLockCheckerSingleton::Instance()->RunDeadLockChecker();
    uint32_t id = DeadLockCheckerSingleton::Instance()->AllocHeartBeatCount();
    DeadLockCheckerSingleton::Instance()->SetTimeout(1);
    testSignalHandlerFlag = false;
    // Do not increase heart beat count
    UsleepForTest(1000000);
    EXPECT_EQ(testSignalHandlerFlag, true);
    // Check signal handler from timeout is incurred only once.
    testSignalHandlerFlag = false;
    UsleepForTest(1000000);
    EXPECT_EQ(testSignalHandlerFlag, false);
    // FreeHeartBeatCount and usleep does not increase heart beat count
    testSignalHandlerFlag = false;
    id = DeadLockCheckerSingleton::Instance()->AllocHeartBeatCount();
    DeadLockCheckerSingleton::Instance()->FreeHeartBeatCount(id);
    // Do not increase heart beat count
    UsleepForTest(1000000);
    // signal handler is not incurred?
    EXPECT_EQ(testSignalHandlerFlag, false);
}

TEST(DeadlockChecker, RegisterOnceAndHeartBeat)
{
    signal(SIGUSR1, SignalHandlerFake);
    testSignalHandlerFlag = false;
    DeadLockCheckerSingleton::Instance()->RunDeadLockChecker();
    DeadLockCheckerSingleton::Instance()->SetTimeout(1);
    DeadLockCheckerSingleton::Instance()->DeRegister();
    DeadLockCheckerSingleton::Instance()->RegisterOnceAndHeartBeat();
    DeadLockCheckerSingleton::Instance()->RegisterOnceAndHeartBeat();
    EXPECT_EQ(testSignalHandlerFlag, false);
    UsleepForTest(2000000);
    EXPECT_EQ(testSignalHandlerFlag, true);
    DeadLockCheckerSingleton::Instance()->DeRegister();
    testSignalHandlerFlag = false;
    UsleepForTest(1000000);
    EXPECT_EQ(testSignalHandlerFlag, false);
}

} // namespace pos
