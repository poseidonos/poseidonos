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
    MOCK_METHOD(int, FlushCompleted, (int metaId), (override));
};

} // namespace pos
