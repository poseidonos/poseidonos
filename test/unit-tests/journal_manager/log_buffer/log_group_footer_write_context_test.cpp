#include "src/journal_manager/log_buffer/log_group_footer_write_context.h"

#include <gtest/gtest.h>

#include <cstring>

#include "test/unit-tests/event_scheduler/callback_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(LogGroupFooterWriteContext, LogGroupFooterWriteContext_testIfExecutedSuccessfully)
{
    LogGroupFooter footer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));
    LogGroupFooterWriteContext context(0, callback, footer, 0);
}

TEST(LogGroupFooterWriteContext, SetIoRequest_testIfContextUpdatedCorrectly)
{
    // Given
    LogGroupFooter footer;
    footer.lastCheckpointedSeginfoVersion = 3;

    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));

    uint64_t offset = 30;

    LogGroupFooterWriteContext context(0, callback, footer, offset);

    // Then
    EXPECT_EQ(context.GetLength(), sizeof(LogGroupFooter));
    EXPECT_EQ(context.GetFileOffset(), offset);
    EXPECT_EQ(std::memcmp((void*)context.GetBuffer(), (void*)(&footer), sizeof(LogGroupFooter)), 0);
}

} // namespace pos
