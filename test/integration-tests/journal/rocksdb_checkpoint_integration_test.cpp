#include <experimental/filesystem>
#include <iostream>

#include "gtest/gtest.h"
#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

using ::testing::_;
using ::testing::AtLeast;

namespace pos
{
class RocksDBCheckpointIntegrationTest : public JournalManagerTestFixture, public ::testing::Test
{
public:
    RocksDBCheckpointIntegrationTest(void);
    virtual ~RocksDBCheckpointIntegrationTest(void) = default;

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);
    JournalConfigurationBuilder builder;
};

RocksDBCheckpointIntegrationTest::RocksDBCheckpointIntegrationTest(void)
: JournalManagerTestFixture(GetLogDirName()),
  builder(testInfo)
{
}

void
RocksDBCheckpointIntegrationTest::SetUp(void)
{
    std::string targetDirName = rocksdbPath + "/" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(targetDirName);
    builder.SetRocksDBEnable(true);
    builder.SetRocksDBBasePath(rocksdbPath);
}

void
RocksDBCheckpointIntegrationTest::TearDown(void)
{
}

TEST_F(RocksDBCheckpointIntegrationTest, TriggerCheckpoint)
{
    POS_TRACE_DEBUG(9999, "RocksDBCheckpointIntegrationTest::TriggerCheckpoint");

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

TEST_F(RocksDBCheckpointIntegrationTest, WriteLogsToTriggerCheckpoint)
{
    POS_TRACE_DEBUG(9999, "RocksDBCheckpointIntegrationTest::WriteLogsToTriggerCheckpoint");

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
