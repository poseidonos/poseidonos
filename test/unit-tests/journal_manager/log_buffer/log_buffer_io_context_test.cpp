#include "src/journal_manager/log_buffer/log_buffer_io_context.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/event_mock.h"

using testing::NiceMock;
using testing::Return;

namespace pos
{
TEST(LogBufferIoContext, LogBufferIoContext_testIfConstructedSuccessfully)
{
    // Given, When
    int logGroupId = 1;
    LogBufferIoContext logBufferIoContext(logGroupId, nullptr);

    // Then
    EXPECT_EQ(logGroupId, logBufferIoContext.GetLogGroupId());
}

TEST(LogBufferIoContext, SetInternalCallback_testIfExecutedSuccessfully)
{
    // Given
    LogBufferIoContext logBufferIoContext(0, nullptr);
    MetaIoCbPtr callback;

    // When, Then
    logBufferIoContext.SetInternalCallback(callback);
}

TEST(LogBufferIoContext, SetFile_testIfExecutedSuccessfully)
{
    // Given
    LogBufferIoContext logBufferIoContext(0, nullptr);
    int fileDescriptor = 0;

    // When, Then
    logBufferIoContext.SetFile(fileDescriptor);
}

TEST(LogBufferIoContext, IoDone_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockEvent>* clientCallback = new NiceMock<MockEvent>;
    LogBufferIoContext logBufferIoContext(0, EventSmartPtr(clientCallback));

    // When, Then
    bool retResult = true;
    EXPECT_CALL(*clientCallback, Execute).WillOnce(Return(retResult));
    logBufferIoContext.IoDone();
}
} // namespace pos
