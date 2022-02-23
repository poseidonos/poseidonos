#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/segment_ctx/segment_lock.h"

namespace pos
{
class MockSegmentLock : public SegmentLock
{
public:
    using SegmentLock::SegmentLock;
    MOCK_METHOD(std::mutex&, GetLock, (), (override));
};

} // namespace pos
