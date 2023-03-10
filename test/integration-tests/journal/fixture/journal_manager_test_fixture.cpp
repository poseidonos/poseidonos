#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

#include <experimental/filesystem>
#include <string>
#include <unistd.h>

#include "src/journal_manager/log_buffer/reset_log_group.h"
#include "src/metadata/segment_context_updater.h"
#include "test/integration-tests/journal/fake/i_context_manager_fake.h"
#include "test/integration-tests/journal/fake/segment_ctx_fake.h"
#include "test/integration-tests/journal/fault_event.h"
#include "test/unit-tests/volume/i_volume_info_manager_mock.h"

using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;

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
    testAllocator = new StrictMock<AllocatorMock>(testInfo, arrayInfo);
    volumeManager = new NiceMock<MockIVolumeInfoManager>();
    journal = new JournalManagerSpy(telemetryPublisher, arrayInfo, stateSub, logFileName);

    segmentContextUpdater = new SegmentContextUpdater(testAllocator->GetSegmentCtxFake(), journal->GetVersionedSegmentContext(), arrayInfo->GetSizeInfo(PartitionType::USER_DATA));
    writeTester = new LogWriteTestFixture(testMapper, testAllocator, arrayInfo, journal, testInfo, segmentContextUpdater);
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
    // Should reset configuration first
    journal->ResetJournalConfiguration(config);

    journal->ResetVersionedSegmentContext();
    journal->DeleteLogBuffer();

    if (journal->IsEnabled() == true)
    {
        journal->InitializeForTest(telemetryClient, testMapper, testAllocator, volumeManager);
    }

    _GetLogBufferSizeInfo();

    segmentContextUpdater->SetVersionedSegmentContext(journal->GetVersionedSegmentContext());
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
    SimulateSPORWithoutRecovery(configurationBuilder);
}

void
JournalManagerTestFixture::SimulateSPORWithoutRecovery(JournalConfigurationBuilder& configurationBuilder)
{
    delete journal;
    delete testMapper;

    testMapper = new StrictMock<MockMapper>(testInfo, arrayInfo, nullptr);
    writeTester->UpdateMapper(testMapper);
    replayTester->UpdateMapper(testMapper);

    telemetryPublisher = new NiceMock<MockTelemetryPublisher>;
    journal = new JournalManagerSpy(telemetryPublisher, arrayInfo, stateSub, GetLogFileName());
    journal->ResetJournalConfiguration(configurationBuilder.Build());
    writeTester->UpdateJournal(journal);

    testAllocator->GetSegmentCtxFake()->LoadContext();
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
    delete testMapper;
    testMapper = new StrictMock<MockMapper>(testInfo, arrayInfo, nullptr);

    std::experimental::filesystem::remove_all(targetDirName);

    // Use copied SPOR Directory
    telemetryPublisher = new NiceMock<MockTelemetryPublisher>;
    journal = new JournalManagerSpy(telemetryPublisher, arrayInfo, stateSub, SPORDirName);
    journal->ResetJournalConfiguration(configurationBuilder.Build());
    writeTester->UpdateJournal(journal);

    testAllocator->GetSegmentCtxFake()->LoadContext();
    journal->InitializeForTest(telemetryClient, testMapper, testAllocator, volumeManager);
}

void
JournalManagerTestFixture::InjectCheckpointFaultAfterMetaFlushCompleted(void)
{
    uint64_t latestContextVersion = 1;

    bool checkpoitnStopped = false;
    EventSmartPtr faultEvent(new FaultEvent(checkpoitnStopped));
    journal->InjectFaultEvent(typeid(ResetLogGroup), faultEvent);

    writeTester->WaitForAllLogWriteDone();
    ExpectCheckpointTriggered();
    journal->StartCheckpoint();
    while (checkpoitnStopped != true)
    {
        usleep(1);
    }
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
    EXPECT_CALL(*(testAllocator->GetIContextManagerFake()),
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
    VirtualBlks blks{
        .startVsa = {
            .stripeId = std::rand() % testInfo->numUserStripes,
            .offset = 0
        },
        .numBlks = 1
    };

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
