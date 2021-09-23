#include "gtest/gtest.h"
#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

using ::testing::_;
using ::testing::AtLeast;

namespace pos
{
class CheckpointIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    CheckpointIntegrationTest(void);
    virtual ~CheckpointIntegrationTest(void) = default;

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);
};

CheckpointIntegrationTest::CheckpointIntegrationTest(void)
: JournalManagerTestFixture(GetLogFileName())
{
}

void
CheckpointIntegrationTest::SetUp(void)
{
}

void
CheckpointIntegrationTest::TearDown(void)
{
}

TEST_F(CheckpointIntegrationTest, TriggerCheckpoint)
{
    POS_TRACE_DEBUG(9999, "CheckpointIntegrationTest::TriggerCheckpoint");

    JournalConfigurationBuilder builder(testInfo);
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(64 * 1024);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    writeTester->WriteLogsWithSize(logGroupSize);
    writeTester->WaitForAllLogWriteDone();
    MapPageList dirtyPages = writeTester->GetDirtyMap();

    // This is dummy writes
    writeTester->WriteLogsWithSize(logGroupSize / 2);
    writeTester->WaitForAllLogWriteDone();

    EXPECT_TRUE(journal->GetNumDirtyMap(0) == static_cast<int>(dirtyPages.size()));
    for (auto it = dirtyPages.begin(); it != dirtyPages.end(); it++)
    {
        EXPECT_CALL(*testMapper,
            FlushDirtyMpages(it->first, ::_, it->second)).Times(1);
    }
    EXPECT_CALL(*(testAllocator->GetIContextManagerMock()), FlushContexts(_, false)).Times(1);

    journal->StartCheckpoint();
    WaitForAllCheckpointDone();
}

TEST_F(CheckpointIntegrationTest, WriteLogsToTriggerCheckpoint)
{
    POS_TRACE_DEBUG(9999, "CheckpointIntegrationTest::WriteLogsToTriggerCheckpoint");

    JournalConfigurationBuilder builder(testInfo);
    builder.SetJournalEnable(true)
        ->SetLogBufferSize(64 * 1024);

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(true);

    ExpectCheckpointTriggered();

    writeTester->WriteLogsWithSize(logBufferSize + logGroupSize);
    writeTester->WaitForAllLogWriteDone();
    WaitForAllCheckpointDone();
}
} // namespace pos
