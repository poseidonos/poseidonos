#include "src/allocator/context_manager/segment_ctx/segment_states.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(SegmentStates, SegmentStates_)
{
}

TEST(SegmentStates, SetSegmentId_TestSimpleSetter)
{
    // given
    SegmentStates segState;
    // when
    segState.SetSegmentId(10);
}

TEST(SegmentStates, GetState_TestSimpleGetter)
{
    // given
    SegmentStates segState;
    segState.SetState(SegmentState::FREE);
    // when
    SegmentState ret = segState.GetState();
    // then
    EXPECT_EQ(SegmentState::FREE, ret);
}

TEST(SegmentStates, SetState_TestSimpleSetter)
{
    // given
    SegmentStates segState;
    // when
    segState.SetState(SegmentState::SSD);
    // then
    SegmentState ret = segState.GetState();
    EXPECT_EQ(SegmentState::SSD, ret);
}

} // namespace pos
