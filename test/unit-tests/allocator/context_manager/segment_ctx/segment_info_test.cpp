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

TEST(SegmentInfo, SegmentInfoData_ConstructorInitializationValue)
{
    int numSegInfos = 4;
    SegmentInfo* segInfos = new SegmentInfo[numSegInfos];
    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegInfos](0, 0, SegmentState::FREE);
    for (int i = 0; i < numSegInfos; ++i)
    {
        segInfos[i].AllocateAndInitSegmentInfoData(&segmentInfoData[i]);
    }

    for (int i = 0; i < numSegInfos; ++i)
    {
        EXPECT_EQ(segInfos[i].GetValidBlockCount(), 0);
        EXPECT_EQ(segInfos[i].GetOccupiedStripeCount(), 0);
        EXPECT_EQ(segInfos[i].GetState(), SegmentState::FREE);
    }

    delete[] segInfos;
    delete[] segmentInfoData;
}

TEST(SegmentInfo, InitSegmentInfoData_testIfSegmentInfoDataInitializationSuccess)
{
    int numSegInfos = 4;
    SegmentInfo* segInfos = new SegmentInfo[numSegInfos];
    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegInfos];
    for (int i = 0; i < numSegInfos; ++i)
    {
        segInfos[i].AllocateAndInitSegmentInfoData(&segmentInfoData[i]);
    }

    for (int i = 0; i < numSegInfos; ++i)
    {
        EXPECT_EQ(segInfos[i].GetValidBlockCount(), 0);
        EXPECT_EQ(segInfos[i].GetOccupiedStripeCount(), 0);
        EXPECT_EQ(segInfos[i].GetState(), SegmentState::FREE);
    }

    delete[] segInfos;
    delete[] segmentInfoData;
}

TEST(SegmentInfo, SetValidBlockCount_TestSimpleSetter)
{
    SegmentInfo segInfos;
    SegmentInfoData segmentInfoData(0, 0, SegmentState::FREE);
    segInfos.AllocateAndInitSegmentInfoData(&segmentInfoData);

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
    SegmentInfoData segmentInfoData(0, 0, SegmentState::FREE);
    segInfos.AllocateAndInitSegmentInfoData(&segmentInfoData);

    EXPECT_EQ(segInfos.IncreaseValidBlockCount(5), 5);
    EXPECT_EQ(segInfos.IncreaseValidBlockCount(3), 8);
    EXPECT_EQ(segInfos.IncreaseValidBlockCount(10), 18);

}

TEST(SegmentInfo, DecreaseValidBlockCount_testDecreaseToNonZero)
{
    // given
    SegmentInfo segInfos;
    SegmentInfoData segmentInfoData(10, 0, SegmentState::FREE);
    segInfos.AllocateAndInitSegmentInfoData(&segmentInfoData);
    segInfos.SetValidBlockCount(10);

    // when
    auto result = segInfos.DecreaseValidBlockCount(3, false);
    bool segmentFreed = result.first;
    // then
    EXPECT_EQ(segmentFreed, false);
}

TEST(SegmentInfo, DecreaseValidBlockCount_testDecreaseToZeroWhenSsdState)
{
    // given
    SegmentInfo segInfos;
    SegmentInfoData segmentInfoData(3, 10, SegmentState::SSD);
    segInfos.AllocateAndInitSegmentInfoData(&segmentInfoData);
    segInfos.SetValidBlockCount(3);
    segInfos.SetOccupiedStripeCount(10);
    segInfos.SetState(SegmentState::SSD);
    // when
    auto result = segInfos.DecreaseValidBlockCount(3, false);
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
    SegmentInfo segInfos;
    SegmentInfoData segmentInfoData(3, 10, SegmentState::NVRAM);
    segInfos.AllocateAndInitSegmentInfoData(&segmentInfoData);
    segInfos.SetValidBlockCount(3);
    segInfos.SetOccupiedStripeCount(10);
    segInfos.SetState(SegmentState::NVRAM);
    // when
    auto result = segInfos.DecreaseValidBlockCount(3, false);
    bool segmentFreed = result.first;
    // then
    EXPECT_EQ(segmentFreed, false);
    EXPECT_NE(segInfos.GetState(), SegmentState::FREE);
}

TEST(SegmentInfo, SetOccupiedStripeCount_TestSimpleSetter)
{
    // given
    SegmentInfo segInfos;
    SegmentInfoData segmentInfoData(0, 0, SegmentState::FREE);
    segInfos.AllocateAndInitSegmentInfoData(&segmentInfoData);

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
    SegmentInfoData segmentInfoData(0, 0, SegmentState::FREE);
    segInfos.AllocateAndInitSegmentInfoData(&segmentInfoData);

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
    SegmentInfoData segmentInfoData(0, 0, SegmentState::FREE);
    segInfos.AllocateAndInitSegmentInfoData(&segmentInfoData);
    // when
    segInfos.MoveToNvramState();
    // then
    EXPECT_EQ(segInfos.GetState(), SegmentState::NVRAM);
}

TEST(SegmentInfo, MoveToSsdState_testIfStateChangedToSSD)
{
    // given
    SegmentInfo segInfos;
    SegmentInfoData segmentInfoData(10, 10, SegmentState::NVRAM);
    segInfos.AllocateAndInitSegmentInfoData(&segmentInfoData);
    segInfos.SetValidBlockCount(10);
    segInfos.SetOccupiedStripeCount(10);
    segInfos.SetState(SegmentState::NVRAM);
    // when
    segInfos.MoveToSsdStateOrFreeStateIfItBecomesEmpty();
    // then
    EXPECT_EQ(segInfos.GetState(), SegmentState::SSD);
}

TEST(SegmentInfo, MoveToSsdState_testIfStateChangedToFree)
{
    // given
    SegmentInfo segInfos;
    SegmentInfoData segmentInfoData(0, 10, SegmentState::NVRAM);
    segInfos.AllocateAndInitSegmentInfoData(&segmentInfoData);
    segInfos.SetValidBlockCount(0);
    segInfos.SetOccupiedStripeCount(10);
    segInfos.SetState(SegmentState::NVRAM);
    // when
    segInfos.MoveToSsdStateOrFreeStateIfItBecomesEmpty();
    // then
    EXPECT_EQ(segInfos.GetState(), SegmentState::FREE);
}

TEST(SegmentInfo, MoveToVictimState_testIfStateChanged)
{
    // given
    SegmentInfo segInfos;
    SegmentInfoData segmentInfoData(10, 10, SegmentState::SSD);
    segInfos.AllocateAndInitSegmentInfoData(&segmentInfoData);
    segInfos.SetValidBlockCount(10);
    segInfos.SetOccupiedStripeCount(10);
    segInfos.SetState(SegmentState::SSD);
    // when
    segInfos.MoveToVictimState();
    // then
    EXPECT_EQ(segInfos.GetState(), SegmentState::VICTIM);
}

TEST(SegmentInfo, GetValidBlockCountIfSsdState_testIfValidCountIsReturnedWhenItsSsdState)
{
    // given
    SegmentInfo segInfos;
    SegmentInfoData segmentInfoData(34, 10, SegmentState::SSD);
    segInfos.AllocateAndInitSegmentInfoData(&segmentInfoData);
    segInfos.SetValidBlockCount(34);
    segInfos.SetOccupiedStripeCount(10);
    segInfos.SetState(SegmentState::SSD);
    // when
    EXPECT_EQ(segInfos.GetValidBlockCountIfSsdState(), 34);
}

TEST(SegmentInfo, GetValidBlockCountIfSsdState_testIfValidCountIsReturnedWhenItsNotSsdState)
{
    SegmentInfo freeSegInfo;
    SegmentInfoData freeSegInfoData(0, 10, SegmentState::FREE);
    freeSegInfo.AllocateAndInitSegmentInfoData(&freeSegInfoData);
    freeSegInfo.SetValidBlockCount(0);
    freeSegInfo.SetOccupiedStripeCount(10);
    freeSegInfo.SetState(SegmentState::FREE);
    EXPECT_EQ(freeSegInfo.GetValidBlockCountIfSsdState(), UINT32_MAX);

    SegmentInfo nvramSegInfo;
    SegmentInfoData nvramSegInfoData(30, 0, SegmentState::NVRAM);
    nvramSegInfo.AllocateAndInitSegmentInfoData(&nvramSegInfoData);
    nvramSegInfo.SetValidBlockCount(30);
    nvramSegInfo.SetOccupiedStripeCount(0);
    nvramSegInfo.SetState(SegmentState::NVRAM);
    EXPECT_EQ(freeSegInfo.GetValidBlockCountIfSsdState(), UINT32_MAX);

    SegmentInfo victimSegInfo;
    SegmentInfoData victimSegInfoData(30, 0, SegmentState::VICTIM);
    victimSegInfo.AllocateAndInitSegmentInfoData(&victimSegInfoData);
    victimSegInfo.SetValidBlockCount(30);
    victimSegInfo.SetOccupiedStripeCount(0);
    victimSegInfo.SetState(SegmentState::VICTIM);
    EXPECT_EQ(freeSegInfo.GetValidBlockCountIfSsdState(), UINT32_MAX);
}

TEST(SegmentInfo, UpdateFrom_testIfSegmentInfoDataIsSuccessfullyUpdatedByUpdateFromMethod)
{
    // Given initialized SegmentInfo and specific data allocated SegmentInfo.
    SegmentInfo SegInfo;
    SegmentInfoData SegInfoData(0, 0, SegmentState::FREE);
    SegInfo.AllocateAndInitSegmentInfoData(&SegInfoData);
    SegInfo.SetValidBlockCount(0);
    SegInfo.SetOccupiedStripeCount(0);
    SegInfo.SetState(SegmentState::FREE);
    // initialized specific values in filledSegInfo.
    SegmentInfo filledSegInfo;
    SegmentInfoData filledSegInfoData(5, 3, SegmentState::VICTIM);
    filledSegInfo.AllocateAndInitSegmentInfoData(&filledSegInfoData);
    filledSegInfo.SetValidBlockCount(5);
    filledSegInfo.SetOccupiedStripeCount(3);
    filledSegInfo.SetState(SegmentState::VICTIM);
    // When execute SegmentInfo::UpdateFrom()
    SegInfo.UpdateFrom(filledSegInfo);

    // Then SegInfo data must be updated.
    EXPECT_EQ(SegInfo.GetValidBlockCount(), 5);
    EXPECT_EQ(SegInfo.GetOccupiedStripeCount(), 3);
    EXPECT_EQ(SegInfo.GetState(), SegmentState::VICTIM);
}

} // namespace pos
