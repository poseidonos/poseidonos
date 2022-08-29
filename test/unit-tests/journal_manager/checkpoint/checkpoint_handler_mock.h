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
    MOCK_METHOD(void, Init, (IMapFlush * mapFlush, IContextManager* contextManager, EventScheduler* scheduler), (override));
    MOCK_METHOD(int, Start, (MapList pendingDirtyMaps, EventSmartPtr callback), (override));
    MOCK_METHOD(int, FlushCompleted, (int metaId, int logGroupId), (override));
    MOCK_METHOD(CheckpointStatus, GetStatus, (), (override));
};

} // namespace pos
