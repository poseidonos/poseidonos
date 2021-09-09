#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_block_map_update.h"

namespace pos
{
class MockReplayBlockMapUpdate : public ReplayBlockMapUpdate
{
public:
    using ReplayBlockMapUpdate::ReplayBlockMapUpdate;
    MOCK_METHOD(int, Replay, (), (override));
};

} // namespace pos
