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
};

} // namespace pos
