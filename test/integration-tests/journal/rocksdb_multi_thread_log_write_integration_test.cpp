#include <functional>
#include <thread>
#include <vector>
#include <experimental/filesystem>

#include "gtest/gtest.h"

#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

namespace pos
{
class RocksDBMultiThreadLogWriteIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    RocksDBMultiThreadLogWriteIntegrationTest(void);
    virtual ~RocksDBMultiThreadLogWriteIntegrationTest(void) = default;
    JournalConfigurationBuilder builder;

    void WriteLogsWithSize(uint64_t size);
    void WaitForAllLogWriteDone(void);
    void WaitForThreads(void);

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    std::vector<std::thread> threadList;
};


RocksDBMultiThreadLogWriteIntegrationTest::RocksDBMultiThreadLogWriteIntegrationTest(void)
: JournalManagerTestFixture(GetLogDirName()),
builder(testInfo)
{
}

void
RocksDBMultiThreadLogWriteIntegrationTest::SetUp(void)
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
RocksDBMultiThreadLogWriteIntegrationTest::TearDown(void)
{
    WaitForThreads();

    std::this_thread::sleep_for(500ms);

    // Teardown : remove rocksdb log files by removing temporary directory.
    std::string targetDirName = rocksdbPath + "/" + GetLogDirName() + "_RocksJournal";
    int ret = std::experimental::filesystem::remove_all(targetDirName);

    // Remove SPOR directory
    std::string SPORDirectory = rocksdbPath + "/SPOR" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(SPORDirectory);
}

void
RocksDBMultiThreadLogWriteIntegrationTest::WriteLogsWithSize(uint64_t size)
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
RocksDBMultiThreadLogWriteIntegrationTest::WaitForAllLogWriteDone(void)
{
    writeTester->WaitForAllLogWriteDone();

    WaitForThreads();
}

void
RocksDBMultiThreadLogWriteIntegrationTest::WaitForThreads(void)
{
    for (auto& th : threadList)
    {
        th.join();
    }
    threadList.clear();
}

TEST_F(RocksDBMultiThreadLogWriteIntegrationTest, WriteLogs)
{
    POS_TRACE_DEBUG(9999, "RocksDBMultiThreadLogWriteIntegrationTest::WriteLogs");

    builder.SetJournalEnable(true)
        ->SetLogBufferSize(64 * 1024);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    WriteLogsWithSize(logGroupSize);
    WaitForAllLogWriteDone();

    writeTester->CompareLogs();
}

TEST_F(RocksDBMultiThreadLogWriteIntegrationTest, WriteLogsToTriggerCheckpoint)
{
    POS_TRACE_DEBUG(9999, "RocksDBMultiThreadLogWriteIntegrationTest::WriteLogsToTriggerCheckpoint");

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
