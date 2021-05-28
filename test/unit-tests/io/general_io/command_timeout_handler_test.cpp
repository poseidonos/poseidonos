#include "src/io/general_io/command_timeout_handler.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/device/device_manager_mock.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"
#include "test/unit-tests/utils/mock_builder.h"
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
    MockAffinityManager mockAffinityMgr = BuildDefaultAffinityManagerMock();
    MockDeviceManager mockDevMgr(&mockAffinityMgr);
    // When : AbortSubmitHandler Constructor, Destructor
    CommandTimeoutHandler::AbortSubmitHandler *commandAbortEvent
        = new CommandTimeoutHandler::AbortSubmitHandler(&abortContext, &mockDevMgr);
    delete commandAbortEvent;
    // Then : Nothing
}

TEST(CommandTimeoutHandler, AbortSubmitHandler_DiskIO)
{
    // Given : Abort Context Initialize
    AbortContext abortContext(nullptr, nullptr, 0);
    MockAffinityManager mockAffinityMgr = BuildDefaultAffinityManagerMock();
    MockDeviceManager mockDevMgr(&mockAffinityMgr);
    MockUBlockDevice *dev = new MockUBlockDevice("", 0, nullptr);
    EXPECT_CALL(*dev, GetSN).WillRepeatedly(Return("different"));
    UblockSharedPtr devSharedPtr(dev);

    // When : AbortSubmitHandler Constructor
    CommandTimeoutHandler::AbortSubmitHandler* commandAbortEvent
        = new CommandTimeoutHandler::AbortSubmitHandler(&abortContext, &mockDevMgr);
    commandAbortEvent->DiskIO(devSharedPtr, nullptr);

    // Then : DiskIO Execute
    delete commandAbortEvent;
}

TEST(CommandTimeoutHandler, AbortSubmitHandler_Execute)
{
    // Given : Abort Context Initialize
    AbortContext abortContext(nullptr, nullptr, 0);
    MockAffinityManager mockAffinityMgr = BuildDefaultAffinityManagerMock();
    MockDeviceManager mockDevMgr(&mockAffinityMgr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc).Times(1);

    // When : AbortSubmitHandler Constructor
    CommandTimeoutHandler::AbortSubmitHandler* commandAbortEvent
        = new CommandTimeoutHandler::AbortSubmitHandler(&abortContext, &mockDevMgr);
    bool actual = commandAbortEvent->Execute();

    // Then : Execute command Abort Event
    bool expected = true;
    ASSERT_EQ(actual, expected);
    delete commandAbortEvent;
}

} // namespace pos
