#pragma once

#include <string>

#include "test/integration-tests/journal/fake/allocator_mock.h"
#include "test/integration-tests/journal/fake/array_info_mock.h"
#include "test/integration-tests/journal/fake/mapper_mock.h"
#include "test/integration-tests/journal/fake/state_subscription_mock.h"
#include "test/integration-tests/journal/fixture/log_write_test_fixture.h"
#include "test/integration-tests/journal/fixture/replay_test_fixture.h"
#include "test/integration-tests/journal/fixture/stripe_test_fixture.h"
#include "test/integration-tests/journal/journal_manager_spy.h"
#include "test/integration-tests/journal/utils/journal_configuration_builder.h"
#include "test/integration-tests/journal/utils/test_info.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_client_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

namespace pos
{
class JournalManagerTestFixture
{
public:
    JournalManagerTestFixture(std::string logFileName);
    virtual ~JournalManagerTestFixture(void);

    void InitializeJournal(void);
    void InitializeJournal(JournalConfigurationSpy* config);
    void SimulateSPORWithoutRecovery(void);
    void SimulateSPORWithoutRecovery(JournalConfigurationSpy* config);
    void SimulateRocksDBSPORWithoutRecovery(void);
    void SetTriggerCheckpoint(bool isCheckpointEnabled);
    void ExpectCheckpointTriggered(void);
    void WaitForAllCheckpointDone(void);
    JournalManagerSpy* GetJournal(void);
    int AddDummyLog(void);

protected:
    JournalManagerSpy* journal;
    MockMapper* testMapper;
    AllocatorMock* testAllocator;
    ArrayInfoMock* arrayInfo;
    StateSubscriptionMock* stateSub;
    IVolumeInfoManager* volumeManager;
    MockTelemetryPublisher* telemetryPublisher;
    MockTelemetryClient* telemetryClient;
    LogWriteTestFixture* writeTester;
    ReplayTestFixture* replayTester;
    TestInfo* testInfo;

    const uint64_t INVALID_BUFFER_SIZE = UINT64_MAX;

    uint64_t logBufferSize;
    uint64_t logGroupSize;
    int numLogGroups;
    const std::string rocksdbPath = "/etc/pos/POSRaid";

private:
    void _GetLogBufferSizeInfo(void);
};
} // namespace pos
