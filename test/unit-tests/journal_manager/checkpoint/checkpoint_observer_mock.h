#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/checkpoint/checkpoint_observer.h"

namespace pos
{
class MockCheckpointObserver : public CheckpointObserver
{
public:
    using CheckpointObserver::CheckpointObserver;
    MOCK_METHOD(void, CheckpointCompleted, (), (override));
};

} // namespace pos
