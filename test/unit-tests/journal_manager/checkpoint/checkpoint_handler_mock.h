#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/checkpoint/checkpoint_handler.h"

namespace pos
{
class MockCheckpointHandler : public CheckpointHandler
{
public:
    using CheckpointHandler::CheckpointHandler;
    MOCK_METHOD(void, Init, (IVersionedSegmentContext* versionedSegCtx, IMapFlush * mapFlush, IContextManager* contextManager, EventScheduler* scheduler), (override));
    MOCK_METHOD(int, Start, (MapList pendingDirtyMaps, EventSmartPtr callback, int logGroupIdInProgress), (override));
    MOCK_METHOD(int, FlushCompleted, (int metaId), (override));
    MOCK_METHOD(CheckpointStatus, GetStatus, (), (override));
};

} // namespace pos
