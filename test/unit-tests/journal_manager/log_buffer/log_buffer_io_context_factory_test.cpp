#include "src/journal_manager/log_buffer/log_buffer_io_context_factory.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(LogBufferIoContextFactory, LogBufferIoContextFactory_)
{
}

TEST(LogBufferIoContextFactory, Init_)
{
}

TEST(LogBufferIoContextFactory, CreateLogBufferIoContext_)
{
    /*

    // Given
    LogWriteContextFactory logWriteContextFactory;

    // When
    uint64_t offset = 0;
    int logGroupId = 1;
    uint64_t groupSize = 512;
    EventSmartPtr callbackEvent;
    char* initializedDataBuffer = new char[groupSize];
    memset(initializedDataBuffer, 0xFF, groupSize);
    LogBufferIoContext* logWriteContext = logWriteContextFactory.CreateLogGroupResetContext(offset, logGroupId, groupSize, callbackEvent, initializedDataBuffer);

    // Then
    EXPECT_EQ(logGroupId, logWriteContext->GetLogGroupId());
    EXPECT_EQ(callbackEvent, dynamic_cast<LogBufferIoContext*>(logWriteContext)->GetClientCallback());
    EXPECT_EQ(offset, logWriteContext->GetFileOffset());
    EXPECT_EQ(groupSize, logWriteContext->GetLength());
    EXPECT_EQ(initializedDataBuffer, logWriteContext->GetBuffer());
*/
}

TEST(LogBufferIoContextFactory, CreateMapUpdateLogWriteIoContext_)
{
}

TEST(LogBufferIoContextFactory, CreateLogWriteIoContext_)
{
}

TEST(LogBufferIoContextFactory, CreateLogGroupFooterWriteContext_)
{
}

TEST(LogBufferIoContextFactory, _GetMaxNumGcBlockMapUpdateInAContext_)
{
}

} // namespace pos
