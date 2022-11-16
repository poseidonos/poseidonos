#include <gtest/gtest.h>
#include <cstring>

#include "src/journal_manager/log_buffer/log_group_footer_write_context.h"

namespace pos
{
TEST(LogGroupFooterWriteContext, LogGroupFooterWriteContext_testIfExecutedSuccessfully)
{
    LogGroupFooterWriteContext context(0, nullptr);
}

TEST(LogGroupFooterWriteContext, SetIoRequest_testIfContextUpdatedCorrectly)
{
    // Given
    LogGroupFooterWriteContext context(0, nullptr);

    // When
    LogGroupFooter footer;
    footer.lastCheckpointedSeginfoVersion = 3;
    uint64_t offset = 30;

    context.SetIoRequest(offset, footer);

    // Then
    EXPECT_EQ(context.GetLength(), sizeof(LogGroupFooter));
    EXPECT_EQ(context.GetFileOffset(), offset);
    EXPECT_EQ(std::memcmp((void*)context.GetBuffer(), (void*)(&footer), sizeof(LogGroupFooter)), 0);
}

} // namespace pos
