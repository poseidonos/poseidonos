#include "src/allocator/context_manager/segment_ctx/segment_info.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(SegmentInfo, SegmentInfo_Constructor)
{
    {
        SegmentInfo segInfo;
    }

    {
        SegmentInfo* segInfo = new SegmentInfo();
        delete segInfo;
    }
}

TEST(SegmentInfo, SetValidBlockCount_TestSimpleSetter)
{
    SegmentInfo segInfos;

    segInfos.SetValidBlockCount(5);
    EXPECT_EQ(segInfos.GetValidBlockCount(), 5);

    segInfos.SetValidBlockCount(3);
    EXPECT_EQ(segInfos.GetValidBlockCount(), 3);

    segInfos.SetValidBlockCount(10);
    EXPECT_EQ(segInfos.GetValidBlockCount(), 10);
}

TEST(SegmentInfo, IncreaseValidBlockCount_TestIncreaseValue)
{
    SegmentInfo segInfos;

    EXPECT_EQ(segInfos.IncreaseValidBlockCount(5), 5);
    EXPECT_EQ(segInfos.IncreaseValidBlockCount(3), 8);
    EXPECT_EQ(segInfos.IncreaseValidBlockCount(10), 18);
}

TEST(SegmentInfo, DecreaseValidBlockCount_testDecreaseToNonZero)
{
    // given
    SegmentInfo segInfos(10, 0, SegmentState::FREE);
    // when
    auto result = segInfos.DecreaseValidBlockCount(3);
    bool segmentFreed = result.first;
    // then
    EXPECT_EQ(segmentFreed, false);
}

TEST(SegmentInfo, DecreaseValidBlockCount_testDecreaseToZeroWhenSsdState)
{
    // given
    SegmentInfo segInfos(3, 10, SegmentState::SSD);
    // when
    auto result = segInfos.DecreaseValidBlockCount(3);
    bool segmentFreed = result.second;
    SegmentState prevState = result.second;

    // then
    EXPECT_EQ(segmentFreed, true);
    EXPECT_EQ(prevState, SegmentState::SSD);
    EXPECT_EQ(segInfos.GetState(), SegmentState::FREE);
    EXPECT_EQ(segInfos.GetValidBlockCount(), 0);
    EXPECT_EQ(segInfos.GetOccupiedStripeCount(), 0);
}

TEST(SegmentInfo, DecreaseValidBlockCount_testDecreaseToZeroWhenNvramState)
{
    // given
    SegmentInfo segInfos(3, 10, SegmentState::NVRAM);
    // when
    auto result = segInfos.DecreaseValidBlockCount(3);
    bool segmentFreed = result.first;
    // then
    EXPECT_EQ(segmentFreed, false);
    EXPECT_NE(segInfos.GetState(), SegmentState::FREE);
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

TEST(SegmentInfo, IncreaseOccupiedStripeCount_TestIncreaseValue)
{
    // given
    SegmentInfo segInfos;
    // when, then
    EXPECT_EQ(segInfos.IncreaseOccupiedStripeCount(), 1);
    EXPECT_EQ(segInfos.IncreaseOccupiedStripeCount(), 2);
    EXPECT_EQ(segInfos.IncreaseOccupiedStripeCount(), 3);
    EXPECT_EQ(segInfos.IncreaseOccupiedStripeCount(), 4);
    EXPECT_EQ(segInfos.IncreaseOccupiedStripeCount(), 5);
}

TEST(SegmentInfo, MoveToNvramState_testIfStateChanged)
{
    // given
    SegmentInfo segInfos;
    // when
    segInfos.MoveToNvramState();
    // then
    EXPECT_EQ(segInfos.GetState(), SegmentState::NVRAM);
}

TEST(SegmentInfo, MoveToSsdState_testIfStateChangedToSSD)
{
    // given
    SegmentInfo segInfos(10, 10, SegmentState::NVRAM);
    // when
    segInfos.MoveToSsdStateOrFreeStateIfItBecomesEmpty();
    // then
    EXPECT_EQ(segInfos.GetState(), SegmentState::SSD);
}

TEST(SegmentInfo, MoveToSsdState_testIfStateChangedToFree)
{
    // given
    SegmentInfo segInfos(0, 10, SegmentState::NVRAM);
    // when
    segInfos.MoveToSsdStateOrFreeStateIfItBecomesEmpty();
    // then
    EXPECT_EQ(segInfos.GetState(), SegmentState::FREE);
}

TEST(SegmentInfo, MoveToVictimState_testIfStateChanged)
{
    // given
    SegmentInfo segInfos(10, 10, SegmentState::SSD);
    // when
    segInfos.MoveToVictimState();
    // then
    EXPECT_EQ(segInfos.GetState(), SegmentState::VICTIM);
}

TEST(SegmentInfo, GetValidBlockCountIfSsdState_testIfValidCountIsReturnedWhenItsSsdState)
{
    // given
    SegmentInfo segInfos(34, 10, SegmentState::SSD);
    // when
    EXPECT_EQ(segInfos.GetValidBlockCountIfSsdState(), 34);
}

TEST(SegmentInfo, GetValidBlockCountIfSsdState_testIfValidCountIsReturnedWhenItsNotSsdState)
{
    SegmentInfo freeSegInfo(0, 0, SegmentState::FREE);
    EXPECT_EQ(freeSegInfo.GetValidBlockCountIfSsdState(), UINT32_MAX);

    SegmentInfo nvramSegInfo(30, 0, SegmentState::NVRAM);
    EXPECT_EQ(freeSegInfo.GetValidBlockCountIfSsdState(), UINT32_MAX);

    SegmentInfo victimSegInfo(30, 0, SegmentState::VICTIM);
    EXPECT_EQ(freeSegInfo.GetValidBlockCountIfSsdState(), UINT32_MAX);
}

} // namespace pos
