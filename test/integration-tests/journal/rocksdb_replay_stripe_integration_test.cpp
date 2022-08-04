#include <experimental/filesystem>
#include "gtest/gtest.h"

#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;

namespace pos
{
class RocksDBReplayStripeIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    RocksDBReplayStripeIntegrationTest(void);
    virtual ~RocksDBReplayStripeIntegrationTest(void) = default;
    JournalConfigurationBuilder builder;

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);
};

RocksDBReplayStripeIntegrationTest::RocksDBReplayStripeIntegrationTest(void)
: JournalManagerTestFixture(GetLogDirName()),
builder(testInfo)
{
}

void
RocksDBReplayStripeIntegrationTest::SetUp(void)
{
    builder.SetRocksDBEnable(true);
    builder.SetRocksDBBasePath(rocksdbPath);
    // remove rocksdb log files by removing temporary directory if exist
    std::string targetDirName = rocksdbPath + "/" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(targetDirName);
    std::string SPORDirectory = rocksdbPath + "/SPOR" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(SPORDirectory);
}

void
RocksDBReplayStripeIntegrationTest::TearDown(void)
{
    // Teardown : remove rocksdb log files by removing temporary directory.
    std::string targetDirName = rocksdbPath + "/" + GetLogDirName() + "_RocksJournal";
    int ret = std::experimental::filesystem::remove_all(targetDirName);

    // Remove SPOR directory
    std::string SPORDirectory = rocksdbPath + "/SPOR" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(SPORDirectory);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplayOverwrite)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplayOverwrite");

    InitializeJournal(builder.Build());

    BlkAddr rba = std::rand() % testInfo->defaultTestVolSizeInBlock;
    StripeId vsid = std::rand() % testInfo->numUserStripes;
    uint32_t numBlksToOverwrite = std::rand() % testInfo->numBlksPerStripe + 1;

    StripeTestFixture stripe(vsid, testInfo->defaultTestVol);
    writeTester->WriteOverwrittenBlockLogs(stripe, rba, 0, numBlksToOverwrite);

    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    replayTester->ExpectReplayOverwrittenBlockLog(stripe);

    VirtualBlks writtenLastBlock = stripe.GetBlockMapList().back().second;
    VirtualBlkAddr tail = ReplayTestFixture::GetNextBlock(writtenLastBlock);
    replayTester->ExpectReplayUnflushedActiveStripe(tail, stripe);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplayFullStripe)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplayFullStripe");

    InitializeJournal(builder.Build());

    StripeId vsid = std::rand() % testInfo->numUserStripes;
    StripeTestFixture stripe(vsid, testInfo->defaultTestVol);
    writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
    writeTester->WriteLogsForStripe(stripe);

    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    replayTester->ExpectReplayFullStripe(stripe);
    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplayFullStripeSeveralTimes)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplayFullStripeSeveralTimes");

    InitializeJournal(builder.Build());

    std::list<StripeTestFixture> writtenStripes;

    uint32_t writtenLogSize = 0;
    uint32_t currentVsid = 0;
    while (writtenLogSize < logGroupSize / 1024)
    {
        StripeTestFixture stripe(currentVsid++, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripes.push_back(stripe);

        writtenLogSize += (stripe.GetBlockMapList().size() * sizeof(BlockWriteDoneLog));
        writtenLogSize += sizeof(StripeMapUpdatedLog);
    }

    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    for (auto stripeLog : writtenStripes)
    {
        replayTester->ExpectReplayFullStripe(stripeLog);
    }

    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplaySeveralUnflushedStripe)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplaySeveralUnflushedStripe");

    InitializeJournal(builder.Build());

    StripeTestFixture partialStripe(0, testInfo->defaultTestVol);
    int startOffset = std::rand() % testInfo->numBlksPerStripe;
    writeTester->GenerateLogsForStripe(partialStripe, startOffset, testInfo->numBlksPerStripe - startOffset);
    writeTester->WriteBlockLogsForStripe(partialStripe);

    StripeTestFixture fullStripe(1, testInfo->defaultTestVol);
    writeTester->GenerateLogsForStripe(fullStripe, 0, testInfo->numBlksPerStripe);
    writeTester->WriteBlockLogsForStripe(fullStripe);

    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    {
        InSequence s;

        replayTester->ExpectReplaySegmentAllocation(partialStripe.GetUserAddr().stripeId);
        replayTester->ExpectReplayStripeAllocation(partialStripe.GetVsid(), partialStripe.GetWbAddr().stripeId);
        replayTester->ExpectReplayBlockLogsForStripe(partialStripe.GetVolumeId(), partialStripe.GetBlockMapList());

        VirtualBlks writtenLastBlock = partialStripe.GetBlockMapList().back().second;
        VirtualBlkAddr tailVsa = ReplayTestFixture::GetNextBlock(writtenLastBlock);
        EXPECT_CALL(*(testAllocator->GetWBStripeAllocatorMock()),
            ReconstructActiveStripe(testInfo->defaultTestVol, partialStripe.GetWbAddr().stripeId, tailVsa, partialStripe.GetRevMap()));
        EXPECT_CALL(*(testAllocator->GetWBStripeAllocatorMock()),
            FinishStripe(partialStripe.GetWbAddr().stripeId, tailVsa));
    }

    {
        InSequence s;

        replayTester->ExpectReplaySegmentAllocation(fullStripe.GetUserAddr().stripeId);
        replayTester->ExpectReplayStripeAllocation(fullStripe.GetVsid(), fullStripe.GetWbAddr().stripeId);
        replayTester->ExpectReplayBlockLogsForStripe(fullStripe.GetVolumeId(), fullStripe.GetBlockMapList());

        VirtualBlks writtenLastBlock = fullStripe.GetBlockMapList().back().second;

        VirtualBlkAddr tail = ReplayTestFixture::GetNextBlock(writtenLastBlock);
        EXPECT_CALL(*(testAllocator->GetWBStripeAllocatorMock()),
            ReconstructActiveStripe(testInfo->defaultTestVol, fullStripe.GetWbAddr().stripeId, tail, fullStripe.GetRevMap()));
        EXPECT_CALL(*(testAllocator->GetIContextReplayerMock()),
            SetActiveStripeTail(testInfo->defaultTestVol, tail, fullStripe.GetWbAddr().stripeId))
            .Times(1);
    }

    EXPECT_CALL(*(testAllocator->GetIContextReplayerMock()), ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplayBlockWritesFromStart)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplayBlockWritesFromStart");

    InitializeJournal(builder.Build());

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    int numBlks = std::rand() % (testInfo->numBlksPerStripe - 1) + 1;
    writeTester->GenerateLogsForStripe(stripe, 0, numBlks);
    writeTester->WriteBlockLogsForStripe(stripe);

    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    {
        InSequence s;

        replayTester->ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
        replayTester->ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
        replayTester->ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), stripe.GetBlockMapList());
    }

    VirtualBlks writtenLastBlock = stripe.GetBlockMapList().back().second;

    VirtualBlkAddr tail = ReplayTestFixture::GetNextBlock(writtenLastBlock);
    replayTester->ExpectReplayUnflushedActiveStripe(tail, stripe);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplayBlockWrites)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplayBlockWrites");

    InitializeJournal(builder.Build());

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    int numBlks = std::rand() % (testInfo->numBlksPerStripe - 1) + 1;
    writeTester->GenerateLogsForStripe(stripe, testInfo->numBlksPerStripe - numBlks, numBlks);
    writeTester->WriteBlockLogsForStripe(stripe);
    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    replayTester->ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
    replayTester->ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
    replayTester->ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), stripe.GetBlockMapList());

    VirtualBlks writtenLastBlock = stripe.GetBlockMapList().back().second;
    VirtualBlkAddr tail = ReplayTestFixture::GetNextBlock(writtenLastBlock);

    replayTester->ExpectReplayUnflushedActiveStripe(tail, stripe);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplayBlockWrites_WhenStripeMapCheckpointed)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplayBlockWrites_WhenStripeMapCheckpointed");

    InitializeJournal(builder.Build());

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    int numBlks = std::rand() % (testInfo->numBlksPerStripe - 1) + 1;
    writeTester->GenerateLogsForStripe(stripe, testInfo->numBlksPerStripe - numBlks, numBlks);
    writeTester->WriteBlockLogsForStripe(stripe);
    writeTester->WaitForAllLogWriteDone();

    SimulateRocksDBSPORWithoutRecovery();

    StripeAddr stroedAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = stripe.GetWbLsid(stripe.GetVsid())};
    replayTester->ExpectReturningStripeAddr(stripe.GetVsid(), stroedAddr);
    replayTester->ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
    replayTester->ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), stripe.GetBlockMapList());

    VirtualBlks writtenLastBlock = stripe.GetBlockMapList().back().second;
    VirtualBlkAddr tail = ReplayTestFixture::GetNextBlock(writtenLastBlock);

    replayTester->ExpectReplayUnflushedActiveStripe(tail, stripe);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplayBlockWritesFromStartToEnd)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplayBlockWritesFromStartToEnd");

    InitializeJournal(builder.Build());

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
    writeTester->WriteBlockLogsForStripe(stripe);

    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    {
        InSequence s;

        replayTester->ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
        replayTester->ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
        replayTester->ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), stripe.GetBlockMapList());
    }

    VirtualBlkAddr tail = {
        .stripeId = stripe.GetVsid(),
        .offset = testInfo->numBlksPerStripe};

    replayTester->ExpectReplayUnflushedActiveStripe(tail, stripe);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplayBlockWritesAndFlush)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplayBlockWritesAndFlush");

    InitializeJournal(builder.Build());

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    int numBlks = std::rand() % (testInfo->numBlksPerStripe - 1) + 1;
    uint32_t startOffset = testInfo->numBlksPerStripe - numBlks;
    writeTester->GenerateLogsForStripe(stripe, startOffset, numBlks);
    writeTester->WriteBlockLogsForStripe(stripe);
    bool writeSuccessful = writeTester->WriteStripeLog(stripe.GetVsid(), stripe.GetWbAddr(), stripe.GetUserAddr());
    EXPECT_TRUE(writeSuccessful == true);

    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();

    {
        InSequence s;

        replayTester->ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
        replayTester->ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
        replayTester->ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), stripe.GetBlockMapList());
        replayTester->ExpectReplayStripeFlush(stripe);
    }

    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplayBlockWritesAndFlush_WhenStripeMapCheckpointed)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplayBlockWritesAndFlush_WhenStripeMapCheckpointed");

    InitializeJournal(builder.Build());

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    int numBlks = std::rand() % (testInfo->numBlksPerStripe - 1) + 1;
    uint32_t startOffset = testInfo->numBlksPerStripe - numBlks;
    writeTester->GenerateLogsForStripe(stripe, startOffset, numBlks);
    writeTester->WriteBlockLogsForStripe(stripe);
    bool writeSuccessful = writeTester->WriteStripeLog(stripe.GetVsid(), stripe.GetWbAddr(), stripe.GetUserAddr());
    EXPECT_TRUE(writeSuccessful == true);

    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    StripeAddr stroedAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = stripe.GetWbAddr().stripeId};
    replayTester->ExpectReturningStripeAddr(stripe.GetVsid(), stroedAddr);

    {
        InSequence s;
        replayTester->ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), stripe.GetBlockMapList());
        replayTester->ExpectReplayStripeFlush(stripe);
    }

    replayTester->ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplayBlockWritesAndFlush_WhenStripeMapCheckpointed2)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplayBlockWritesAndFlush_WhenStripeMapCheckpointed2");

    InitializeJournal(builder.Build());

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    int numBlks = std::rand() % (testInfo->numBlksPerStripe - 1) + 1;
    uint32_t startOffset = testInfo->numBlksPerStripe - numBlks;
    writeTester->GenerateLogsForStripe(stripe, startOffset, numBlks);
    writeTester->WriteBlockLogsForStripe(stripe);
    bool writeSuccessful = writeTester->WriteStripeLog(stripe.GetVsid(), stripe.GetWbAddr(), stripe.GetUserAddr());
    EXPECT_TRUE(writeSuccessful == true);

    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    StripeAddr stroedAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = stripe.GetUserAddr().stripeId};
    replayTester->ExpectReturningStripeAddr(stripe.GetVsid(), stroedAddr);

    {
        InSequence s;
        replayTester->ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), stripe.GetBlockMapList());
    }

    replayTester->ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplayFlush)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplayFlushButMapNotUpdated");

    InitializeJournal(builder.Build());

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    bool writeSuccessful = writeTester->WriteStripeLog(stripe.GetVsid(), stripe.GetWbAddr(), stripe.GetUserAddr());
    EXPECT_TRUE(writeSuccessful == true);

    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    replayTester->ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
    replayTester->ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
    replayTester->ExpectReplayStripeFlush(stripe);

    EXPECT_CALL(*(testAllocator->GetIContextReplayerMock()), ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayStripeIntegrationTest, ReplayGcStripe)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayStripeIntegrationTest::ReplayGcStripe");

    InitializeJournal(builder.Build());

    StripeTestFixture stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);

    bool writeSuccessful = writeTester->WriteGcStripeLog(testInfo->defaultTestVol, stripe);
    EXPECT_TRUE(writeSuccessful == true);

    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    replayTester->ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
    replayTester->ExpectReplayBlockLogsForStripe(testInfo->defaultTestVol, stripe.GetBlockMapList());
    replayTester->ExpectReplayStripeFlush(stripe);

    EXPECT_CALL(*(testAllocator->GetIContextReplayerMock()), ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}
} // namespace pos
