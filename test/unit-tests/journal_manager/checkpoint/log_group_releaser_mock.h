#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/checkpoint/log_group_releaser.h"

namespace pos
{
class MockLogGroupReleaser : public LogGroupReleaser
{
public:
    using LogGroupReleaser::LogGroupReleaser;
    MOCK_METHOD(void, Init, (JournalConfiguration* config, LogBufferWriteDoneNotifier* notified, IJournalLogBuffer* logBuffer, CheckpointManager* cpManager, IMapFlush* mapFlush, IContextManager* contextManager, EventScheduler* scheduler), (override));
    MOCK_METHOD(void, AddToFullLogGroup, (int groupId, uint32_t sequenceNumber), (override));
    MOCK_METHOD(int, GetFlushingLogGroupId, (), (override));
    MOCK_METHOD((std::list<LogGroupInfo>), GetFullLogGroups, (), (override));
    MOCK_METHOD(CheckpointStatus, GetStatus, (), (override));
    MOCK_METHOD(void, LogGroupResetCompleted, (int logGroupId), (override));
    MOCK_METHOD(void, _FlushNextLogGroup, (), (override));
    MOCK_METHOD(void, _TriggerCheckpoint, (), (override));
};

} // namespace pos
