#include "src/journal_manager/log_buffer/reset_log_group.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"

using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
TEST(ResetLogGroup, Execute_testIfLogBufferReseted)
{
    // Given
    int logGroupId = 0;
    NiceMock<MockJournalLogBuffer> logBuffer;
    ResetLogGroup event(&logBuffer, logGroupId, nullptr);

    // Then
    EXPECT_CALL(logBuffer, AsyncReset(logGroupId, _));

    // When
    event.Execute();
}

} // namespace pos
