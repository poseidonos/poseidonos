#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/segment_lock_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/segment_states_mock.h"
#include "test/unit-tests/lib/bitmap_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(AllocatorCtx, AfterLoad_)
{
    // given
    AllocatorCtxHeader* buf = new AllocatorCtxHeader();
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    AllocatorCtx allocCtx(allocBitmap, segStates, segLocks, nullptr, "");

    // given 1.
    EXPECT_CALL(*allocBitmap, SetNumBitsSet).WillOnce(Return());
    buf->sig = AllocatorCtx::SIG_ALLOCATOR_CTX;
    // when 1.
    allocCtx.AfterLoad((char*)buf);

    // given 2.
    buf->sig = 0;
    // when 2.
    EXPECT_DEATH(allocCtx.AfterLoad((char*)buf), "");

    delete allocBitmap;
    delete segStates;
    delete segLocks;
    delete buf;
}

TEST(AllocatorCtx, GetStoredVersion_)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, nullptr, "");
    // when
    allocCtx.GetStoredVersion();
}

TEST(AllocatorCtx, ResetDirtyVersion_)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, nullptr, "");
    // when
    allocCtx.ResetDirtyVersion();
}

TEST(AllocatorCtx, UpdatePrevLsid_)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, nullptr, "");
    // when
    allocCtx.UpdatePrevLsid();
}

TEST(AllocatorCtx, SetCurrentSsdLsid_)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, nullptr, "");
    // when
    allocCtx.SetCurrentSsdLsid(10);
}

TEST(AllocatorCtx, RollbackCurrentSsdLsid_)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, nullptr, "");
    // when
    allocCtx.RollbackCurrentSsdLsid();
}

TEST(AllocatorCtx, SetPrevSsdLsid_)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, nullptr, "");
    // when
    allocCtx.SetPrevSsdLsid(10);
}

TEST(AllocatorCtx, SetNextSsdLsid_)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    AllocatorCtx allocCtx(allocBitmap, segStates, segLocks, &addrInfo, "");

    EXPECT_CALL(*segStates, SetState);
    // when
    allocCtx.SetNextSsdLsid(0);

    delete allocBitmap;
    delete segStates;
    delete segLocks;
}

TEST(AllocatorCtx, AllocateSegment_)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    AllocatorCtx allocCtx(allocBitmap, segStates, segLocks, nullptr, "");

    EXPECT_CALL(*allocBitmap, SetBit);
    // when
    allocCtx.AllocateSegment(10);

    delete allocBitmap;
    delete segStates;
    delete segLocks;
}

TEST(AllocatorCtx, ReleaseSegment_)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    AllocatorCtx allocCtx(allocBitmap, segStates, segLocks, nullptr, "");

    EXPECT_CALL(*allocBitmap, ClearBit);
    // when
    allocCtx.ReleaseSegment(10);

    delete allocBitmap;
    delete segStates;
    delete segLocks;
}

TEST(AllocatorCtx, AllocateFreeSegment_)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    AllocatorCtx allocCtx(allocBitmap, segStates, segLocks, nullptr, "");

    // given 1.
    EXPECT_CALL(*allocBitmap, SetFirstZeroBit).WillOnce(Return(22));
    EXPECT_CALL(*allocBitmap, SetNextZeroBit).Times(0);
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(true));
    // when 1.
    int ret = allocCtx.AllocateFreeSegment(10);
    // then 1.
    EXPECT_EQ(22, ret);

    // given 2.
    EXPECT_CALL(*allocBitmap, SetFirstZeroBit).WillOnce(Return(22));
    EXPECT_CALL(*allocBitmap, SetNextZeroBit).Times(0);
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(false));
    // when 2.
    ret = allocCtx.AllocateFreeSegment(10);
    // then 2.
    EXPECT_EQ(UNMAP_SEGMENT, ret);

    // given 3.
    EXPECT_CALL(*allocBitmap, SetNextZeroBit).WillOnce(Return(10));
    EXPECT_CALL(*allocBitmap, SetFirstZeroBit).Times(0);
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(true));
    // when 3.
    ret = allocCtx.AllocateFreeSegment(UNMAP_SEGMENT);
    // then 3.
    EXPECT_EQ(10, ret);

    // given 4.
    EXPECT_CALL(*allocBitmap, SetNextZeroBit).WillOnce(Return(10));
    EXPECT_CALL(*allocBitmap, SetFirstZeroBit).Times(0);
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(false));
    // when 4.
    ret = allocCtx.AllocateFreeSegment(UNMAP_SEGMENT);
    // then 4.
    EXPECT_EQ(UNMAP_SEGMENT, ret);

    delete allocBitmap;
    delete segStates;
    delete segLocks;
}

TEST(AllocatorCtx, GetNumOfFreeUserDataSegment_)
{
}

TEST(AllocatorCtx, GetSegmentState_)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    AllocatorCtx allocCtx(allocBitmap, segStates, segLocks, nullptr, "");

    // given 1.
    EXPECT_CALL(*segStates, GetState).WillOnce(Return(SegmentState::FREE));
    // when 1.
    SegmentState ret = allocCtx.GetSegmentState(0, false);
    // then 1.
    EXPECT_EQ(SegmentState::FREE, ret);

    // given 2.
    EXPECT_CALL(*segStates, GetState).WillOnce(Return(SegmentState::SSD));
    // when 2.
    ret = allocCtx.GetSegmentState(0, true);
    // then 2.
    EXPECT_EQ(SegmentState::SSD, ret);

    delete allocBitmap;
    delete segStates;
    delete segLocks;
}

TEST(AllocatorCtx, SetSegmentState_)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    AllocatorCtx allocCtx(allocBitmap, segStates, segLocks, nullptr, "");

    // given 1.
    EXPECT_CALL(*segStates, SetState);
    // when 1.
    allocCtx.SetSegmentState(0, SegmentState::FREE, false);

    // given 2.
    EXPECT_CALL(*segStates, SetState);
    // when 2.
    allocCtx.SetSegmentState(0, SegmentState::SSD, true);

    delete allocBitmap;
    delete segStates;
    delete segLocks;
}

TEST(AllocatorCtx, SetAllocatedSegmentCount_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    AllocatorCtx allocCtx(allocBitmap, segStates, segLocks, nullptr, "");

    EXPECT_CALL(*allocBitmap, SetNumBitsSet);
    // when 1.
    allocCtx.SetAllocatedSegmentCount(10);

    delete allocBitmap;
    delete segStates;
    delete segLocks;
}

TEST(AllocatorCtx, GetAllocatedSegmentCount_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    AllocatorCtx allocCtx(allocBitmap, segStates, segLocks, nullptr, "");

    EXPECT_CALL(*allocBitmap, GetNumBitsSet).WillOnce(Return(10));
    // when
    int ret = allocCtx.GetAllocatedSegmentCount();
    EXPECT_EQ(10, ret);

    delete allocBitmap;
    delete segStates;
    delete segLocks;
}

TEST(AllocatorCtx, GetTotalSegmentsCount_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    AllocatorCtx allocCtx(allocBitmap, segStates, segLocks, nullptr, "");

    EXPECT_CALL(*allocBitmap, GetNumBits).WillOnce(Return(100));
    // when
    int ret = allocCtx.GetTotalSegmentsCount();
    EXPECT_EQ(100, ret);

    delete allocBitmap;
    delete segStates;
    delete segLocks;
}

} // namespace pos
