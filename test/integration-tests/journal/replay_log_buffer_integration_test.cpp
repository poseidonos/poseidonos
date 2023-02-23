#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include "src/allocator/include/allocator_const.h"
#include "src/journal_manager/log/log_event.h"
#include "test/integration-tests/journal/fake/i_segment_ctx_fake.h"
#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"
#include "test/integration-tests/journal/utils/used_offset_calculator.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::Return;

namespace pos
{
class ReplayLogBufferIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    ReplayLogBufferIntegrationTest(void);
    virtual ~ReplayLogBufferIntegrationTest(void) = default;

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    uint64_t _CalculateLogGroupSize(uint32_t numStripes, uint32_t numBlockMapLogsInStripe);
    void _IncreaseOffsetIfOverMpageSize(uint64_t& nextOffset, uint64_t logSize);
};

ReplayLogBufferIntegrationTest::ReplayLogBufferIntegrationTest(void)
: JournalManagerTestFixture(GetLogFileName())
{
}

void
ReplayLogBufferIntegrationTest::SetUp(void)
{
}

void
ReplayLogBufferIntegrationTest::TearDown(void)
{
}

void
ReplayLogBufferIntegrationTest::_IncreaseOffsetIfOverMpageSize(uint64_t& nextOffset, uint64_t logSize)
{
    uint64_t currentMetaPage = nextOffset / testInfo->metaPageSize;
    uint64_t endMetaPage = (nextOffset + logSize - 1) / testInfo->metaPageSize;
    if (currentMetaPage != endMetaPage)
    {
        nextOffset = endMetaPage * testInfo->metaPageSize;
    }
}

uint64_t
ReplayLogBufferIntegrationTest::_CalculateLogGroupSize(uint32_t numStripes, uint32_t numBlockMapLogsInStripe)
{
    uint64_t nextOffset = 0;

    for (uint32_t stripeIndex = 0; stripeIndex < numStripes; stripeIndex++)
    {
        for (uint32_t blockIndex = 0; blockIndex < numBlockMapLogsInStripe; blockIndex++)
        {
            // TODO (cheolho.kang): Needs to be refactored to share code with UsedOffsetCalculator
            _IncreaseOffsetIfOverMpageSize(nextOffset, sizeof(BlockWriteDoneLog));
            nextOffset += sizeof(BlockWriteDoneLog);
        }
        _IncreaseOffsetIfOverMpageSize(nextOffset, sizeof(StripeMapUpdatedLog));
        nextOffset += sizeof(StripeMapUpdatedLog);
    }

    nextOffset += sizeof(LogGroupFooter);
    return nextOffset;
}

TEST_F(ReplayLogBufferIntegrationTest, ReplayFullLogBuffer)
{
    POS_TRACE_DEBUG(9999, "ReplayLogBufferIntegrationTest::ReplaySeveralLogGroup");

    JournalConfigurationBuilder builder(testInfo);
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(16 * 1024);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    BlkAddr rba = std::rand() % testInfo->defaultTestVolSizeInBlock;
    StripeId vsid = std::rand() % testInfo->numUserStripes;
    StripeTestFixture stripe(vsid, testInfo->defaultTestVol);

    UsedOffsetCalculator usedOffset(journal, logBufferSize - sizeof(LogGroupFooter));
    uint64_t startOffset = 0;
    while (usedOffset.CanBeWritten(sizeof(BlockWriteDoneLog)) == true)
    {
        writeTester->WriteOverwrittenBlockLogs(stripe, rba, startOffset++, 1);
    }

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    replayTester->ExpectReplayOverwrittenBlockLog(stripe);

    VirtualBlks writtenLastBlock = stripe.GetBlockMapList().back().second;
    VirtualBlkAddr tail = ReplayTestFixture::GetNextBlock(writtenLastBlock);
    replayTester->ExpectReplayUnflushedActiveStripe(tail, stripe);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(ReplayLogBufferIntegrationTest, ReplayCirculatedLogBuffer)
{
    POS_TRACE_DEBUG(9999, "ReplayLogBufferIntegrationTest::ReplayCirculatedLogBuffer");

    JournalConfigurationBuilder builder(testInfo);
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(16 * 1024);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    // Write dummy logs to the first log group (to be cleared by checkpoint later)
    uint32_t startSegmentIdForDummyLog = testInfo->numUserSegments - 1;
    writeTester->WriteLogsWithSize(logGroupSize - sizeof(LogGroupFooter), startSegmentIdForDummyLog);

    // Write logs to fill log buffer, and start checkpoint to clear the first log group
    StripeId currentVsid = 0;

    UsedOffsetCalculator usedOffset(journal, logBufferSize);
    std::list<StripeTestFixture> writtenLogs;
    while (1)
    {
        StripeTestFixture stripe(currentVsid++, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);

        uint32_t logSize = sizeof(BlockWriteDoneLog) * stripe.GetBlockMapList().size()
            + sizeof(StripeMapUpdatedLog);
        if (usedOffset.CanBeWritten(logSize) == true)
        {
            writeTester->WriteLogsForStripe(stripe);
        }
        else
        {
            break;
        }

        writtenLogs.push_back(stripe);

        if (writtenLogs.size() == 1)
        {
            // When previous log group written with dummy data is ready
            // to be checkpointed, trigger checkpoint
            ExpectCheckpointTriggered();
            journal->StartCheckpoint();
        }
    }

    writeTester->WaitForAllLogWriteDone();

    SimulateSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    for (auto stripeLog : writtenLogs)
    {
        replayTester->ExpectReplayFullStripe(stripeLog);
    }

    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

// TODO (cheolho.kang): Create a separate file for scenarios that check Valid Block Count / Stripe Occupied Count
TEST_F(ReplayLogBufferIntegrationTest, DISABLED_ReplayFullSegmentWhenCheckpointFailEventIfSegmentContextFlushed)
{
    POS_TRACE_DEBUG(9999, "ReplayLogBufferIntegrationTest::ReplayFullSegmentWhenCheckpointFailEventIfSegmentContextFlushed");

    // Given: Set the log buffer size to only get logs for one segment write.
    // Given: Versioned Segment Context disabled
    uint32_t numStripesDoubleReplayed = 10;
    uint32_t targetSegmentId = 0;
    uint32_t numSegments = 1;
    uint32_t numTotalStripes = testInfo->numStripesPerSegment * numSegments;        // 64 * 1
    uint32_t numBlockMapLogsPerStripe = 8;
    uint64_t sizeLogGroupAlignByMpage = _CalculateLogGroupSize(numTotalStripes / 2, numBlockMapLogsPerStripe);
    uint32_t numStripesPerLogGroup = numTotalStripes / 2;
    JournalConfigurationBuilder builder(testInfo);
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(sizeLogGroupAlignByMpage * 2);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    // Given: Add logs for half of the segment to log group 0
    uint32_t stripeIndex = 0;
    std::vector<StripeTestFixture> writtenStripesForLogGroup0;
    for (stripeIndex = 0; stripeIndex < numTotalStripes / 2; stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGroup0.push_back(stripe);
    }
    // Given: 10 stripes are written to LogGroup1 before starting Checkpoint
    std::vector<StripeTestFixture> writtenStripesForLogGroup1;
    for (uint32_t i = 0; i < numStripesDoubleReplayed; i++, stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGroup1.push_back(stripe);
    }

    // When: Checkpoint Fail before LogGroupReset
    InjectCheckpointFaultAfterMetaFlushCompleted();

    // Given: Add all remaining logs
    std::vector<StripeTestFixture> writtenStripesForLogGroup1AfterCheckpoint;
    for (uint32_t i = 0; i < numTotalStripes / 2 - numStripesDoubleReplayed; i++, stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGroup1.push_back(stripe);
        writtenStripesForLogGroup1AfterCheckpoint.push_back(stripe);
    }
    writeTester->WaitForAllLogWriteDone();

    // When: SPOR
    SimulateSPORWithoutRecovery(builder);
    replayTester->ExpectReturningUnmapStripes();
    for (auto stripeLog : writtenStripesForLogGroup0)
    {
        replayTester->ExpectReplayFullStripe(stripeLog, false);
    }
    for (uint32_t index = 0; index < numTotalStripes / 2 - numStripesDoubleReplayed; index++)
    {
        replayTester->ExpectReplayFullStripe(writtenStripesForLogGroup1[index]);
    }

    // Then: Valid block count overflow occurs when performing a block map update for the 22th (numStripesPerLogGroup(32) - numStripeDobuleReplayed(10)) stripe in LogGroup 1
    int overflowedStripeId = numStripesPerLogGroup - numStripesDoubleReplayed;
    auto overflowedStripe = writtenStripesForLogGroup1[overflowedStripeId];
    auto overflowedBlockMapList = overflowedStripe.GetBlockMapList()[0];
    replayTester->ExpectReplaySegmentAllocation(overflowedStripe.GetUserAddr().stripeId);
    replayTester->ExpectReplayStripeAllocation(overflowedStripe.GetVsid(), overflowedStripe.GetWbAddr().stripeId);
    VirtualBlks virtualBlks = {
        .startVsa = {
            .stripeId = overflowedStripe.GetVsid(),
            .offset = 0},
        .numBlks = 1};
    EXPECT_CALL(*(testMapper->GetVSAMapMock()), SetVSAsWithSyncOpen(overflowedStripe.GetVolumeId(), overflowedBlockMapList.first, virtualBlks));
    try
    {
        journal->DoRecoveryForTest();
    }
    catch (const std::exception& e)
    {
        POS_TRACE_INFO(9999, "Expected assert fail, {}", e.what());
    }
}

TEST_F(ReplayLogBufferIntegrationTest, ReplayFullSegmentWhenCheckpointFailEventIfSegmentContextFlushedWhenVSCEnabled)
{
    POS_TRACE_DEBUG(9999, "ReplayLogBufferIntegrationTest::ReplayFullSegmentWhenCheckpointFailEventIfSegmentContextFlushedWhenVSCEnabled");

    // Given: Set the log buffer size to only get logs for one segment write.
    // Given: Versioned Segment Context Enabled
    uint32_t numStripesDoubleReplayed = 10;
    uint32_t targetSegmentId = 0;
    uint32_t numSegments = 1;
    uint32_t numTotalStripes = testInfo->numStripesPerSegment * numSegments;        // 64 * 1
    uint32_t numBlockMapLogsPerStripe = 8;
    uint64_t sizeLogGroupAlignByMpage = _CalculateLogGroupSize(numTotalStripes / 2, numBlockMapLogsPerStripe);
    JournalConfigurationBuilder builder(testInfo);
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(sizeLogGroupAlignByMpage * 2)
        ->SetVersionedSegmentContextEnable(true);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    // Given: Add logs for half of the segment to log group 0
    uint32_t stripeIndex = 0;
    std::vector<StripeTestFixture> writtenStripesForLogGroup0;
    for (stripeIndex = 0; stripeIndex < numTotalStripes / 2; stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGroup0.push_back(stripe);
    }
    // Given: 10 stripes are written to LogGroup1 before starting Checkpoint
    std::vector<StripeTestFixture> writtenStripesForLogGroup1;
    std::vector<StripeTestFixture> writtenStripesForLogGoup1BeforeCheckpoint;
    for (uint32_t index = 0; index < numStripesDoubleReplayed; index++, stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGroup1.push_back(stripe);
        writtenStripesForLogGoup1BeforeCheckpoint.push_back(stripe);
    }

    // When: Checkpoint Fail before LogGroupReset
    InjectCheckpointFaultAfterMetaFlushCompleted();

    // Given: Add all remaining logs
    for (; stripeIndex < numTotalStripes; stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGroup1.push_back(stripe);
    }
    writeTester->WaitForAllLogWriteDone();

    // When: SPOR
    SimulateSPORWithoutRecovery(builder);

    replayTester->ExpectReturningUnmapStripes();
    for (auto stripeLog : writtenStripesForLogGroup0)
    {
        replayTester->ExpectReplayFullStripe(stripeLog, false);
    }
    for (auto stripeLog : writtenStripesForLogGroup1)
    {
        replayTester->ExpectReplayFullStripe(stripeLog);
    }

    replayTester->ExpectReplayFlushedActiveStripe();

    // Then: Replay finished successfully
    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}
} // namespace pos
