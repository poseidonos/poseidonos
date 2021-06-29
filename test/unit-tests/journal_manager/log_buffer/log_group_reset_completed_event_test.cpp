#include "src/journal_manager/log_buffer/log_group_reset_completed_event.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/log_buffer/i_log_group_reset_completed_mock.h"

using testing::NiceMock;

namespace pos
{
TEST(LogGroupResetCompletedEvent, Execute_testIfExecutedSuccessfully)
{
    // Given
    int logGroupId = 0;
    NiceMock<MockILogGroupResetCompleted> target;
    LogGroupResetCompletedEvent logGroupResetCompletedEvent(&target, logGroupId);

    // When
    EXPECT_CALL(target, LogGroupResetCompleted(logGroupId));

    bool result = logGroupResetCompletedEvent.Execute();

    // Then
    bool expect = true;
    EXPECT_EQ(expect, result);
}

} // namespace pos
