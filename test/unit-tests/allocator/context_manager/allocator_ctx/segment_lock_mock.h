#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/segment_lock.h"

namespace pos
{
class MockSegmentLock : public SegmentLock
{
public:
    using SegmentLock::SegmentLock;
};

} // namespace pos
