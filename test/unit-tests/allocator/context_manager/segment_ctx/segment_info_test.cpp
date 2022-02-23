#include "src/allocator/context_manager/segment_ctx/segment_info.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(SegmentInfo, SegmentInfo_Constructor)
{
    SegmentInfo segInfo();
}

TEST(SegmentInfo, GetValidBlockCount_TestSimpleGetter)
{
    // given
    SegmentInfo segInfos;
    segInfos.SetValidBlockCount(5);
    // when
    int ret = segInfos.GetValidBlockCount();
    // then
    EXPECT_EQ(5, ret);
}

TEST(SegmentInfo, SetValidBlockCount_TestSimpleSetter)
{
    // given
    SegmentInfo segInfos;
    // when
    segInfos.SetValidBlockCount(5);
    // then
    int ret = segInfos.GetValidBlockCount();
    EXPECT_EQ(5, ret);
}

TEST(SegmentInfo, IncreaseValidBlockCount_TestIncreaseValue)
{
    // given
    SegmentInfo segInfos;
    // when
    int ret = segInfos.IncreaseValidBlockCount(5);
    // then
    EXPECT_EQ(5, ret);
}

TEST(SegmentInfo, DecreaseValidBlockCount_TestDecreaseValue)
{
    // given
    SegmentInfo segInfos;
    segInfos.IncreaseValidBlockCount(5);
    // when
    int ret = segInfos.DecreaseValidBlockCount(3);
    // then
    EXPECT_EQ(2, ret);
}

TEST(SegmentInfo, SetOccupiedStripeCount_TestSimpleSetter)
{
    // given
    SegmentInfo segInfos;
    // when
    segInfos.SetOccupiedStripeCount(3);
    // then
    int ret = segInfos.GetOccupiedStripeCount();
    EXPECT_EQ(3, ret);
}

TEST(SegmentInfo, GetOccupiedStripeCount_TestSimpleGetter)
{
    // given
    SegmentInfo segInfos;
    // when
    segInfos.SetOccupiedStripeCount(7);
    // then
    int ret = segInfos.GetOccupiedStripeCount();
    EXPECT_EQ(7, ret);
}

TEST(SegmentInfo, IncreaseOccupiedStripeCount_TestIncreaseValue)
{
    // given
    SegmentInfo segInfos;
    // when
    segInfos.SetOccupiedStripeCount(7);
    // then
    int ret = segInfos.IncreaseOccupiedStripeCount();
    EXPECT_EQ(8, ret);
}

} // namespace pos
