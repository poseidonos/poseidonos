#include "src/journal_manager/log_buffer/log_write_context.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/journal_manager/log_buffer/log_buffer_io_context.h"
#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/journal_manager/log/log_handler_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"

using testing::NiceMock;
using testing::Return;

namespace pos
{
static const uint32_t INVALID_GROUP_ID = UINT32_MAX;

TEST(LogWriteContext, LogWriteContext_testIfConstructedSuccessfully)
{
    // Given, When
    NiceMock<MockLogHandlerInterface>* log = new NiceMock<MockLogHandlerInterface>;
    MapList dirtyList;
    LogWriteContext logWriteContext(log, dirtyList, nullptr);

    // Then: Log group id is invalid
    int result = logWriteContext.GetLogGroupId();
    int retValue = INVALID_GROUP_ID;
    EXPECT_EQ(retValue, result);
}
TEST(LogWriteContext, GetLog_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockLogHandlerInterface>* log = new NiceMock<MockLogHandlerInterface>;
    MapList dirtyList;
    LogWriteContext logWriteContext(log, dirtyList, nullptr);

    // When
    LogHandlerInterface* result = logWriteContext.GetLog();

    // Then
    EXPECT_EQ(log, result);
}

TEST(LogWriteContext, SetBufferAllocated_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockLogHandlerInterface>* log = new NiceMock<MockLogHandlerInterface>;
    MapList dirtyList;
    LogWriteContext logWriteContext(log, dirtyList, nullptr);

    // When
    uint64_t fileOffset = 0;
    int logGroupId = 1;
    uint32_t seqNum = 1;
    
    ON_CALL(*log, GetSize).WillByDefault(Return(10));
    ON_CALL(*log, GetData).WillByDefault(Return(nullptr));

    EXPECT_CALL(*log, SetSeqNum(seqNum));

    logWriteContext.SetLogAllocated(logGroupId, seqNum);

    // Then
    EXPECT_EQ(logGroupId, logWriteContext.GetLogGroupId());
}
} // namespace pos
