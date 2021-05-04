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
    // Given : Nothing
    // When : Constructor and Destructor
    CommandTimeoutHandler* commandTimeoutHandler = new CommandTimeoutHandler();
    delete commandTimeoutHandler;
    // Then : Nothing
}

TEST(CommandTimeoutHandler, IsPendingAbortZero)
{
    // Given : Nothing
    // When : Constructor
    CommandTimeoutHandler* commandTimeoutHandler = new CommandTimeoutHandler();
    // Then : Check Pending Abort Zero
    bool actual = commandTimeoutHandler->IsPendingAbortZero();
    bool expected = true;
    ASSERT_EQ(actual, expected);
    delete commandTimeoutHandler;
}

TEST(CommandTimeoutHandler, AbortSubmitHandler_Constructor)
{
    // Given : Abort Context Initialize
    AbortContext abortContext(nullptr, nullptr, 0);
    // When : AbortSubmitHandler Constructor, Destructor
    CommandTimeoutHandler::AbortSubmitHandler *commandAbortEvent
        = new CommandTimeoutHandler::AbortSubmitHandler(&abortContext);
    delete commandAbortEvent;
    // Then : Nothing
}

TEST(CommandTimeoutHandler, AbortSubmitHandler_DiskIO)
{
    // Given : Abort Context Initialize
    AbortContext abortContext(nullptr, nullptr, 0);
    // When : AbortSubmitHandler Constructor
    CommandTimeoutHandler::AbortSubmitHandler* commandAbortEvent
        = new CommandTimeoutHandler::AbortSubmitHandler(&abortContext);
    MockUBlockDevice *dev = new MockUBlockDevice("", 0, nullptr);
    UblockSharedPtr devSharedPtr(dev);
    ON_CALL(*dev, GetSN()).WillByDefault(Return("different"));
    // Then : DiskIO Execute
    commandAbortEvent->DiskIO(devSharedPtr, nullptr);
    delete commandAbortEvent;
}

TEST(CommandTimeoutHandler, AbortSubmitHandler_Execute)
{
    // Given : Abort Context Initialize
    AbortContext abortContext(nullptr, nullptr, 0);
    // When : AbortSubmitHandler Constructor
    CommandTimeoutHandler::AbortSubmitHandler* commandAbortEvent
        = new CommandTimeoutHandler::AbortSubmitHandler(&abortContext);
    UblockSharedPtr devSharedPtr(new MockUBlockDevice("", 100, nullptr));
    // Then : Execute command Abort Event
    bool actual = commandAbortEvent->Execute();
    bool expected = true;
    ASSERT_EQ(actual, expected);
    delete commandAbortEvent;
}

} // namespace pos
