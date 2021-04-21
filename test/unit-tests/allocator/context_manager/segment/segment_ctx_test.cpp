#include "src/allocator/context_manager/segment/segment_ctx.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/segment/segment_info_mock.h"
#include "test/unit-tests/allocator/context_manager/segment/segment_states_mock.h"

#include <gtest/gtest.h>

using namespace ::testing;

namespace pos
{
TEST(SegmentCtx, SegmentCtx_)
{
}

TEST(SegmentCtx, Init_)
{
}

TEST(SegmentCtx, Close_)
{
}

TEST(SegmentCtx, GetGcThreshold_)
{
}

TEST(SegmentCtx, GetUrgentThreshold_)
{
}

TEST(SegmentCtx, GetGCVictimSegment_)
{
}

TEST(SegmentCtx, GetNumOfFreeUserDataSegment_)
{
}

TEST(SegmentCtx, FreeUserDataSegment_)
{
}

TEST(SegmentCtx, ReplaySsdLsid_)
{
}

TEST(SegmentCtx, ReplaySegmentAllocation_)
{
}

TEST(SegmentCtx, UpdateOccupiedStripeCount_)
{
}

TEST(SegmentCtx, SetGcThreshold_)
{
}

TEST(SegmentCtx, SetUrgentThreshold_)
{
}

TEST(SegmentCtx, GetPrevSsdLsid_)
{
}

TEST(SegmentCtx, SetPrevSsdLsid_)
{
}

TEST(SegmentCtx, GetCurrentSsdLsid_)
{
}

TEST(SegmentCtx, SetCurrentSsdLsid_)
{
}

TEST(SegmentCtx, GetSegmentState_)
{
}

TEST(SegmentCtx, UsedSegmentStateChange_)
{
}

TEST(SegmentCtx, GetSegmentBitmap_)
{
}

TEST(SegmentCtx, IncreaseInvalidBlockCount_)
{
}

TEST(SegmentCtx, GetInvalidBlockCount_)
{
}

TEST(SegmentCtx, GetNumSegment_)
{
}

TEST(SegmentCtx, StoreSegmentInfoSync_)
{
}

TEST(SegmentCtx, StoreSegmentInfoAsync_)
{
}

TEST(SegmentCtx, ReleaseRequestIo_)
{
}

TEST(SegmentCtx, IsSegmentInfoRequestIo_)
{
}

TEST(SegmentCtx, FreeAllInvalidatedSegment_SSDStateAndAllInvalidated)
{
    // Given
    const int NUM_SEGMENT = 1;

    MockAllocatorAddressInfo mockAllocatorAddressInfo;
    EXPECT_CALL(mockAllocatorAddressInfo, GetnumUserAreaSegments).WillRepeatedly(Return(NUM_SEGMENT));

    MockSegmentStates mockSegmentStates;
    mockSegmentStates.Setstate(SegmentState::SSD);

    MockSegmentInfo mockSegmentInfo(1);
    EXPECT_CALL(mockSegmentInfo, GetValidBlockCount).WillOnce(Return(0));

    SegmentCtx segmentCtxSUT(nullptr, 0, 0, &mockSegmentStates, &mockSegmentInfo, 0, 0, 0, 0, 0, "arr0", &mockAllocatorAddressInfo, nullptr, nullptr);

    // When
    segmentCtxSUT.FreeAllInvalidatedSegment();

    // Then
    EXPECT_EQ(mockSegmentStates.Getstate(), SegmentState::FREE);
}

} // namespace pos
