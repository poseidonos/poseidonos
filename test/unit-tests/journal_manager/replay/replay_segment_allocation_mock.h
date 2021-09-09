#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_segment_allocation.h"

namespace pos
{
class MockReplaySegmentAllocation : public ReplaySegmentAllocation
{
public:
    using ReplaySegmentAllocation::ReplaySegmentAllocation;
    MOCK_METHOD(int, Replay, (), (override));
};

} // namespace pos
