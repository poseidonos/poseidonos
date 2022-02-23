#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/checkpoint/checkpoint_handler.h"

namespace pos
{
class MockCheckpointHandler : public CheckpointHandler
{
public:
    using CheckpointHandler::CheckpointHandler;
    MOCK_METHOD(void, Init, (IMapFlush* mapFlush, IContextManager* contextManer, EventScheduler* scheduler), (override));
    MOCK_METHOD(int, Start, (MapPageList pendingDirtyPages, EventSmartPtr callback), (override));
    MOCK_METHOD(int, FlushCompleted, (int metaId), (override));
    MOCK_METHOD(CheckpointStatus, GetStatus, (), (override));
};

} // namespace pos
