#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/checkpoint/checkpoint_manager.h"

namespace pos
{
class MockCheckpointManager : public CheckpointManager
{
public:
    using CheckpointManager::CheckpointManager;
    MOCK_METHOD(void, Init, (IMapFlush* mapFlush, IContextManager* ctxManager, EventScheduler* scheduler,
        DirtyMapManager* dMapManager, TelemetryPublisher* tp), (override));
    MOCK_METHOD(int, RequestCheckpoint, (int logGroupId, EventSmartPtr callback), (override));
    MOCK_METHOD(int, StartCheckpoint, (EventSmartPtr callback), (override));
    MOCK_METHOD(CheckpointStatus, GetStatus, (), (override));
    MOCK_METHOD(void, CheckpointCompleted, (), (override));
    MOCK_METHOD(void, BlockCheckpointAndWaitToBeIdle, (), (override));
    MOCK_METHOD(void, UnblockCheckpoint, (), (override));
};

} // namespace pos
