#include "src/io/general_io/command_timeout_handler.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "src/device/unvme/unvme_cmd.h"
#include <gtest/gtest.h>
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(CommandTimeoutHandler, Constructor)
{
    CommandTimeoutHandler *commandTimeoutHandler = new CommandTimeoutHandler();
    delete commandTimeoutHandler;
}

TEST(CommandTimeoutHandler, IsPendingAbortZero)
{
    CommandTimeoutHandler *commandTimeoutHandler = new CommandTimeoutHandler();
    bool actual = commandTimeoutHandler->IsPendingAbortZero();
    bool expected = true;
    ASSERT_EQ(actual, expected);
    delete commandTimeoutHandler;
}

TEST(CommandTimeoutHandler, AbortSubmitHandler_Constructor)
{
    AbortContext abortContext(nullptr, nullptr, 0);
    CommandTimeoutHandler::AbortSubmitHandler *commandAbortEvent 
        = new CommandTimeoutHandler::AbortSubmitHandler(&abortContext);
    delete commandAbortEvent;
}

TEST(CommandTimeoutHandler, AbortSubmitHandler_DiskIO)
{
    AbortContext abortContext(nullptr, nullptr, 0);
    CommandTimeoutHandler::AbortSubmitHandler *commandAbortEvent 
        = new CommandTimeoutHandler::AbortSubmitHandler(&abortContext);
    MockUBlockDevice *dev = new MockUBlockDevice("", 0, nullptr);
    UblockSharedPtr devSharedPtr(dev);
    ON_CALL(*dev, GetSN()).WillByDefault(Return("different"));
    commandAbortEvent->DiskIO(devSharedPtr, nullptr);
    delete commandAbortEvent;
}

TEST(CommandTimeoutHandler, AbortSubmitHandler_Execute)
{
    AbortContext abortContext(nullptr, nullptr, 0);
    CommandTimeoutHandler::AbortSubmitHandler *commandAbortEvent 
        = new CommandTimeoutHandler::AbortSubmitHandler(&abortContext);
    UblockSharedPtr devSharedPtr(new MockUBlockDevice("", 100, nullptr));

    bool actual = commandAbortEvent->Execute();
    bool expected = true;
    ASSERT_EQ(actual, expected);
    delete commandAbortEvent;
}

} // namespace pos
