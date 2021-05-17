#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/allocator_ctx/segment_lock.h"

namespace pos
{
class MockSegmentLock : public SegmentLock
{
public:
    using SegmentLock::SegmentLock;
    MOCK_METHOD(std::mutex&, GetLock, (), (override));
};

} // namespace pos
