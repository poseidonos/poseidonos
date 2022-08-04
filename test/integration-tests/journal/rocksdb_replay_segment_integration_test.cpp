#include <experimental/filesystem>

#include "gtest/gtest.h"

#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

using ::testing::Return;

namespace pos
{
class RocksDBReplaySegmentIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    RocksDBReplaySegmentIntegrationTest(void);
    virtual ~RocksDBReplaySegmentIntegrationTest(void) = default;
    JournalConfigurationBuilder builder;

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);
};

RocksDBReplaySegmentIntegrationTest::RocksDBReplaySegmentIntegrationTest(void)
: JournalManagerTestFixture(GetLogDirName()),
builder(testInfo)
{
}

void
RocksDBReplaySegmentIntegrationTest::SetUp(void)
{
    testInfo->numStripesPerSegment = testInfo->numStripesPerSegment / 8;

    builder.SetRocksDBEnable(true);
    builder.SetRocksDBBasePath(rocksdbPath);
    // remove rocksdb log files by removing temporary directory if exist
    std::string targetDirName = rocksdbPath + "/" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(targetDirName);
    std::string SPORDirectory = rocksdbPath + "/SPOR" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(SPORDirectory);
}

void
RocksDBReplaySegmentIntegrationTest::TearDown(void)
{
    // Teardown : remove rocksdb log files by removing temporary directory.
    std::string targetDirName = rocksdbPath + "/" + GetLogDirName() + "_RocksJournal";
    int ret = std::experimental::filesystem::remove_all(targetDirName);

    // Remove SPOR directory
    std::string SPORDirectory = rocksdbPath + "/SPOR" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(SPORDirectory);
}

TEST_F(RocksDBReplaySegmentIntegrationTest, ReplaySegmentsWithPartial)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplaySegmentIntegrationTest::ReplaySegmentsWithPartial");

    InitializeJournal(builder.Build());

    uint32_t lengthOfPartialIndex = testInfo->numStripesPerSegment / 2;
    uint32_t numSegments = 5;
    uint32_t numTests = testInfo->numStripesPerSegment * numSegments - lengthOfPartialIndex;

    std::list<StripeTestFixture> writtenStripes;
    for (uint32_t index = lengthOfPartialIndex; index < numTests; index++)
    {
        StripeTestFixture stripe(index, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripes.push_back(stripe);
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

TEST_F(RocksDBReplaySegmentIntegrationTest, ReplaySegment)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplaySegmentIntegrationTest::ReplaySegment");

    InitializeJournal(builder.Build());

    uint32_t numSegments = 1;
    uint32_t numTests = testInfo->numStripesPerSegment * numSegments;

    std::list<StripeTestFixture> writtenStripes;
    for (uint32_t index = 0; index < numTests; index++)
    {
        StripeTestFixture stripe(index, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripes.push_back(stripe);
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

TEST_F(RocksDBReplaySegmentIntegrationTest, ReplayFullSegment)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplaySegmentIntegrationTest::ReplayFullSegment");

    InitializeJournal(builder.Build());

    uint32_t numSegments = 3;
    uint32_t numTests = testInfo->numStripesPerSegment * numSegments;

    std::list<StripeTestFixture> writtenStripes;
    for (uint32_t index = 0; index < numTests; index++)
    {
        StripeTestFixture stripe(index, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripes.push_back(stripe);
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

TEST_F(RocksDBReplaySegmentIntegrationTest, ReplayReusedSegment)
{
    POS_TRACE_DEBUG(9999, "RocksDBReplaySegmentIntegrationTest::ReplayReusedSegment");

    InitializeJournal(builder.Build());

    uint32_t numTestsBeforeSegmentFull = testInfo->numStripesPerSegment;
    uint32_t numTestbeforeGC = testInfo->numStripesPerSegment / 2;

    std::list<StripeTestFixture> writtenStripes;
    for (uint32_t i = numTestbeforeGC + 1; i < numTestsBeforeSegmentFull; i++)
    {
        StripeTestFixture stripe(i, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripes.push_back(stripe);
    }

    for (uint32_t i = 0; i <= numTestbeforeGC; i++)
    {
        StripeTestFixture stripe(i, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripes.push_back(stripe);
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
} // namespace pos
