#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/checkpoint/log_group_releaser.h"

namespace pos
{
class MockLogGroupReleaser : public LogGroupReleaser
{
public:
    using LogGroupReleaser::LogGroupReleaser;
    MOCK_METHOD(void, Init, (LogBufferWriteDoneNotifier* notified, JournalLogBuffer* logBuffer,
        DirtyMapManager* dirtyPage, CallbackSequenceController* sequencer, IMapFlush* mapFlush,
        IContextManager* contextManager), (override));
    MOCK_METHOD(void, AddToFullLogGroup, (int groupId), (override));
    MOCK_METHOD(int, StartCheckpoint, (), (override));
    MOCK_METHOD(void, CheckpointCompleted, (), (override));
    MOCK_METHOD(int, GetFlushingLogGroupId, (), (override));
    MOCK_METHOD(std::list<int>, GetFullLogGroups, (), (override));
    MOCK_METHOD(CheckpointStatus, GetStatus, (), (override));
    MOCK_METHOD(void, LogGroupResetCompleted, (int logGroupId), (override));
    MOCK_METHOD(void, _FlushNextLogGroup, (), (override));
};

} // namespace pos
