#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

#include <string>

#include "test/unit-tests/volume/i_volume_manager_mock.h"

using ::testing::AtLeast;
using ::testing::NiceMock;
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
    testMapper = new StrictMock<MockMapper>(testInfo, arrayInfo, nullptr);

    testAllocator = new StrictMock<AllocatorMock>(arrayInfo);
    stateSub = new StateSubscriptionMock();
    volumeManager = new NiceMock<MockIVolumeManager>();
    journal = new JournalManagerSpy(arrayInfo, stateSub, logFileName);

    writeTester = new LogWriteTestFixture(testMapper, arrayInfo, journal, testInfo);
    replayTester = new ReplayTestFixture(testMapper, testAllocator, testInfo);
}

JournalManagerTestFixture::~JournalManagerTestFixture(void)
{
    if (journal->IsCheckpointEnabled() == true)
    {
        WaitForAllCheckpointDone();
    }

    writeTester->WaitForAllLogWriteDone();
    writeTester->Reset();

    journal->DeleteLogBuffer();

    delete volumeManager;
    delete replayTester;
    delete testInfo;
    delete journal;
    delete testMapper;
    delete testAllocator;
    delete arrayInfo;
    delete writeTester;
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
        journal->InitializeForTest(testMapper, testAllocator, volumeManager);
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

    journal = new JournalManagerSpy(arrayInfo, stateSub, GetLogFileName());
    journal->ResetJournalConfiguration(configurationBuilder.Build());
    writeTester->UpdateJournal(journal);

    journal->InitializeForTest(testMapper, testAllocator, volumeManager);
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
        FlushContextsAsync)
        .Times(AtLeast(1));
}

void
JournalManagerTestFixture::WaitForAllCheckpointDone(void)
{
    while (journal->GetNumFullLogGroups() != 0)
    {
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
