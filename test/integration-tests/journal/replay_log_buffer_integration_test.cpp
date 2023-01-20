#include <iostream>

#include "gtest/gtest.h"
#include "src/allocator/include/allocator_const.h"
#include "src/journal_manager/log/log_event.h"
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
    std::vector<StripeTestFixture> writtenStripesForLogGoup1BeforeCheckpoint;
    for (uint32_t index = 0; index < 10; index++, stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGoup1.push_back(stripe);
        writtenStripesForLogGoup1BeforeCheckpoint.push_back(stripe);
    }
    std::vector<StripeTestFixture> writtenStripesForLogGoup1AfterCheckpoint;
    for (uint32_t index = 0; index < numTotalStripes / 2 - 10; index++, stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGoup1.push_back(stripe);
        writtenStripesForLogGoup1AfterCheckpoint.push_back(stripe);
    }
    writeTester->WaitForAllLogWriteDone();

    // When: Checkpoint가 발생했을 때 segment context는 flush가 완료, VSAMap은 flush fail
    uint64_t latestContextVersion = 1;
    EXPECT_CALL(*(testAllocator->GetIContextManagerMock()), GetStoredContextVersion(SEGMENT_CTX))
        .WillOnce(Return(0))
        .WillRepeatedly(Return(1));
    EXPECT_CALL(*testMapper, FlushDirtyMpages).Times(1);
    ON_CALL(*testMapper, FlushDirtyMpages).WillByDefault([&](int volId, EventSmartPtr event) {
        ((LogGroupReleaserSpy*)(journal->GetLogGroupReleaser()))->ForceCompleteCheckpoint();
        return -1;
    });

    // When: Checkpoint 중 SPOR
    journal->StartCheckpoint();
    WaitForAllCheckpointDone();
    SimulateSPORWithoutRecovery(builder);

    // When: Replay 시 segment context에는 writtenStripesForLogGoup0 + writtenStripesForLogGoup1BeforeCheckpoint 에 해당하는 valid block count가 이미 기록되어 있음
    testInfo->numUserSegments;
    uint32_t* actualValidCount = new uint32_t[testInfo->numUserSegments];
    uint32_t numStripesBeforeCheckpoint = writtenStripesForLogGoup0.size() + writtenStripesForLogGoup1BeforeCheckpoint.size();
    actualValidCount[targetSegmentId] = numStripesBeforeCheckpoint * testInfo->numBlksPerStripe;

    // Given: Replay로 호출되는 Validate는 이곳에서 actualValidCount를 증가시키도록 함
    // 우선은 해당 IT의 유효성을 검증하는 중이라, 유의미하다면, 추후 occupied count 코드 또한 같이 추가할 예정
    ON_CALL(*(testAllocator->GetISegmentCtxMock()), ValidateBlks).WillByDefault([&](VirtualBlks blks) {
        SegmentId segmentId = blks.startVsa.stripeId / testInfo->numStripesPerSegment;
        actualValidCount[segmentId]++;
        return true;
    });

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

    // Then: 예상하는 valid block count보다 writtenStripesForLogGoup1BeforeCheckpoint에 해당하는 block count 만큼 더 커야함.
    uint32_t expectedValidCount = numTotalStripes * testInfo->numBlksPerStripe;
    EXPECT_NE(expectedValidCount, actualValidCount[targetSegmentId]);
    expectedValidCount = (numTotalStripes + writtenStripesForLogGoup1BeforeCheckpoint.size()) * testInfo->numBlksPerStripe;
    EXPECT_EQ(expectedValidCount, actualValidCount[targetSegmentId]);

    delete[] actualValidCount;
}

TEST_F(ReplayLogBufferIntegrationTest, ReplayFullSegmentWhenCheckpointFailEventIfSegmentContextFlushedWhenVSCEnabled)
{
    POS_TRACE_DEBUG(9999, "ReplayLogBufferIntegrationTest::ReplayFullSegmentWhenCheckpointFailEventIfSegmentContextFlushedWhenVSCEnabled");

    // Given: 하나의 segment write에 대한 log까지만 받을 수 있는 log buffer 크기로 조정
    // Given: Versioned Segment Checkpoint Enable!
    // Given: Checkpoint triggering은 disable
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
    for (uint32_t index = 0; index < 10; index++, stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGoup1.push_back(stripe);
        writtenStripesForLogGoup1BeforeCheckpoint.push_back(stripe);
    }
    std::vector<StripeTestFixture> writtenStripesForLogGoup1AfterCheckpoint;
    for (uint32_t index = 0; index < numTotalStripes / 2 - 10; index++, stripeIndex++)
    {
        StripeTestFixture stripe(stripeIndex, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripesForLogGoup1.push_back(stripe);
        writtenStripesForLogGoup1AfterCheckpoint.push_back(stripe);
    }
    writeTester->WaitForAllLogWriteDone();

    // When: Checkpoint가 발생했을 때 segment context는 flush가 완료, VSAMap은 flush fail
    uint64_t latestContextVersion = 1;
    EXPECT_CALL(*(testAllocator->GetIContextManagerMock()), GetStoredContextVersion(SEGMENT_CTX))
        .WillOnce(Return(0))
        .WillRepeatedly(Return(1));
    EXPECT_CALL(*testMapper, FlushDirtyMpages).Times(1);
    ON_CALL(*testMapper, FlushDirtyMpages).WillByDefault([&](int volId, EventSmartPtr event) {
        ((LogGroupReleaserSpy*)(journal->GetLogGroupReleaser()))->ForceCompleteCheckpoint();
        return -1;
    });

    // Checkpoint시 사용할 segment context 정보 get
    SegmentInfo* segInfos = journal->GetVersionedSegmentContext()->GetUpdatedInfoToFlush(0);

    // When: Checkpoint 중 SPOR
    journal->StartCheckpoint();
    WaitForAllCheckpointDone();
    SimulateSPORWithoutRecovery(builder);

    // When: Replay 시 segment context에는 writtenStripesForLogGoup0에 해당하는 valid block count가 이미 기록되어 있음
    testInfo->numUserSegments;
    uint32_t* actualValidCount = new uint32_t[testInfo->numUserSegments];
    actualValidCount[targetSegmentId] = segInfos[targetSegmentId].GetValidBlockCount();

    // Given: Replay로 호출되는 Validate는 이곳에서 actualValidCount를 증가시키도록 함
    ON_CALL(*(testAllocator->GetISegmentCtxMock()), ValidateBlks).WillByDefault([&](VirtualBlks blks) {
        SegmentId segmentId = blks.startVsa.stripeId / testInfo->numStripesPerSegment;
        actualValidCount[segmentId]++;
        return true;
    });
    ON_CALL(*(testAllocator->GetISegmentCtxMock()), InvalidateBlks).WillByDefault([&](VirtualBlks blks, bool isForced) {
        SegmentId segmentId = blks.startVsa.stripeId / testInfo->numStripesPerSegment;
        actualValidCount[segmentId]--;
        return true;
    });

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

    // Then: 예상하는 valid block count와 정확히 일치해야 함
    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
    uint32_t expectedValidCount = numTotalStripes * testInfo->numBlksPerStripe;
    EXPECT_EQ(expectedValidCount, actualValidCount[targetSegmentId]);

    delete[] actualValidCount;
}
} // namespace pos
