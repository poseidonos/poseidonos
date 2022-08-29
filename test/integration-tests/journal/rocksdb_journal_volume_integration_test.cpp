#include <experimental/filesystem>

#include "rocksdb_journal_volume_integration_test.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::Return;

namespace pos
{
RocksDBJournalVolumeIntegrationTest::RocksDBJournalVolumeIntegrationTest(void)
: JournalManagerTestFixture(GetLogDirName()),
builder(testInfo)
{
}

void
RocksDBJournalVolumeIntegrationTest::SetUp(void)
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
RocksDBJournalVolumeIntegrationTest::TearDown(void)
{
    // Teardown : remove rocksdb log files by removing temporary directory.
    std::string targetDirName = rocksdbPath + "/" + GetLogDirName() + "_RocksJournal";
    int ret = std::experimental::filesystem::remove_all(targetDirName);

    // Remove SPOR directory
    std::string SPORDirectory = rocksdbPath + "/SPOR" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(SPORDirectory);
}

// Write one stripe per volume and return stripe list indexed by volume id
StripeList
RocksDBJournalVolumeIntegrationTest::WriteStripes(int numVolumes)
{
    StripeList writtenStripes(numVolumes);
    StripeId currentVsid = std::rand() % testInfo->numUserStripes;

    for (int vol = 0; vol < numVolumes; vol++)
    {
        StripeTestFixture stripe(currentVsid++, vol);
        writeTester->GenerateLogsForStripe(stripe, 0, testInfo->numBlksPerStripe);
        writeTester->WriteLogsForStripe(stripe);
        writtenStripes[vol] = stripe;
    }

    writeTester->WaitForAllLogWriteDone();

    return writtenStripes;
}

void
RocksDBJournalVolumeIntegrationTest::DeleteVolumes(Volumes& volumesToDelete)
{
    EXPECT_CALL(*testMapper, FlushDirtyMpages).Times(AtLeast(1));
    EXPECT_CALL(*(testAllocator->GetIContextManagerMock()), FlushContexts(_, false, _)).Times(volumesToDelete.size());
    for (auto volId : volumesToDelete)
    {
        EXPECT_TRUE(journal->VolumeDeleted(volId) == 0);
    }

    writeTester->WaitForAllLogWriteDone();
}

void
RocksDBJournalVolumeIntegrationTest::CheckVolumeDeleteLogsWritten(Volumes& volumesToDelete)
{
    LogList logList;
    EXPECT_TRUE(journal->GetLogs(logList) == 0);

    std::list<LogHandlerInterface*> logs = logList.GetLogs();

    int deleteVolumeLogFound = 0;
    
    while (logs.size() != 0)
    {
        LogHandlerInterface* log = logs.front();

        if (log->GetType() == LogType::VOLUME_DELETED)
        {
            deleteVolumeLogFound++;

            VolumeDeletedLog* logData = reinterpret_cast<VolumeDeletedLog*>(log->GetData());
            EXPECT_TRUE(volumesToDelete.find(logData->volId) != volumesToDelete.end());
        }

        logs.pop_front();
    }
    EXPECT_TRUE(deleteVolumeLogFound == static_cast<int>(volumesToDelete.size()));
}

void
RocksDBJournalVolumeIntegrationTest::ExpectReplayStripes(StripeList& writtenStripes,
    int numVolumes, Volumes& deletedVolumes)
{
    replayTester->ExpectReturningUnmapStripes();

    for (int volId = 0; volId < numVolumes; volId++)
    {
        auto stripe = writtenStripes[volId];

        replayTester->ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
        replayTester->ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);

        if (deletedVolumes.find(volId) == deletedVolumes.end())
        {
            InSequence s;
            replayTester->ExpectReplayBlockLogsForStripe(volId, stripe.GetBlockMapList());
        }
        replayTester->ExpectReplayStripeFlush(stripe);
    }
}

void
RocksDBJournalVolumeIntegrationTest::ExpectReplayTail(int numVolumesWritten)
{
    for (int volId = 0; volId < numVolumesWritten; volId++)
    {
        EXPECT_CALL(*(testAllocator->GetIContextReplayerMock()),
            ResetActiveStripeTail(volId))
            .Times(1);
    }
    EXPECT_CALL(*(testAllocator->GetIContextReplayerMock()), ReplaySsdLsid).Times(1);
}

TEST_F(RocksDBJournalVolumeIntegrationTest, DisableJournalAndNotifyVolumeDeleted)
{
    POS_TRACE_DEBUG(9999, "RocksDBJournalVolumeIntegrationTest::DisableJournalAndNotifyVolumeDeleted");

    builder.SetJournalEnable(false);

    InitializeJournal(builder.Build());

    EXPECT_TRUE(journal->VolumeDeleted(testInfo->defaultTestVol) == 0);
}

TEST_F(RocksDBJournalVolumeIntegrationTest, WriteLogsAndDeleteVolume)
{
    POS_TRACE_DEBUG(9999, "RocksDBJournalVolumeIntegrationTest::WriteLogsAndDeleteVolume");

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    int numVolumes = testInfo->maxNumVolume;
    StripeList writtenStripes = WriteStripes(numVolumes);

    Volumes volumesToDelete = {numVolumes / 2};

    DeleteVolumes(volumesToDelete);
    CheckVolumeDeleteLogsWritten(volumesToDelete);

    SimulateRocksDBSPORWithoutRecovery();
    ExpectReplayStripes(writtenStripes, numVolumes, volumesToDelete);
    ExpectReplayTail(numVolumes);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBJournalVolumeIntegrationTest, WriteLogsAndDeleteVolumes)
{
    POS_TRACE_DEBUG(9999, "RocksDBJournalVolumeIntegrationTest::WriteLogsAndDeleteVolumes");

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    int numVolumes = testInfo->maxNumVolume;
    StripeList writtenStripes = WriteStripes(numVolumes);

    Volumes volumesToDelete = {1, 5};

    DeleteVolumes(volumesToDelete);
    CheckVolumeDeleteLogsWritten(volumesToDelete);

    SimulateRocksDBSPORWithoutRecovery();
    ExpectReplayStripes(writtenStripes, numVolumes, volumesToDelete);
    ExpectReplayTail(numVolumes);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

TEST_F(RocksDBJournalVolumeIntegrationTest, DeleteAndCreateVolume)
{
    POS_TRACE_DEBUG(9999, "RocksDBJournalVolumeIntegrationTest::DeleteAndCreateVolume");

    InitializeJournal(builder.Build());
    SetTriggerCheckpoint(false);

    int numVolumes = 3;
    StripeList writtenStripes = WriteStripes(numVolumes);

    Volumes volumesToDelete = {1};
    DeleteVolumes(volumesToDelete);

    // Assume deleted new volume is created with volume id 1

    StripeList writtenStripesAfter = WriteStripes(numVolumes);

    writeTester->WaitForAllLogWriteDone();
    SimulateRocksDBSPORWithoutRecovery();

    ExpectReplayStripes(writtenStripes, numVolumes, volumesToDelete);

    Volumes emptyList = {};
    ExpectReplayStripes(writtenStripesAfter, numVolumes, emptyList);

    ExpectReplayTail(numVolumes);

    EXPECT_TRUE(journal->DoRecoveryForTest() == 0);
}

} // namespace pos
