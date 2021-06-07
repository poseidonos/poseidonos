#include "src/journal_manager/log_write/log_write_statistics.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/log/log_handler_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/map_update_log_write_context_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(LogWriteStatistics, Init_testIfItsEnabled)
{
    // Given
    LogWriteStatistics stats;

    // When
    int numLogGroups = 2;
    stats.Init(numLogGroups);

    // Then
    EXPECT_TRUE(stats.IsEnabled() == true);
}

TEST(LogWriteStatistics, UpdateStatus_testIfUpdateFailsWhenDisabled)
{
    // Given
    LogWriteStatistics stats;
    NiceMock<MockLogWriteContext> context;

    // When
    EXPECT_TRUE(stats.IsEnabled() == false);

    // Then
    EXPECT_TRUE(stats.UpdateStatus(&context) == false);
}

TEST(LogWriteStatistics, UpdateStatus_testIfStatsUpdatedWhenBlockWriteDone)
{
    // Given
    LogWriteStatistics stats;
    stats.Init(2);

    NiceMock<MockMapUpdateLogWriteContext> context;
    NiceMock<MockLogHandlerInterface> log;

    ON_CALL(context, GetLogGroupId).WillByDefault(Return(0));
    ON_CALL(context, GetLog).WillByDefault(Return(&log));

    StripeId vsid = 100;
    BlockWriteDoneLog logData;
    logData.volId = 1;
    logData.startVsa.stripeId = vsid;
    logData.startVsa.offset = 0;
    logData.numBlks = 5;

    ON_CALL(log, GetType).WillByDefault(Return(LogType::BLOCK_WRITE_DONE));
    ON_CALL(log, GetVsid).WillByDefault(Return(vsid));
    ON_CALL(log, GetData).WillByDefault(Return((char*)(&logData)));

    // When
    EXPECT_TRUE(stats.UpdateStatus(&context) == true);

    // Then
}

TEST(LogWriteStatistics, UpdateStatus_testIfStatsUpdatedWhenStripeMapUpdated)
{
    // Given
    LogWriteStatistics stats;
    stats.Init(2);

    NiceMock<MockMapUpdateLogWriteContext> context;
    NiceMock<MockLogHandlerInterface> log;

    ON_CALL(context, GetLogGroupId).WillByDefault(Return(0));
    ON_CALL(context, GetLog).WillByDefault(Return(&log));

    StripeId vsid = 100;
    StripeMapUpdatedLog logData;
    logData.vsid = vsid;
    logData.oldMap.stripeId = 1;
    logData.oldMap.stripeLoc = IN_WRITE_BUFFER_AREA;
    logData.newMap.stripeId = vsid;
    logData.newMap.stripeLoc = IN_USER_AREA;

    ON_CALL(log, GetType).WillByDefault(Return(LogType::STRIPE_MAP_UPDATED));
    ON_CALL(log, GetVsid).WillByDefault(Return(vsid));
    ON_CALL(log, GetData).WillByDefault(Return((char*)(&logData)));

    // When
    EXPECT_TRUE(stats.UpdateStatus(&context) == true);

    // Then
}

TEST(LogWriteStatistics, UpdateStatus_testIfStatsUpdatedWhenStripeIsWritten)
{
    // Given
    LogWriteStatistics stats;
    stats.Init(2);

    StripeId vsid = 135;
    StripeId wbStripeId = 10;
    uint64_t offset = 0;
    for (int count = 0; count < 10; count++)
    {
        NiceMock<MockMapUpdateLogWriteContext> blockWriteContext;
        NiceMock<MockLogHandlerInterface> blockWriteLog;

        ON_CALL(blockWriteContext, GetLogGroupId).WillByDefault(Return(0));
        ON_CALL(blockWriteContext, GetLog).WillByDefault(Return(&blockWriteLog));

        BlockWriteDoneLog logData;
        logData.volId = 4;
        logData.startVsa.stripeId = vsid;
        logData.startVsa.offset = offset;
        logData.writeBufferStripeAddress.stripeId = wbStripeId;
        logData.writeBufferStripeAddress.stripeLoc = IN_WRITE_BUFFER_AREA;
        logData.numBlks = 10;

        ON_CALL(blockWriteLog, GetType).WillByDefault(Return(LogType::BLOCK_WRITE_DONE));
        ON_CALL(blockWriteLog, GetVsid).WillByDefault(Return(vsid));
        ON_CALL(blockWriteLog, GetData).WillByDefault(Return((char*)(&logData)));

        // When
        EXPECT_TRUE(stats.UpdateStatus(&blockWriteContext) == true);

        offset += logData.numBlks;
    }

    NiceMock<MockMapUpdateLogWriteContext> stripeMapUpdatedContext;
    NiceMock<MockLogHandlerInterface> stripeMapLog;

    ON_CALL(stripeMapUpdatedContext, GetLogGroupId).WillByDefault(Return(0));
    ON_CALL(stripeMapUpdatedContext, GetLog).WillByDefault(Return(&stripeMapLog));

    StripeMapUpdatedLog logData;
    logData.vsid = vsid;
    logData.oldMap.stripeId = wbStripeId;
    logData.oldMap.stripeLoc = IN_WRITE_BUFFER_AREA;
    logData.newMap.stripeId = vsid;
    logData.newMap.stripeLoc = IN_USER_AREA;

    ON_CALL(stripeMapLog, GetType).WillByDefault(Return(LogType::STRIPE_MAP_UPDATED));
    ON_CALL(stripeMapLog, GetVsid).WillByDefault(Return(vsid));
    ON_CALL(stripeMapLog, GetData).WillByDefault(Return((char*)(&logData)));

    // When
    EXPECT_TRUE(stats.UpdateStatus(&stripeMapUpdatedContext) == true);

    // Then
    stats.PrintStats(0);
}

TEST(LogWriteStatistics, AddToList_testIfLogNotAddedWhenDisabled)
{
    // Given
    LogWriteStatistics stats;
    EXPECT_TRUE(stats.IsEnabled() == false);

    NiceMock<MockLogWriteContext> context;

    // When
    stats.AddToList(&context);

    // Then
}

TEST(LogWriteStatistics, AddToList_testIfLogAddedWhenEnabled)
{
    // Given
    LogWriteStatistics stats;
    stats.Init(2);
    EXPECT_TRUE(stats.IsEnabled() == true);

    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;

    // When
    stats.AddToList(context);

    // Then
}

TEST(LogWriteStatistics, PrintStats_testIfNotPrintedWhenDisabled)
{
    // Given
    LogWriteStatistics stats;
    EXPECT_TRUE(stats.IsEnabled() == false);

    // When
    stats.PrintStats(0);

    // Then
}

} // namespace pos
