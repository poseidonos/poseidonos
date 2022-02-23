#include <gtest/gtest.h>
#include "src/signal_handler/user_signal_interface.h"

namespace pos
{

TEST(UserSignalInterface, Enable)
{
    UserSignalInterface::Enable(true);
    UserSignalInterface::Enable(false);
}

TEST(UserSignalInterface, SetTimeout)
{
    UserSignalInterface::SetTimeout(130);
    UserSignalInterface::SetTimeout(5);
}

TEST(UserSignalInterface, TriggerBacktrace)
{
    // Init Signal Handler.
    UserSignalInterface::TriggerBacktrace();
}

}  // namespace pos

