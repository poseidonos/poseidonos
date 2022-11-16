#include "src/journal_manager/log_buffer/log_group_reset_context.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(LogGroupResetContext, SetIoRequest_testIfExecutedSuccessfully)
{
    // Given
    LogGroupResetContext logGroupResetContext(0, nullptr);

    // When
    uint64_t fileOffset = 0;
    uint64_t length = 1024;
    char* buffer = new char[length];
    logGroupResetContext.SetIoRequest(fileOffset, length, buffer);

    // Then
    EXPECT_EQ(fileOffset, logGroupResetContext.GetFileOffset());
    EXPECT_EQ(length, logGroupResetContext.GetLength());
    EXPECT_EQ(buffer, logGroupResetContext.GetBuffer());

    delete buffer;
}
} // namespace pos
