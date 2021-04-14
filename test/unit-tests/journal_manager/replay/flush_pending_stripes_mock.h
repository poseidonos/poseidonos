#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/flush_pending_stripes.h"

namespace pos
{
class MockFlushPendingStripes : public FlushPendingStripes
{
public:
    using FlushPendingStripes::FlushPendingStripes;
    MOCK_METHOD(int, Start, (), (override));
    MOCK_METHOD(ReplayTaskId, GetId, (), (override));
    MOCK_METHOD(int, GetWeight, (), (override));
    MOCK_METHOD(int, GetNumSubTasks, (), (override));
};

} // namespace pos
