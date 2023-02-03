#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "src/allocator/include/allocator_const.h"
#include "src/journal_manager/log/log_event.h"
#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"
#include "test/integration-tests/journal/utils/used_offset_calculator.h"
#include "test/integration-tests/journal/fake/i_segment_ctx_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::Mock;

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
    writeTester->WriteLogsWithSize(logGroupSize - sizeof(LogGroupFooter));

    // Write logs to fill log buffer, and start checkpoint to clear the first log group
    StripeId currentVsid = 0;

    UsedOffsetCalculator usedOffset(journal, logBufferSize);
    std::list<StripeTestFixture> writtenLogs;
    while (1)
    {
        StripeTestFixture stripe(currentVsid++, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);

        uint32_t logSize = sizeof(BlockWriteDoneLog) * stripe.GetBlockMapList().size() + sizeof(StripeMapUpdatedLog);
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

TEST_F(ReplayLogBufferIntegrationTest, ReplayFullSegmentWhenCheckpointFailEventIfSegmentContextFlushed)
{
    POS_TRACE_DEBUG(9999, "ReplayLogBufferIntegrationTest::ReplayFullSegmentWhenCheckpointFailEventIfSegmentContextFlushed");

    // Given: 하나의 segment write에 대한 log까지만 받을 수 있는 log buffer 크기로 조정
    // Given: Checkpoint triggering은 disable
    uint32_t numDoubleReplayed = 10;
    uint32_t targetSegmentId = 0;
    uint32_t numSegments = 1;
    uint32_t numTotalStripes = testInfo->numStripesPerSegment * numSegments;
    uint32_t numBlockMapLogsPerStripe = 8;
    uint64_t sizeLogGroupAlignByMpage = _CalculateLogGroupSize(numTotalStripes / 2, numBlockMapLogsPerStripe);
    JournalConfigurationBuilder builder(testInfo);
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(sizeLogGroupAlignByMpage * 2);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    // Given: Segment의 절반에 대한 write는 log group 0에 add
    uint32_t stripeIndex = 0;
    std::vector<StripeTestFixture> writtenStripesForLogGoup0;
    for (stripeIndex = 0; stripeIndex < numTotalStripes / 2; stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGoup0.push_back(stripe);
    }
    // Given: 이후 로그는 모두 log group1에 적지만, 그중 10개의 stripe은 checkpoint 전에 적힌 stripe으로 가정
    std::vector<StripeTestFixture> writtenStripesForLogGoup1;
    for (uint32_t index = 0; index < numDoubleReplayed; index++, stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGoup1.push_back(stripe);
    }

    // When: Checkpoint 중 LogGroupReset에서 SPOR
    InjectCheckpointFaultAfterMetaFlushCompleted();

    // Given: 나머지 로그 모두 writre
    std::vector<StripeTestFixture> writtenStripesForLogGoup1AfterCheckpoint;
    for (uint32_t index = 0; index < numTotalStripes / 2 - numDoubleReplayed; index++, stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGoup1.push_back(stripe);
        writtenStripesForLogGoup1AfterCheckpoint.push_back(stripe);
    }
    writeTester->WaitForAllLogWriteDone();

    SimulateSPORWithoutRecovery(builder);

    replayTester->ExpectReturningUnmapStripes();
    for (auto stripeLog : writtenStripesForLogGoup0)
    {
        replayTester->ExpectReplayFullStripeWithoutReplaySegmentContex(stripeLog);
    }
    for (uint32_t index = 0; index < numTotalStripes / 2 - numDoubleReplayed; index++)
    {
        replayTester->ExpectReplayFullStripe(writtenStripesForLogGoup1[index]);
    }
    // Given 55번째 (32 + 22 + 1) stripe에 대한 block map update를 진행할 때 valid block count overflow 발생
    auto overflowedStripe = writtenStripesForLogGoup1[22];
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
        // FAIL();
    }
    catch(const std::exception& e)
    {
        // SUCCEED();
        POS_TRACE_INFO(9999,
            "Expected assert fail, Valid block count was redundantly updated. error_msg: {}", e.what());
    }
}

TEST_F(ReplayLogBufferIntegrationTest, ReplayFullSegmentWhenCheckpointFailEventIfSegmentContextFlushedWhenVSCEnabled)
{
    POS_TRACE_DEBUG(9999, "ReplayLogBufferIntegrationTest::ReplayFullSegmentWhenCheckpointFailEventIfSegmentContextFlushedWhenVSCEnabled");

    // Given: 하나의 segment write에 대한 log까지만 받을 수 있는 log buffer 크기로 조정
    // Given: Versioned Segment Checkpoint Enable!
    // Given: Checkpoint triggering은 disable
    uint32_t numDoubleReplayed = 10;
    uint32_t targetSegmentId = 0;
    uint32_t numSegments = 1;
    uint32_t numTotalStripes = testInfo->numStripesPerSegment * numSegments;
    uint32_t numBlockMapLogsPerStripe = 8;
    uint64_t sizeLogGroupAlignByMpage = _CalculateLogGroupSize(numTotalStripes / 2, numBlockMapLogsPerStripe);
    JournalConfigurationBuilder builder(testInfo);
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(sizeLogGroupAlignByMpage * 2)
        ->SetVersionedSegmentContextEnable(true);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    // Given: Segment의 절반에 대한 write는 log group 0에 add
    uint32_t stripeIndex = 0;
    std::vector<StripeTestFixture> writtenStripesForLogGoup0;
    for (stripeIndex = 0; stripeIndex < numTotalStripes / 2; stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGoup0.push_back(stripe);
    }
    // Given: 이후 로그는 모두 log group1에 적지만, 그중 10개의 stripe은 checkpoint 전에 적힌 stripe으로 가정
    std::vector<StripeTestFixture> writtenStripesForLogGoup1;
    std::vector<StripeTestFixture> writtenStripesForLogGoup1BeforeCheckpoint;
    for (uint32_t index = 0; index < numDoubleReplayed; index++, stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGoup1.push_back(stripe);
        writtenStripesForLogGoup1BeforeCheckpoint.push_back(stripe);
    }

    // When: Checkpoint 중 LogGroupReset에서 SPOR
    InjectCheckpointFaultAfterMetaFlushCompleted();

    // Given: 나머지 로그 모두 writre
    for (; stripeIndex < numTotalStripes; stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGoup1.push_back(stripe);
    }
    writeTester->WaitForAllLogWriteDone();

    SimulateSPORWithoutRecovery(builder);

    replayTester->ExpectReturningUnmapStripes();
    for (auto stripeLog : writtenStripesForLogGoup0)
    {
        replayTester->ExpectReplayFullStripeWithoutReplaySegmentContex(stripeLog);
    }
    for (auto stripeLog : writtenStripesForLogGoup1)
    {
        replayTester->ExpectReplayFullStripe(stripeLog);
    }
    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}
} // namespace pos
