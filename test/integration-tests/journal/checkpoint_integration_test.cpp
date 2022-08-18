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
    MapList dirtyMaps = writeTester->GetDirtyMap();

    // This is dummy writes
    writeTester->WriteLogsWithSize(logGroupSize / 2);
    writeTester->WaitForAllLogWriteDone();

    EXPECT_TRUE(journal->GetNumDirtyMap(0) == static_cast<int>(dirtyMaps.size()));
    for (auto mapId : dirtyMaps)
    {
        EXPECT_CALL(*testMapper,
            FlushDirtyMpages(mapId, _))
            .Times(1);
    }
    EXPECT_CALL(*(testAllocator->GetIContextManagerMock()), FlushContexts).Times(1);

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
