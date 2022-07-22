#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/status/i_checkpoint_status.h"

namespace pos
{
class MockICheckpointStatus : public ICheckpointStatus
{
public:
    using ICheckpointStatus::ICheckpointStatus;
    MOCK_METHOD(int, GetFlushingLogGroupId, (), (override));
    MOCK_METHOD((std::list<int>), GetFullLogGroups, (), (override));
    MOCK_METHOD(CheckpointStatus, GetStatus, (), (override));
};

} // namespace pos
