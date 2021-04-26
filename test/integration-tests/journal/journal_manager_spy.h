#pragma once

#include <string>

#include "src/journal_manager/config/journal_configuration.h"
#include "src/journal_manager/log/log_list.h"
#include "src/journal_manager/checkpoint/log_group_releaser.h"
#include "src/journal_manager/journal_manager.h"

#include "test/integration-tests/journal/fake/mapper_mock.h"
#include "test/integration-tests/journal/fake/allocator_mock.h"

namespace pos
{
class JournalManagerSpy : public JournalManager
{
public:
    JournalManagerSpy(IArrayInfo* array, IStateControl* stateSub, std::string logFileName);
    virtual ~JournalManagerSpy(void);

    int InitializeForTest(Mapper* mapper, Allocator* allocator);
    int DoRecoveryForTest(void);

    void DeleteLogBuffer(void);

    void ResetJournalConfiguration(JournalConfiguration* journalConfig);
    void StartCheckpoint(void);
    void SetTriggerCheckpoint(bool val);
    bool IsCheckpointEnabled(void);

    uint64_t GetLogBufferSize(void);
    uint64_t GetLogGroupSize(void);

    int GetNumFullLogGroups(void);
    int GetNumDirtyMap(int logGroupId);

    int GetLogs(LogList& logList);
    uint64_t GetNumLogsAdded(void);

    int VolumeDeleted(int volId);

    uint64_t GetNextOffset(void);

private:
    int _GetLogsFromBuffer(LogList& logList);
};

class LogGroupReleaserTester : public LogGroupReleaser
{
public:
    void UpdateFlushingLogGroup(void);

    bool triggerCheckpoint = true;

protected:
    virtual void _FlushNextLogGroup(void) override;
};
} // namespace pos
