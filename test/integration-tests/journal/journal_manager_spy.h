#pragma once

#include <string>

#include "src/journal_manager/checkpoint/log_group_releaser.h"
#include "src/journal_manager/config/journal_configuration.h"
#include "src/journal_manager/journal_manager.h"
#include "src/journal_manager/log/log_list.h"
#include "test/integration-tests/journal/fake/allocator_mock.h"
#include "test/integration-tests/journal/fake/mapper_mock.h"
#include "test/integration-tests/journal/log_group_releaser_spy.h"

namespace pos
{
class MockEventScheduler;
class IJournalWriter;
class IVolumeEventHandler;
class IJournalStatusProvider;
class TelemetryPublisher;
class TelemetryClient;

class JournalManagerSpy : public JournalManager
{
public:
    JournalManagerSpy(TelemetryPublisher* tp, IArrayInfo* array, IStateControl* stateSub,
        std::string logFileName);
    virtual ~JournalManagerSpy(void);

    int InitializeForTest(TelemetryClient* telemetryClient, Mapper* mapper, Allocator* allocator,
        IVolumeInfoManager* volumeManager);
    int DoRecoveryForTest(void);
    void DeleteLogBuffer(void);
    void ResetJournalConfiguration(JournalConfiguration* journalConfig);
    void StartCheckpoint(void);
    void SetTriggerCheckpoint(bool val);
    bool IsCheckpointEnabled(void);
    uint64_t GetLogBufferSize(void);
    uint64_t GetLogGroupSize(void);
    int GetNumLogGroups(void);
    uint64_t GetMetaPageSize(void);
    bool IsCheckpointCompleted(void);
    int GetNumDirtyMap(int logGroupId);
    int GetLogs(LogList& logList);
    uint64_t GetNumLogsAdded(void);
    int VolumeDeleted(int volId);
    uint64_t GetNextOffset(void);
    IJournalWriter* GetJournalWriter(void);
    IJournalStatusProvider* GetStatusProvider(void);

private:
    int _GetLogsFromBuffer(LogList& logList);

    MockEventScheduler* eventScheduler;
    std::string LogFileName;
};
} // namespace pos
