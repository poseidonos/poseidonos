#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

#include <experimental/filesystem>
#include <string>

#include "test/unit-tests/volume/i_volume_info_manager_mock.h"

using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::Return;

namespace pos
{
JournalManagerTestFixture::JournalManagerTestFixture(std::string logFileName)
: logBufferSize(INVALID_BUFFER_SIZE),
  logGroupSize(INVALID_BUFFER_SIZE),
  numLogGroups(0)
{
    testInfo = new TestInfo();
    arrayInfo = new ArrayInfoMock(testInfo);
    stateSub = new StateSubscriptionMock();

    telemetryPublisher = new NiceMock<MockTelemetryPublisher>;
    telemetryClient = new NiceMock<MockTelemetryClient>;
    testMapper = new StrictMock<MockMapper>(testInfo, arrayInfo, nullptr);
    testAllocator = new StrictMock<AllocatorMock>(arrayInfo);
    volumeManager = new NiceMock<MockIVolumeInfoManager>();
    journal = new JournalManagerSpy(telemetryPublisher, arrayInfo, stateSub, logFileName);

    writeTester = new LogWriteTestFixture(testMapper, arrayInfo, journal, testInfo);
    replayTester = new ReplayTestFixture(testMapper, testAllocator, testInfo);
}

JournalManagerTestFixture::~JournalManagerTestFixture(void)
{
    if (journal->IsEnabled() == true && journal->IsCheckpointEnabled() == true)
    {
        WaitForAllCheckpointDone();
    }

    writeTester->WaitForAllLogWriteDone();
    writeTester->Reset();

    journal->DeleteLogBuffer();

    delete replayTester;
    delete writeTester;

    delete journal;
    delete volumeManager;
    delete testAllocator;
    delete testMapper;

    delete stateSub;
    delete arrayInfo;
    delete testInfo;
}

void
JournalManagerTestFixture::InitializeJournal(void)
{
    JournalConfigurationBuilder configurationBuilder(testInfo);
    InitializeJournal(configurationBuilder.Build());
}

void
JournalManagerTestFixture::InitializeJournal(JournalConfigurationSpy* config)
{
    journal->ResetJournalConfiguration(config);
    journal->DeleteLogBuffer();

    if (journal->IsEnabled() == true)
    {
        journal->InitializeForTest(telemetryClient, testMapper, testAllocator, volumeManager);
    }

    _GetLogBufferSizeInfo();
    writeTester->Reset();
}

void
JournalManagerTestFixture::_GetLogBufferSizeInfo(void)
{
    logBufferSize = journal->GetLogBufferSize();
    logGroupSize = journal->GetLogGroupSize();
    numLogGroups = journal->GetNumLogGroups();
}

void
JournalManagerTestFixture::SimulateSPORWithoutRecovery(void)
{
    JournalConfigurationBuilder configurationBuilder(testInfo);

    delete journal;

    telemetryPublisher = new NiceMock<MockTelemetryPublisher>;
    journal = new JournalManagerSpy(telemetryPublisher, arrayInfo, stateSub, GetLogFileName());
    journal->ResetJournalConfiguration(configurationBuilder.Build());
    writeTester->UpdateJournal(journal);

    journal->InitializeForTest(telemetryClient, testMapper, testAllocator, volumeManager);
}

void
JournalManagerTestFixture::SimulateRocksDBSPORWithoutRecovery(void)
{
    JournalConfigurationBuilder configurationBuilder(testInfo);
    configurationBuilder.SetRocksDBEnable(true);
    configurationBuilder.SetRocksDBBasePath(rocksdbPath);

    // To Simulate SPOR, copy rocksdb data to another directory at any time which is similar to closing rocksdb abrubtly before closing db.
    std::string SPORDirName = "SPOR" + GetLogDirName();
    std::string SPORDirectory = rocksdbPath + "/" + SPORDirName + "_RocksJournal";
    std::string targetDirName = rocksdbPath + "/" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::copy(targetDirName, SPORDirectory);

    // Remove Existing Target Directory
    delete journal;
    std::experimental::filesystem::remove_all(targetDirName);

    // Use copied SPOR Directory
    telemetryPublisher = new NiceMock<MockTelemetryPublisher>;
    journal = new JournalManagerSpy(telemetryPublisher, arrayInfo, stateSub, SPORDirName);
    journal->ResetJournalConfiguration(configurationBuilder.Build());
    writeTester->UpdateJournal(journal);

    journal->InitializeForTest(telemetryClient, testMapper, testAllocator, volumeManager);
}

void
JournalManagerTestFixture::SetTriggerCheckpoint(bool isCheckpointEnabled)
{
    journal->SetTriggerCheckpoint(isCheckpointEnabled);
}

void
JournalManagerTestFixture::ExpectCheckpointTriggered(void)
{
    EXPECT_CALL(*testMapper, FlushDirtyMpages).Times(AtLeast(1));
    EXPECT_CALL(*(testAllocator->GetIContextManagerMock()),
        FlushContexts)
        .Times(AtLeast(1));
}

void
JournalManagerTestFixture::WaitForAllCheckpointDone(void)
{
    while (journal->IsCheckpointCompleted() == false)
    {
        usleep(1);
    }
}

JournalManagerSpy*
JournalManagerTestFixture::GetJournal(void)
{
    return journal;
}

int
JournalManagerTestFixture::AddDummyLog(void)
{
    VirtualBlks blks;
    blks.startVsa = UNMAP_VSA;
    blks.numBlks = 1;

    bool result = writeTester->WriteBlockLog(testInfo->defaultTestVol,
        0, blks);

    if (result == true)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}
} // namespace pos
