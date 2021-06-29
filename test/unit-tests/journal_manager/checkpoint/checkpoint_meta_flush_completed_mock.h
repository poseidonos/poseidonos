#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/checkpoint/checkpoint_meta_flush_completed.h"

namespace pos
{
class MockCheckpointMetaFlushCompleted : public CheckpointMetaFlushCompleted
{
public:
    using CheckpointMetaFlushCompleted::CheckpointMetaFlushCompleted;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos
