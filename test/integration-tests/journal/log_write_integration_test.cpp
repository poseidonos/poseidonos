#include "gtest/gtest.h"
#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

using ::testing::AtLeast;

namespace pos
{
class LogWriteIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    LogWriteIntegrationTest(void);
    virtual ~LogWriteIntegrationTest(void) = default;

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);
};

LogWriteIntegrationTest::LogWriteIntegrationTest(void)
: JournalManagerTestFixture(GetLogFileName())
{
}

void
LogWriteIntegrationTest::SetUp(void)
{
}

void
LogWriteIntegrationTest::TearDown(void)
{
}

TEST_F(LogWriteIntegrationTest, WriteLogWithJournalDisabled)
{
    POS_TRACE_DEBUG(9999, "LogWriteIntegrationTest::WriteLogWithJournalDisabled");

    JournalConfigurationBuilder builder(testInfo);
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

TEST_F(LogWriteIntegrationTest, WriteLog_UserStripe)
{
    POS_TRACE_DEBUG(9999, "LogWriteIntegrationTest::WriteLog_UserStripe");
    InitializeJournal();

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

TEST_F(LogWriteIntegrationTest, WriteLog_GcStripe)
{
    POS_TRACE_DEBUG(9999, "LogWriteIntegrationTest::WriteLog_GcStripe");
    InitializeJournal();

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

TEST_F(LogWriteIntegrationTest, DISABLED_WriteLog_GcStripes)
{
    POS_TRACE_DEBUG(9999, "LogWriteIntegrationTest::WriteLog_GcStripe");
    JournalConfigurationBuilder builder(testInfo);
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

TEST_F(LogWriteIntegrationTest, WriteLogs)
{
    POS_TRACE_DEBUG(9999, "LogWriteIntegrationTest::WriteLogs");

    JournalConfigurationBuilder builder(testInfo);
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(64 * 1024);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    writeTester->WriteLogsWithSize(logGroupSize);
    writeTester->WaitForAllLogWriteDone();
    writeTester->CompareLogs();
}

TEST_F(LogWriteIntegrationTest, DISABLED_PendLogWrite)
{
    POS_TRACE_DEBUG(9999, "LogWriteIntegrationTest::PendLogWrite");

    JournalConfigurationBuilder builder(testInfo);
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
