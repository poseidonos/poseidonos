#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/load_replayed_segment_context.h"

namespace pos
{
class MockLoadReplayedSegmentContext : public LoadReplayedSegmentContext
{
public:
    using LoadReplayedSegmentContext::LoadReplayedSegmentContext;
    MOCK_METHOD(int, Start, (), (override));
    MOCK_METHOD(ReplayTaskId, GetId, (), (override));
    MOCK_METHOD(int, GetWeight, (), (override));
    MOCK_METHOD(int, GetNumSubTasks, (), (override));
};

} // namespace pos
