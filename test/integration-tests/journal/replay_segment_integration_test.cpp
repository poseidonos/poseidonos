#include "gtest/gtest.h"

#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

using ::testing::Return;

namespace pos
{
class ReplaySegmentIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    ReplaySegmentIntegrationTest(void);
    virtual ~ReplaySegmentIntegrationTest(void) = default;

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);
};

ReplaySegmentIntegrationTest::ReplaySegmentIntegrationTest(void)
: JournalManagerTestFixture(GetLogFileName())
{
}

void
ReplaySegmentIntegrationTest::SetUp(void)
{
    testInfo->numStripesPerSegment = testInfo->numStripesPerSegment / 8;
}

void
ReplaySegmentIntegrationTest::TearDown(void)
{
}

TEST_F(ReplaySegmentIntegrationTest, ReplaySegmentsWithPartial)
{
    POS_TRACE_DEBUG(9999, "ReplaySegmentIntegrationTest::ReplaySegmentsWithPartial");

    InitializeJournal();

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
    SimulateSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    for (auto stripeLog : writtenStripes)
    {
        replayTester->ExpectReplayFullStripe(stripeLog);
    }

    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(ReplaySegmentIntegrationTest, ReplaySegment)
{
    POS_TRACE_DEBUG(9999, "ReplaySegmentIntegrationTest::ReplaySegment");

    InitializeJournal();

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
    SimulateSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    for (auto stripeLog : writtenStripes)
    {
        replayTester->ExpectReplayFullStripe(stripeLog);
    }

    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(ReplaySegmentIntegrationTest, ReplayFullSegment)
{
    POS_TRACE_DEBUG(9999, "ReplaySegmentIntegrationTest::ReplayFullSegment");

    InitializeJournal();

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
    SimulateSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    for (auto stripeLog : writtenStripes)
    {
        replayTester->ExpectReplayFullStripe(stripeLog);
    }

    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(ReplaySegmentIntegrationTest, ReplayReusedSegment)
{
    POS_TRACE_DEBUG(9999, "ReplaySegmentIntegrationTest::ReplayReusedSegment");

    InitializeJournal();

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
    SimulateSPORWithoutRecovery();

    replayTester->ExpectReturningUnmapStripes();
    for (auto stripeLog : writtenStripes)
    {
        replayTester->ExpectReplayFullStripe(stripeLog);
    }

    replayTester->ExpectReplayFlushedActiveStripe();

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}
} // namespace pos
