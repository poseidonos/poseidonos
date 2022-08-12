#include "src/journal_manager/log_write/log_group_buffer_status.h"

#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"

namespace pos
{
static const uint64_t META_PAGE_SIZE = 4032;

TEST(LogGroupBufferStatus, Reset_testIfAllReset)
{
    uint64_t startOffset = 100;
    LogGroupBufferStatus status(startOffset, 0, 0);
    status.Reset();

    EXPECT_EQ(status.GetSeqNum(), 0);
    EXPECT_EQ(status.GetNumLogsAdded(), 0);
    EXPECT_EQ(status.GetNumLogsFilled(), 0);
    EXPECT_EQ(status.GetNextOffset(), startOffset);
    EXPECT_EQ(status.GetStatus(), LogGroupStatus::INIT);
}

TEST(LogGroupBufferStatus, SetActive_testIfStatusChangedToActive)
{
    LogGroupBufferStatus status(0, 0, 0);

    uint64_t seqNum = 1000;
    status.SetActive(1000);

    EXPECT_EQ(status.GetStatus(), LogGroupStatus::ACTIVE);
    EXPECT_EQ(status.GetSeqNum(), seqNum);
}

TEST(LogGroupBufferStatus, TryToAllocate_testIfAllocatedOffsetIsNotCrossingMetaPage)
{
    // Given: Initialized buffer status
    uint64_t startOffset = 0;
    uint64_t maxOffset = 1024 * 1024;
    LogGroupBufferStatus status(startOffset, maxOffset, META_PAGE_SIZE);

    // When: Try to allocate log
    uint64_t logSize = 52;

    // Then: Allocation should not cross the meta page
    for (int testCount = 0; testCount < 10; testCount++)
    {
        uint64_t allocatedOffset = 0;
        int expectReturnCode = 0;
        EXPECT_TRUE(status.TryToAllocate(logSize, allocatedOffset) == expectReturnCode);

        uint64_t startMpage = allocatedOffset / META_PAGE_SIZE;
        uint64_t endMpage = (allocatedOffset + logSize - 1) / META_PAGE_SIZE;

        EXPECT_TRUE(startMpage == endMpage);
    }
}

TEST(LogGroupBufferStatus, TryToAllocate_testIfAllocFailWithSizeLargerThanMetaPage)
{
    // Given: Initialized buffer status
    uint64_t startOffset = 0;
    uint64_t maxOffset = 1024 * 1024;
    LogGroupBufferStatus status(startOffset, maxOffset, META_PAGE_SIZE);

    // When: Try to allocate log size larger than meta page
    uint64_t logSize = META_PAGE_SIZE * 2;

    // Then: Allocation should be failed
    uint64_t allocatedOffset = 0;

    int expectReturnCode =  -1 * EID(JOURNAL_INVALID_SIZE_LOG_REQUESTED);
    EXPECT_TRUE(status.TryToAllocate(logSize, allocatedOffset) == expectReturnCode);
}

TEST(LogGroupBufferStatus, TryToAllocate_testIfAllocFailsWhenFull)
{
    // Given: Initialized buffer status with small maxOffset
    uint64_t startOffset = 0;
    uint64_t maxOffset = META_PAGE_SIZE;
    LogGroupBufferStatus status(startOffset, maxOffset, META_PAGE_SIZE);

    uint64_t logSize = 52;

    // When: Log buffer offset is allocated
    int numTestsToMakeBufferFull = META_PAGE_SIZE / logSize;
    for (int testCount = 0; testCount < numTestsToMakeBufferFull; testCount++)
    {
        uint64_t offset = 0;
        int expectReturnCode = 0;
        EXPECT_TRUE(status.TryToAllocate(logSize, offset) == expectReturnCode);
    }

    // When: Try to allocate one more log
    // Then: Allocation should be failed
    uint64_t offset = 0;
    int expectReturnCode = EID(JOURNAL_LOG_GROUP_FULL);
    EXPECT_TRUE(status.TryToAllocate(logSize, offset) == expectReturnCode);
}

TEST(LogGroupBufferStatus, LogFilled_testIfLogFilled)
{
    // Given: Initialized buffer status
    uint64_t startOffset = 0;
    uint64_t maxOffset = 1024 * 1024;
    LogGroupBufferStatus status(startOffset, maxOffset, META_PAGE_SIZE);

    // When: Allocate logs 10 times, and notify log filled
    uint32_t logSize = 52;
    int numLogsToTest = 10;
    for (int testCount = 0; testCount < numLogsToTest; testCount++)
    {
        uint64_t offset = 0;
        int expectReturnCode = 0;
        EXPECT_TRUE(status.TryToAllocate(logSize, offset) == expectReturnCode);
    }
    for (int testCount = 0; testCount < numLogsToTest; testCount++)
    {
        status.LogFilled();
    }

    // Then: 10 logs should be filled
    EXPECT_EQ(status.GetNumLogsAdded(), numLogsToTest);
    EXPECT_EQ(status.GetNumLogsFilled(), numLogsToTest);
}

TEST(LogGroupBufferStatus, TryToSetFull_testIfSetFullSuccess)
{
    // Given: Initialized buffer status with small maxOffset
    uint64_t startOffset = 0;
    uint64_t maxOffset = META_PAGE_SIZE;
    LogGroupBufferStatus status(startOffset, maxOffset, META_PAGE_SIZE);

    uint64_t logSize = 52;

    // When: Try to allocate log buffer until it's full, and notify all logs are filled
    int numTestsToMakeBufferFull = META_PAGE_SIZE / logSize;
    uint64_t offset = 0;

    for (int testCount = 0; testCount < numTestsToMakeBufferFull; testCount++)
    {
        int expectReturnCode = 0;
        EXPECT_TRUE(status.TryToAllocate(logSize, offset) == expectReturnCode);
    }
    for (int testCount = 0; testCount < numTestsToMakeBufferFull; testCount++)
    {
        status.LogFilled();
    }

    // When: One more log try to allocate buffer, but fails
    int expectReturnCode = EID(JOURNAL_LOG_GROUP_FULL);
    EXPECT_TRUE(status.TryToAllocate(logSize, offset) == expectReturnCode);

    // Then: TryToSetFull should be succeed
    EXPECT_EQ(status.TryToSetFull(), true);
}

TEST(LogGroupBufferStatus, TryToSetFull_testIfSetFullFailWhenNotFullyFilled)
{
    // Given: Initialized buffer status with small maxOffset
    uint64_t startOffset = 0;
    uint64_t maxOffset = META_PAGE_SIZE;
    LogGroupBufferStatus status(startOffset, maxOffset, META_PAGE_SIZE);

    uint64_t logSize = 52;

    // When: Try to allocate log buffer until it's full, and notify all logs except the last one are filled
    int numTestsToMakeBufferFull = META_PAGE_SIZE / logSize;
    uint64_t offset = 0;

    for (int testCount = 0; testCount < numTestsToMakeBufferFull; testCount++)
    {
        int expectReturnCode = 0;
        EXPECT_TRUE(status.TryToAllocate(logSize, offset) == expectReturnCode);
    }
    for (int testCount = 0; testCount < numTestsToMakeBufferFull - 1; testCount++)
    {
        status.LogFilled();
    }

    // When: One more buffer allocation requested, and found there's no more space
    int expectReturnCode = EID(JOURNAL_LOG_GROUP_FULL);
    EXPECT_TRUE(status.TryToAllocate(logSize, offset) == expectReturnCode);

    // Then: TryToSetFull should not be succeed
    EXPECT_EQ(status.TryToSetFull(), false);

    // When: The last log is filled
    status.LogFilled();

    // Then: TryToSetFull should be succeed
    EXPECT_EQ(status.TryToSetFull(), true);
}

TEST(LogGroupBufferStatus, TryToSetFull_testIfSetFullFailWhenNotWaitingToBeFilled)
{
    // Given: Initialized buffer status with small maxOffset
    uint64_t startOffset = 0;
    uint64_t maxOffset = META_PAGE_SIZE;
    LogGroupBufferStatus status(startOffset, maxOffset, META_PAGE_SIZE);

    uint64_t logSize = 52;

    // When: Try to allocate log buffer until it's full, and notify all logs
    int numTestsToMakeBufferFull = META_PAGE_SIZE / logSize;
    uint64_t offset = 0;
    for (int testCount = 0; testCount < numTestsToMakeBufferFull; testCount++)
    {
        int expectReturnCode = 0;
        EXPECT_TRUE(status.TryToAllocate(logSize, offset) == expectReturnCode);
    }
    for (int testCount = 0; testCount < numTestsToMakeBufferFull; testCount++)
    {
        status.LogFilled();
    }

    // Then: TryToSetFull should not be succeed
    EXPECT_EQ(status.TryToSetFull(), false);

    // When: One more buffer allocation requested, and found there's no more space
    int expectReturnCode = EID(JOURNAL_LOG_GROUP_FULL);
    EXPECT_TRUE(status.TryToAllocate(logSize, offset) == expectReturnCode);

    // Then: TryToSetFull should be succeed
    EXPECT_EQ(status.TryToSetFull(), true);
}
} // namespace pos
