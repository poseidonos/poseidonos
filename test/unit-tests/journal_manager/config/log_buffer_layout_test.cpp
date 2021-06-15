#include <gtest/gtest.h>
#include "src/journal_manager/config/log_buffer_layout.h"

namespace pos
{
static const uint64_t LOG_BUFFER_SIZE = 16 * 1024 * 1024;
static const int NUM_LOG_GROUPS = 2;

TEST(LogBufferLayout, LogBufferLayout_testIfExecutedSuccessfully)
{
    // Given

    // When, Then
    LogBufferLayout *layout = new LogBufferLayout;

    delete layout;
}

TEST(LogBufferLayout, Init_testIfLogGroupLayoutIsAdded)
{
    // Given
    LogBufferLayout layout;

    // When
    layout.Init(LOG_BUFFER_SIZE, NUM_LOG_GROUPS);

    // Then
    for (int groupId = 0; groupId < NUM_LOG_GROUPS; groupId++)
    {
        LogGroupLayout groupLayout = layout.GetLayout(groupId);

        uint64_t expectedStartOffset = (LOG_BUFFER_SIZE / NUM_LOG_GROUPS) * groupId;
        EXPECT_EQ(groupLayout.startOffset, expectedStartOffset);

        uint64_t expectedMaxOffset = (LOG_BUFFER_SIZE / NUM_LOG_GROUPS) * (groupId + 1);
        EXPECT_EQ(groupLayout.maxOffset, expectedMaxOffset);

        uint64_t expectedFooterStartOffset = expectedMaxOffset - sizeof(LogGroupFooter);
        EXPECT_EQ(groupLayout.footerStartOffset, expectedFooterStartOffset);
    }
}

TEST(LogBufferLayout, GetLayout_testIfEmptyLayoutReturnedWhenNotInitialized)
{
    // Given
    LogBufferLayout layout;

    // When
    LogGroupLayout groupLayout = layout.GetLayout(NUM_LOG_GROUPS + 1);

    // Then
    EXPECT_EQ(groupLayout.startOffset, 0);
    EXPECT_EQ(groupLayout.footerStartOffset, 0);
    EXPECT_EQ(groupLayout.maxOffset, 0);
}

} // namespace pos
