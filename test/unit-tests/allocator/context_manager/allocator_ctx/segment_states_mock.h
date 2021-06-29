#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/allocator_ctx/segment_states.h"

namespace pos
{
class MockSegmentStates : public SegmentStates
{
public:
    using SegmentStates::SegmentStates;
    MOCK_METHOD(SegmentState, GetState, (), (override));
    MOCK_METHOD(void, SetState, (SegmentState newState), (override));
};

} // namespace pos
