#include <experimental/filesystem>
#include <iostream>

#include "gtest/gtest.h"

#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"
#include "test/integration-tests/journal/utils/used_offset_calculator.h"
#include "src/journal_manager/log/log_event.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::Return;

namespace pos
{
class RocksDBReplayLogBufferIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    RocksDBReplayLogBufferIntegrationTest(void);
    virtual ~RocksDBReplayLogBufferIntegrationTest(void) = default;
    JournalConfigurationBuilder builder;

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);
};

RocksDBReplayLogBufferIntegrationTest::RocksDBReplayLogBufferIntegrationTest(void)
:JournalManagerTestFixture(GetLogDirName()),
builder(testInfo)
{
}

void
RocksDBReplayLogBufferIntegrationTest::SetUp(void)
{
    builder.SetRocksDBEnable(true);

    // remove rocksdb log files by removing temporary directory if exist
    std::string targetDirName = "/etc/pos/POSRaid/" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(targetDirName);
    std::string SPORDirectory = "/etc/pos/POSRaid/SPOR" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(SPORDirectory);
}

void
RocksDBReplayLogBufferIntegrationTest::TearDown(void)
{
    // Teardown : remove rocksdb log files by removing temporary directory.
    std::string targetDirName = "/etc/pos/POSRaid/" + GetLogDirName() + "_RocksJournal";
    int ret = std::experimental::filesystem::remove_all(targetDirName);

    // Remove SPOR directory
    std::string SPORDirectory = "/etc/pos/POSRaid/SPOR" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(SPORDirectory);
}

TEST_F(RocksDBReplayLogBufferIntegrationTest, ReplayFullLogBuffer)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayLogBufferIntegrationTest::ReplaySeveralLogGroup");

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
    SimulateRocksDBSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    replayTester->ExpectReplayOverwrittenBlockLog(stripe);

    VirtualBlks writtenLastBlock = stripe.GetBlockMapList().back().second;
    VirtualBlkAddr tail = ReplayTestFixture::GetNextBlock(writtenLastBlock);
    replayTester->ExpectReplayUnflushedActiveStripe(tail, stripe);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBReplayLogBufferIntegrationTest, ReplayCirculatedLogBuffer)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplayLogBufferIntegrationTest::ReplayCirculatedLogBuffer");

    builder.SetJournalEnable(true)
        ->SetLogBufferSize(16 * 1024);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    // Write dummy logs to the first log group (to be cleared by checkpoint later)
    writeTester->WriteLogsWithSize(logGroupSize);

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

    SimulateRocksDBSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    for (auto stripeLog : writtenLogs)
    {
        replayTester->ExpectReplayFullStripe(stripeLog);
    }

    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}
} // namespace pos
