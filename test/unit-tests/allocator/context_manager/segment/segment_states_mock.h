#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/segment/segment_states.h"

namespace pos
{
class MockSegmentStates : public SegmentStates
{
public:
    using SegmentStates::SegmentStates;
};

} // namespace pos
