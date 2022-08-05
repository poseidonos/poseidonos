#include <experimental/filesystem>

#include "gtest/gtest.h"
#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

using ::testing::AtLeast;

namespace pos
{
class RocksDBLogWriteIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    RocksDBLogWriteIntegrationTest(void);
    virtual ~RocksDBLogWriteIntegrationTest(void) = default;
    JournalConfigurationBuilder builder;

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);
};

RocksDBLogWriteIntegrationTest::RocksDBLogWriteIntegrationTest(void)
: JournalManagerTestFixture(GetLogDirName()),
builder(testInfo)
{
}

void
RocksDBLogWriteIntegrationTest::SetUp(void)
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
RocksDBLogWriteIntegrationTest::TearDown(void)
{
    // Teardown : remove rocksdb log files by removing temporary directory.
    std::string targetDirName = rocksdbPath + "/" + GetLogDirName() + "_RocksJournal";
    int ret = std::experimental::filesystem::remove_all(targetDirName);

    // Remove SPOR directory
    std::string SPORDirectory = rocksdbPath + "/SPOR" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(SPORDirectory);
}

TEST_F(RocksDBLogWriteIntegrationTest, WriteLogWithJournalDisabled)
{
    POS_TRACE_DEBUG(9999, "RocksDBLogWriteIntegrationTest::WriteLogWithJournalDisabled");

    builder.SetJournalEnable(false);

    InitializeJournal(builder.Build());

    VirtualBlkAddr vsa = {.stripeId = 0, .offset = testInfo->numTest};
    VirtualBlks blks = {.startVsa = vsa, .numBlks = testInfo->numTest};
    bool writeSuccessful = writeTester->WriteBlockLog(testInfo->defaultTestVol, 0, blks);
    EXPECT_TRUE(writeSuccessful == false);

    LogList readLogs;
    int result = journal->GetLogs(readLogs);

    EXPECT_TRUE(result == 0);
    EXPECT_TRUE(readLogs.IsEmpty() == true);
}

TEST_F(RocksDBLogWriteIntegrationTest, WriteLog_UserStripe)
{
    POS_TRACE_DEBUG(9999, "RocksDBLogWriteIntegrationTest::WriteLog_UserStripe");
    InitializeJournal(builder.Build());

    StripeId vsid = 100;

    // Write block map updated log
    VirtualBlkAddr vsa = {.stripeId = vsid, .offset = 0};
    VirtualBlks blks = {.startVsa = vsa, .numBlks = (uint32_t)testInfo->numBlksPerStripe};

    bool writeSuccessful = writeTester->WriteBlockLog(testInfo->defaultTestVol, 0, blks);
    EXPECT_TRUE(writeSuccessful == true);
    writeTester->WaitForAllLogWriteDone();

    EXPECT_TRUE(journal->GetNumLogsAdded() == 1);

    // Write stripe map updated log
    StripeAddr oldAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = StripeTestFixture::GetWbLsid(blks.startVsa.stripeId)};

    StripeAddr newAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = vsid};

    writeSuccessful = writeTester->WriteStripeLog(vsid, oldAddr, newAddr);
    EXPECT_TRUE(writeSuccessful == true);
    writeTester->WaitForAllLogWriteDone();

    EXPECT_TRUE(journal->GetNumLogsAdded() == 2);
    writeTester->CompareLogs();
}

TEST_F(RocksDBLogWriteIntegrationTest, WriteLog_GcStripe)
{
    POS_TRACE_DEBUG(9999, "RocksDBLogWriteIntegrationTest::WriteLog_GcStripe");
    InitializeJournal(builder.Build());

    int volumeId = testInfo->defaultTestVol;
    StripeId vsid = 200;
    StripeId wbLsid = 3;
    StripeId userLsid = 200;

    bool writeSuccessful = writeTester->WriteGcStripeLog(volumeId, vsid, wbLsid, userLsid);
    EXPECT_TRUE(writeSuccessful == true);
    writeTester->WaitForAllLogWriteDone();

    EXPECT_TRUE(journal->GetNumLogsAdded() == 2);
    writeTester->CompareLogs();
}

TEST_F(RocksDBLogWriteIntegrationTest, DISABLED_WriteLog_GcStripes)
{
    POS_TRACE_DEBUG(9999, "RocksDBLogWriteIntegrationTest::WriteLog_GcStripe");
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(4160 * 2);
    testInfo->numBlksPerStripe *= 2;
    InitializeJournal(builder.Build());
    EXPECT_CALL(*(testAllocator->GetIContextManagerMock()),
        FlushContexts)
        .Times(AtLeast(1));

    int volumeId = testInfo->defaultTestVol;
    StripeId vsid = 200;
    StripeId wbLsid = 3;
    StripeId userLsid = 200;

    bool writeSuccessful = writeTester->WriteGcStripeLog(volumeId, vsid, wbLsid, userLsid);
    EXPECT_TRUE(writeSuccessful == true);
    writeTester->WaitForAllLogWriteDone();

    EXPECT_EQ(journal->GetNumLogsAdded(), 3);
    // TODO (cheolho.kang): Fix wait issue
    WaitForAllCheckpointDone();
}

TEST_F(RocksDBLogWriteIntegrationTest, WriteLogs)
{
    POS_TRACE_DEBUG(9999, "RocksDBLogWriteIntegrationTest::WriteLogs");

    builder.SetJournalEnable(true)
        ->SetLogBufferSize(64 * 1024);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    writeTester->WriteLogsWithSize(logGroupSize);
    writeTester->WaitForAllLogWriteDone();
    writeTester->CompareLogs();
}

TEST_F(RocksDBLogWriteIntegrationTest, DISABLED_PendLogWrite)
{
    POS_TRACE_DEBUG(9999, "RocksDBLogWriteIntegrationTest::PendLogWrite");

    builder.SetJournalEnable(true)
        ->SetLogBufferSize(64 * 1024);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    writeTester->WriteLogsWithSize(logBufferSize + logGroupSize);
    EXPECT_TRUE(writeTester->AreAllLogWritesDone() == false);

    ExpectCheckpointTriggered();
    journal->StartCheckpoint();

    writeTester->WaitForAllLogWriteDone();
    EXPECT_TRUE(writeTester->AreAllLogWritesDone() == true);
}
} // namespace pos
