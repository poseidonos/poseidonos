#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/checkpoint/checkpoint_completion.h"

namespace pos
{
class MockCheckpointCompletion : public CheckpointCompletion
{
public:
    using CheckpointCompletion::CheckpointCompletion;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos
