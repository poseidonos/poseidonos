#include <functional>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

namespace pos
{
class MultiThreadLogWriteIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    MultiThreadLogWriteIntegrationTest(void);
    virtual ~MultiThreadLogWriteIntegrationTest(void) = default;

    void WriteLogsWithSize(uint64_t size);
    void WaitForAllLogWriteDone(void);
    void WaitForThreads(void);

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    std::vector<std::thread> threadList;
};


MultiThreadLogWriteIntegrationTest::MultiThreadLogWriteIntegrationTest(void)
: JournalManagerTestFixture(GetLogFileName())
{
}

void
MultiThreadLogWriteIntegrationTest::SetUp(void)
{
}

void
MultiThreadLogWriteIntegrationTest::TearDown(void)
{
    WaitForThreads();

    std::this_thread::sleep_for(500ms);
}

void
MultiThreadLogWriteIntegrationTest::WriteLogsWithSize(uint64_t size)
{
    StripeId currentVsid = 0;
    uint64_t roughUsedSizeCalculation = 0;

    while (roughUsedSizeCalculation < size)
    {
        StripeTestFixture stripe(currentVsid++, testInfo->defaultTestVol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);

        for (auto blkLog : stripe.GetBlockMapList())
        {
            threadList.push_back(std::thread(&LogWriteTestFixture::WriteBlockLog,
                writeTester, stripe.GetVolumeId(), blkLog.first, blkLog.second));

            roughUsedSizeCalculation += sizeof(BlockWriteDoneLog);
            if (roughUsedSizeCalculation >= size)
            {
                break;
            }
        }

        WaitForThreads();

        writeTester->WriteStripeLog(stripe.GetVsid(), stripe.GetWbAddr(),
            stripe.GetUserAddr());

        roughUsedSizeCalculation += sizeof(StripeMapUpdatedLog);
        if (roughUsedSizeCalculation >= size)
        {
            break;
        }
    }
}

void
MultiThreadLogWriteIntegrationTest::WaitForAllLogWriteDone(void)
{
    writeTester->WaitForAllLogWriteDone();

    WaitForThreads();
}

void
MultiThreadLogWriteIntegrationTest::WaitForThreads(void)
{
    for (auto& th : threadList)
    {
        th.join();
    }
    threadList.clear();
}

TEST_F(MultiThreadLogWriteIntegrationTest, WriteLogs)
{
    POS_TRACE_DEBUG(9999, "MultiThreadLogWriteIntegrationTest::WriteLogs");

    JournalConfigurationBuilder builder(testInfo);
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(64 * 1024);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    WriteLogsWithSize(logGroupSize);
    WaitForAllLogWriteDone();

    writeTester->CompareLogs();
}

TEST_F(MultiThreadLogWriteIntegrationTest, WriteLogsToTriggerCheckpoint)
{
    POS_TRACE_DEBUG(9999, "MultiThreadLogWriteIntegrationTest::WriteLogsToTriggerCheckpoint");

    JournalConfigurationBuilder builder(testInfo);
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(64 * 1024);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(true);

    ExpectCheckpointTriggered();

    WriteLogsWithSize(logBufferSize + logGroupSize);

    WaitForAllLogWriteDone();

    SetTriggerCheckpoint(false);
    WaitForAllCheckpointDone();
}
} // namespace pos
