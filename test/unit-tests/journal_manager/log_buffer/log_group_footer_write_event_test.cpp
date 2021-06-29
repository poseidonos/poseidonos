#include <gtest/gtest.h>

#include "src/journal_manager/log_buffer/log_group_footer_write_event.h"

#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"

using ::testing::Field;
using ::testing::NiceMock;
using ::testing::Pointee;
using ::testing::Return;

namespace pos
{
TEST(LogGroupFooterWriteEvent, Execute_testIfSuccessWhenWriteSuccess)
{
    // Given
    NiceMock<MockJournalLogBuffer> logBuffer;
    LogGroupFooter footer;

    LogGroupFooterWriteEvent event(&logBuffer, footer, 0, 0, nullptr);

    // When
    EXPECT_CALL(logBuffer, InternalIo).WillOnce(Return(0));
    bool result = event.Execute();

    // Then
    EXPECT_EQ(result, true);
}

TEST(LogGroupFooterWriteEvent, Execute_testIfFailsWhenWriteFails)
{
    // Given
    NiceMock<MockJournalLogBuffer> logBuffer;
    LogGroupFooter footer;

    LogGroupFooterWriteEvent event(&logBuffer, footer, 0, 0, nullptr);

    // When
    EXPECT_CALL(logBuffer, InternalIo).WillOnce(Return(-1));
    bool result = event.Execute();

    // Then
    EXPECT_EQ(result, false);
}

} // namespace pos
