#include "src/allocator/context_manager/segment_ctx/segment_lock.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(SegmentLock, SegmentLock_)
{
}

TEST(SegmentLock, GetLock_TestSimpleGetter)
{
    // given
    SegmentLock segLock;
    // when
    std::mutex& m = segLock.GetLock();
}

} // namespace pos
