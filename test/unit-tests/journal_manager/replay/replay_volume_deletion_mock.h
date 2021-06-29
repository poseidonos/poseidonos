#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/replay/replay_volume_deletion.h"

namespace pos
{
class MockReplayVolumeDeletion : public ReplayVolumeDeletion
{
public:
    using ReplayVolumeDeletion::ReplayVolumeDeletion;
    MOCK_METHOD(int, Start, (), (override));
    MOCK_METHOD(ReplayTaskId, GetId, (), (override));
    MOCK_METHOD(int, GetWeight, (), (override));
    MOCK_METHOD(int, GetNumSubTasks, (), (override));
};

} // namespace pos
