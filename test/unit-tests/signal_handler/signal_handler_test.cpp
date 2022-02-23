#include <gtest/gtest.h>
#include <signal.h>
#include "src/signal_handler/signal_handler.h"

namespace pos
{

void IntendedSegfault(void)
{
    *((volatile int *)0) = 0;
}

TEST(SignalHandler, SegFaultHandlerIntendedFault)
{
    SignalHandlerSingleton::Instance()->Register();
    ASSERT_DEATH(IntendedSegfault(), "");
    SignalHandlerSingleton::Instance()->Deregister();
}

TEST(SignalHandler, AbortHandler)
{
    SignalHandlerSingleton::Instance()->Register();
    ASSERT_DEATH(SignalHandlerSingleton::Instance()->ExceptionHandler(SIGABRT), "");
    SignalHandlerSingleton::Instance()->Deregister();
}

TEST(SignalHandler, SegFaultHandler)
{
    SignalHandlerSingleton::Instance()->Register();
    ASSERT_DEATH(SignalHandlerSingleton::Instance()->ExceptionHandler(SIGSEGV), "");
    SignalHandlerSingleton::Instance()->Deregister();
}

TEST(SignalHandler, INTHandler)
{
    SignalHandlerSingleton::Instance()->Register();
    SignalHandlerSingleton::Instance()->INTHandler(SIGINT);
    SignalHandlerSingleton::Instance()->Deregister();
}

}  // namespace pos
