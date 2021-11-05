#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

#include <gtest/gtest.h>

#include "src/allocator/include/allocator_const.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_info_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_lock_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_states_mock.h"
#include "test/unit-tests/lib/bitmap_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(SegmentCtx, AfterLoad_testIfSegmentSignatureSuccess)
{
    // given
    SegmentCtxHeader header;
    header.sig = SegmentCtx::SIG_SEGMENT_CTX;
    header.ctxVersion = 5;
    header.numValidSegment = 100;

    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(100);
    SegmentCtx segCtx(&header, nullptr, nullptr, nullptr, segmentBitmap, nullptr, nullptr);

    EXPECT_CALL(*segmentBitmap, SetNumBitsSet(100));

    // when
    segCtx.AfterLoad(nullptr);

    EXPECT_EQ(segCtx.GetStoredVersion(), header.ctxVersion);
    delete segmentBitmap;
}

TEST(SegmentCtx, BeforeFlush_TestSimpleSetter)
{
    // given
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(100);
    SegmentCtxHeader* buf = new SegmentCtxHeader();
    SegmentCtx segCtx(nullptr, nullptr, nullptr, nullptr, segmentBitmap, nullptr, nullptr);
    buf->sig = SegmentCtx::SIG_SEGMENT_CTX;

    // when
    segCtx.BeforeFlush((char*)buf);

    delete buf;
    delete segmentBitmap;
}

TEST(SegmentCtx, GetCtxLock_TestSimpleGetter)
{
    SegmentCtx segCtx(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    std::mutex& m = segCtx.GetCtxLock();
}

TEST(SegmentCtx, FinalizeIo_TestSimpleSetter)
{
    // given
    SegmentCtx segCtx(nullptr, nullptr, nullptr, nullptr);
    SegmentCtxHeader* buf = new SegmentCtxHeader();
    buf->sig = SegmentCtx::SIG_SEGMENT_CTX;
    AsyncMetaFileIoCtx ctx;
    ctx.buffer = (char*)buf;
    // when
    segCtx.FinalizeIo(&ctx);

    delete buf;
}

TEST(SegmentCtx, IncreaseOccupiedStripeCount_IfOccupiedStripeCountSmallerThanMax)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);

    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr, &addrInfo);

    EXPECT_CALL(*segInfos, IncreaseOccupiedStripeCount).WillOnce(Return(1));

    // when
    bool segmentFreed = segCtx.IncreaseOccupiedStripeCount(0);
    // then
    EXPECT_EQ(segmentFreed, false);

    delete segInfos;
}

TEST(SegmentCtx, IncreaseOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndValidCountIsNotZero)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    addrInfo.SetnumUserAreaSegments(1);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, nullptr, &addrInfo);

    std::mutex lock;
    EXPECT_CALL(*segStateLock, GetLock).WillOnce(ReturnRef(lock));

    // when
    EXPECT_CALL(*segInfos, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*segInfos, GetValidBlockCount).WillOnce(Return(1));
    bool segmentFreed = segCtx.IncreaseOccupiedStripeCount(0);
    // then
    EXPECT_EQ(segmentFreed, false);

    delete segInfos;
    delete segStates;
    delete segStateLock;
    delete segmentBitmap;
}

TEST(SegmentCtx, IncreaseOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndValidCountIsZeroAndSegStateFree)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    addrInfo.SetnumUserAreaSegments(1);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, nullptr, &addrInfo);

    std::mutex lock;
    EXPECT_CALL(*segStateLock, GetLock).WillOnce(ReturnRef(lock));

    // when
    EXPECT_CALL(*segInfos, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*segInfos, GetValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(*segStates, GetState).WillOnce(Return(SegmentState::FREE));
    bool segmentFreed = segCtx.IncreaseOccupiedStripeCount(0);
    // then
    EXPECT_EQ(segmentFreed, false);

    delete segInfos;
    delete segStates;
    delete segStateLock;
    delete segmentBitmap;
}

TEST(SegmentCtx, IncreaseOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndValidCountIsZeroAndSegStateSSD)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    addrInfo.SetnumUserAreaSegments(1);

    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, nullptr, &addrInfo);

    std::mutex lock;
    EXPECT_CALL(*segStateLock, GetLock).WillOnce(ReturnRef(lock));

    // when
    EXPECT_CALL(*segInfos, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*segInfos, GetValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(*segStates, GetState).WillOnce(Return(SegmentState::SSD));

    // then
    EXPECT_CALL(*segInfos, SetOccupiedStripeCount(0)).Times(1);
    EXPECT_CALL(*segStates, SetState(SegmentState::FREE)).Times(1);

    bool segmentFreed = segCtx.IncreaseOccupiedStripeCount(0);
    EXPECT_EQ(segmentFreed, true);

    delete segInfos;
    delete segStates;
    delete segStateLock;
    delete segmentBitmap;
}

TEST(SegmentCtx, IncreaseValidBlockCount_TestIncreaseValue)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr, &addrInfo);

    // given 1.
    EXPECT_CALL(*segInfos, IncreaseValidBlockCount).WillOnce(Return(3));
    EXPECT_CALL(addrInfo, GetblksPerSegment).WillOnce(Return(5));
    // when 1.
    int ret = segCtx.IncreaseValidBlockCount(0, 1);
    // then 1.
    EXPECT_EQ(3, ret);
    // given 2.
    EXPECT_CALL(*segInfos, IncreaseValidBlockCount).WillOnce(Return(6));
    EXPECT_CALL(addrInfo, GetblksPerSegment).WillOnce(Return(5));
    EXPECT_CALL(addrInfo, IsUT).WillOnce(Return(false)).WillOnce(Return(true));
    // when 2.
    ret = segCtx.IncreaseValidBlockCount(0, 1);

    delete segInfos;
}

TEST(SegmentCtx, DecreaseValidBlockCount_TestDecreaseValue)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr, addrInfo);

    // given 1.
    EXPECT_CALL(*segInfos, DecreaseValidBlockCount).WillOnce(Return(4));
    // when 1.
    bool ret = segCtx.DecreaseValidBlockCount(0, 1);
    // then 1.
    EXPECT_EQ(false, ret);

    delete addrInfo;
    delete segInfos;
}

TEST(SegmentCtx, DecreaseValidBlockCount_TestDecreaseValueWhenValidCountZeroAndNVRAMState)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, nullptr, addrInfo);

    std::mutex lock;
    EXPECT_CALL(*segStateLock, GetLock).WillOnce(ReturnRef(lock));

    // given 1.
    EXPECT_CALL(*segInfos, DecreaseValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(*segStates, GetState).WillOnce(Return(SegmentState::NVRAM));

    // when 1.
    bool ret = segCtx.DecreaseValidBlockCount(0, 1);
    EXPECT_EQ(false, ret);

    delete addrInfo;
    delete segInfos;
    delete segStates;
    delete segStateLock;
    delete segmentBitmap;
}

TEST(SegmentCtx, DecreaseValidBlockCount_TestDecreaseValueWhenValidCountZeroAndSSDState)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, nullptr, addrInfo);

    std::mutex lock;
    EXPECT_CALL(*segStateLock, GetLock).WillOnce(ReturnRef(lock));

    // given 1.
    EXPECT_CALL(*segInfos, DecreaseValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(*segStates, GetState).WillOnce(Return(SegmentState::SSD));
    EXPECT_CALL(*segInfos, GetOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*addrInfo, GetstripesPerSegment).WillOnce(Return(100));
    EXPECT_CALL(*segInfos, SetOccupiedStripeCount(0)).Times(1);
    EXPECT_CALL(*segStates, SetState(SegmentState::FREE)).Times(1);

    // when 1.
    bool ret = segCtx.DecreaseValidBlockCount(0, 1);
    EXPECT_EQ(true, ret);

    delete addrInfo;
    delete segInfos;
    delete segStates;
    delete segStateLock;
    delete segmentBitmap;
}

TEST(SegmentCtx, GetValidBlockCount_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr, nullptr);

    // given 1.
    EXPECT_CALL(*segInfos, GetValidBlockCount).WillOnce(Return(10));
    // when 1.
    int ret = segCtx.GetValidBlockCount(0);
    // then 1.
    EXPECT_EQ(10, ret);

    delete segInfos;
}

TEST(SegmentCtx, GetOccupiedStripeCount_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr, nullptr);

    // given 1.
    EXPECT_CALL(*segInfos, GetOccupiedStripeCount).WillOnce(Return(5));
    // when 1.
    int ret = segCtx.GetOccupiedStripeCount(0);
    // then 1.
    EXPECT_EQ(5, ret);

    delete segInfos;
}

TEST(SegmentCtx, IncreaseOccupiedStripeCount)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr, &addrInfo);

    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillOnce(Return(100));
    // given 1.
    EXPECT_CALL(*segInfos, IncreaseOccupiedStripeCount).WillOnce(Return(6));
    // when 1.
    bool segmentFreed = segCtx.IncreaseOccupiedStripeCount(0);
    // then 1.
    EXPECT_EQ(false, segmentFreed);

    delete segInfos;
}

TEST(SegmentCtx, GetSegmentState_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    addrInfo.SetnumUserAreaSegments(1);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segLocks, segmentBitmap, nullptr, &addrInfo);

    // given 1.
    EXPECT_CALL(*segStates, GetState).WillOnce(Return(SegmentState::FREE));
    // when 1.
    SegmentState ret = segCtx.GetSegmentState(0, false);
    // then 1.
    EXPECT_EQ(SegmentState::FREE, ret);

    // given 2.
    std::mutex segStateLock;
    EXPECT_CALL(*segLocks, GetLock).WillOnce(ReturnRef(segStateLock));
    EXPECT_CALL(*segStates, GetState).WillOnce(Return(SegmentState::SSD));
    // when 2.
    ret = segCtx.GetSegmentState(0, true);
    // then 2.
    EXPECT_EQ(SegmentState::SSD, ret);

    delete segInfos;
    delete segStates;
    delete segLocks;
    delete segmentBitmap;
}

TEST(SegmentCtx, SetSegmentState_TestSimpleSetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(1000);
    addrInfo.SetnumUserAreaSegments(1);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segLocks, segmentBitmap, nullptr, &addrInfo);

    // given 1.
    EXPECT_CALL(*segStates, SetState);
    // when 1.
    segCtx.SetSegmentState(0, SegmentState::FREE, false);

    // given 2.
    std::mutex segStateLock;
    EXPECT_CALL(*segLocks, GetLock).WillOnce(ReturnRef(segStateLock));
    EXPECT_CALL(*segStates, SetState);
    // when 2.
    segCtx.SetSegmentState(0, SegmentState::SSD, true);

    delete segInfos;
    delete segStates;
    delete segLocks;
    delete segmentBitmap;
}

TEST(SegmentCtx, GetSegStateLock_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segLocks, segmentBitmap, nullptr, nullptr);

    std::mutex m;
    EXPECT_CALL(*segLocks, GetLock).WillOnce(ReturnRef(m));
    // when
    segCtx.GetSegStateLock(0);

    delete segInfos;
    delete segStates;
    delete segLocks;
    delete segmentBitmap;
}

TEST(SegmentCtx, CopySegmentInfoToBufferforWBT_CheckCopiedBuffer)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(1);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr, &addrInfo);

    uint32_t result = 0;
    // given 1.
    EXPECT_CALL(*segInfos, GetValidBlockCount).WillOnce(Return(8));
    EXPECT_CALL(*segInfos, GetOccupiedStripeCount).Times(0);
    // when 1.
    segCtx.CopySegmentInfoToBufferforWBT(WBT_SEGMENT_VALID_COUNT, (char*)&result);
    // then 1.
    EXPECT_EQ(8, result);

    // given 2.
    EXPECT_CALL(*segInfos, GetOccupiedStripeCount).WillOnce(Return(12));
    EXPECT_CALL(*segInfos, GetValidBlockCount).Times(0);
    // when 2.
    segCtx.CopySegmentInfoToBufferforWBT(WBT_SEGMENT_OCCUPIED_STRIPE, (char*)&result);
    // then 2.
    EXPECT_EQ(12, result);

    delete segInfos;
}

TEST(SegmentCtx, GetSegmentCtxLock_TestSimpleGetter)
{
    // given
    SegmentCtx segCtx(nullptr, nullptr, nullptr, nullptr);

    // when
    std::mutex& m = segCtx.GetSegmentCtxLock();
}

TEST(SegmentCtx, Init_testInitAndClose)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    SegmentCtx segCtx(nullptr, &addrInfo);

    // when
    segCtx.Init();

    // then
    segCtx.Dispose();
}

TEST(SegmentCtx, GetSectionAddr_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segLocks, segmentBitmap, nullptr, nullptr);

    // when 1.
    char* buf = segCtx.GetSectionAddr(SC_HEADER);

    // when 2.
    buf = segCtx.GetSectionAddr(SC_SEGMENT_INFO);
    EXPECT_EQ(reinterpret_cast<char*>(segInfos), buf);

    // when 3.
    buf = segCtx.GetSectionAddr(AC_SEGMENT_STATES);
    EXPECT_EQ(reinterpret_cast<char*>(segStates), buf);

    // when 4.
    EXPECT_CALL(*segmentBitmap, GetMapAddr).WillOnce(Return((uint64_t*)segmentBitmap));
    buf = segCtx.GetSectionAddr(AC_ALLOCATE_SEGMENT_BITMAP);
    EXPECT_EQ(reinterpret_cast<char*>(segmentBitmap), buf);

    delete segInfos;
    delete segStates;
    delete segLocks;
    delete segmentBitmap;
}

TEST(SegmentCtx, GetSectionSize_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    addrInfo.SetnumUserAreaSegments(10);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(10);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segLocks, segmentBitmap, nullptr, &addrInfo);

    // when 1.
    int ret = segCtx.GetSectionSize(SC_HEADER);
    // then 1.
    EXPECT_EQ(sizeof(SegmentCtxHeader), ret);

    // when 2.
    ret = segCtx.GetSectionSize(SC_SEGMENT_INFO);
    EXPECT_EQ(10 * sizeof(SegmentInfo), ret);

    // when 3.
    ret = segCtx.GetSectionSize(AC_SEGMENT_STATES);
    EXPECT_EQ(sizeof(SegmentStates) * 10, ret);

    // when 4.
    EXPECT_CALL(*segmentBitmap, GetNumEntry).WillOnce(Return(10));
    ret = segCtx.GetSectionSize(AC_ALLOCATE_SEGMENT_BITMAP);
    EXPECT_EQ(10 * BITMAP_ENTRY_SIZE, ret);

    delete segInfos;
    delete segStates;
    delete segLocks;
    delete segmentBitmap;
}

TEST(SegmentCtx, GetStoredVersion_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr, nullptr);

    // when 1.
    int ret = segCtx.GetStoredVersion();
    delete segInfos;
}

TEST(SegmentCtx, CopySegmentInfoFromBufferforWBT_TestSimpleSetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(1);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr, &addrInfo);
    char* buf = new char[1];

    // given 1.
    EXPECT_CALL(*segInfos, SetValidBlockCount);
    // when 1.
    segCtx.CopySegmentInfoFromBufferforWBT(WBT_SEGMENT_VALID_COUNT, buf);

    // given 1.
    EXPECT_CALL(*segInfos, SetOccupiedStripeCount);
    // when 1.
    segCtx.CopySegmentInfoFromBufferforWBT(WBT_SEGMENT_OCCUPIED_STRIPE, buf);

    delete segInfos;
}

TEST(SegmentCtx, GetSegmentInfo_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr, nullptr);
    // when
    segCtx.GetSegmentInfo();
}

TEST(SegmentCtx, ResetDirtyVersion_TestSimpleSetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr, nullptr);
    // when
    segCtx.ResetDirtyVersion();
}

TEST(SegmentCtx, AllocateSegment_TestSimpleInterfaceFunc)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, nullptr, nullptr);

    EXPECT_CALL(*segStates, SetState(SegmentState::NVRAM));
    EXPECT_CALL(*segmentBitmap, SetBit(0));
    // when
    segCtx.AllocateSegment(0);

    delete segInfos;
    delete segStates;
    delete segStateLock;
    delete segmentBitmap;
}

TEST(SegmentCtx, ReleaseSegment_TestSimpleInterfaceFunc)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, nullptr, nullptr);

    EXPECT_CALL(*segStates, SetState(SegmentState::FREE));
    EXPECT_CALL(*segmentBitmap, ClearBit);

    // when
    segCtx.ReleaseSegment(0);

    delete segInfos;
    delete segStates;
    delete segStateLock;
    delete segmentBitmap;
}

TEST(SegmentCtx, AllocateFreeSegment_TestAllocSegmentWithCheckingConditions)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>[100]();
    SegmentStates* segStates = new SegmentStates[100]();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(100);
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, rebuildCtx, addrInfo);

    // given 1.
    EXPECT_CALL(*segmentBitmap, SetNextZeroBit).WillOnce(Return(22));
    EXPECT_CALL(*segmentBitmap, IsValidBit).WillOnce(Return(true));
    EXPECT_CALL(*rebuildCtx, IsRebuildTargetSegment(22)).WillOnce(Return(false));

    // when 1.
    int ret = segCtx.AllocateFreeSegment();
    // then 1.
    EXPECT_EQ(22, ret);
    EXPECT_EQ(segStates[22].GetState(), SegmentState::NVRAM);

    // given 2.
    EXPECT_CALL(*segmentBitmap, SetNextZeroBit).WillOnce(Return(33));
    EXPECT_CALL(*segmentBitmap, IsValidBit).WillOnce(Return(false));
    // when 2.
    ret = segCtx.AllocateFreeSegment();
    // then 2.
    EXPECT_EQ(UNMAP_SEGMENT, ret);

    // given 3.
    EXPECT_CALL(*segmentBitmap, SetNextZeroBit).WillOnce(Return(10));
    EXPECT_CALL(*segmentBitmap, SetFirstZeroBit).Times(0);
    EXPECT_CALL(*segmentBitmap, IsValidBit).WillOnce(Return(true));
    EXPECT_CALL(*rebuildCtx, IsRebuildTargetSegment(10)).WillOnce(Return(false));
    // when 3.
    ret = segCtx.AllocateFreeSegment();
    // then 3.
    EXPECT_EQ(10, ret);
    EXPECT_EQ(segStates[10].GetState(), SegmentState::NVRAM);

    // given 4.
    EXPECT_CALL(*segmentBitmap, SetNextZeroBit).WillOnce(Return(8));
    EXPECT_CALL(*segmentBitmap, SetFirstZeroBit).WillOnce(Return(UNMAP_SEGMENT));
    EXPECT_CALL(*segmentBitmap, IsValidBit).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*rebuildCtx, IsRebuildTargetSegment(8)).WillOnce(Return(true));
    // when 4.
    ret = segCtx.AllocateFreeSegment();
    // then 4.
    EXPECT_EQ(UNMAP_SEGMENT, ret);

    delete addrInfo;
    delete[] segInfos;
    delete[] segStates;
    delete segStateLock;
    delete segmentBitmap;
    delete rebuildCtx;
}

TEST(SegmentCtx, GetNumOfFreeSegment_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, nullptr, addrInfo);

    EXPECT_CALL(*segmentBitmap, GetNumBits).WillOnce(Return(10));
    EXPECT_CALL(*segmentBitmap, GetNumBitsSet).WillOnce(Return(3));
    int ret = segCtx.GetNumOfFreeSegment();
    EXPECT_EQ(7, ret);

    EXPECT_CALL(*segmentBitmap, GetNumBits).WillOnce(Return(10));
    EXPECT_CALL(*segmentBitmap, GetNumBitsSetWoLock).WillOnce(Return(3));
    ret = segCtx.GetNumOfFreeSegmentWoLock();
    EXPECT_EQ(7, ret);

    delete addrInfo;
    delete segInfos;
    delete segStates;
    delete segStateLock;
    delete segmentBitmap;
}

TEST(SegmentCtx, SetAllocatedSegmentCount_TestSimpleSEtter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, nullptr, addrInfo);

    EXPECT_CALL(*segmentBitmap, SetNumBitsSet);
    // when 1.
    segCtx.SetAllocatedSegmentCount(10);

    delete addrInfo;
    delete segInfos;
    delete segStates;
    delete segStateLock;
    delete segmentBitmap;
}

TEST(SegmentCtx, GetAllocatedSegmentCount_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, nullptr, addrInfo);

    EXPECT_CALL(*segmentBitmap, GetNumBitsSet).WillOnce(Return(10));
    // when
    int ret = segCtx.GetAllocatedSegmentCount();
    EXPECT_EQ(10, ret);

    delete addrInfo;
    delete segInfos;
    delete segStates;
    delete segStateLock;
    delete segmentBitmap;
}

TEST(SegmentCtx, GetTotalSegmentsCount_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(100);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, nullptr, addrInfo);

    EXPECT_CALL(*segmentBitmap, GetNumBits).WillOnce(Return(100));
    // when
    int ret = segCtx.GetTotalSegmentsCount();
    EXPECT_EQ(100, ret);

    delete addrInfo;
    delete segInfos;
    delete segStates;
    delete segStateLock;
    delete segmentBitmap;
}

TEST(SegmentCtx, GetUsedSegment_TestGetAllocatedSegment)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(100);
    SegmentCtx segCtx(nullptr, segInfos, segStates, segStateLock, segmentBitmap, nullptr, addrInfo);

    // given 1.
    EXPECT_CALL(*segmentBitmap, FindFirstSetBit).WillOnce(Return(5));
    EXPECT_CALL(*segmentBitmap, IsValidBit).WillOnce(Return(true));
    // when 1.
    int ret = segCtx.GetUsedSegment(0);
    // then 1.
    EXPECT_EQ(5, ret);

    // given 2.
    EXPECT_CALL(*segmentBitmap, FindFirstSetBit).WillOnce(Return(5));
    EXPECT_CALL(*segmentBitmap, IsValidBit).WillOnce(Return(false));
    // when 2.
    ret = segCtx.GetUsedSegment(0);
    // then 2.
    EXPECT_EQ(UNMAP_SEGMENT, ret);

    delete addrInfo;
    delete segInfos;
    delete segStates;
    delete segStateLock;
    delete segmentBitmap;
}

TEST(SegmentCtx, DISABLED_FindMostInvalidSSDSegment_testIfMostInvalidSegmentIsReturned)
{
}

TEST(SegmentCtx, GetRebuildTargetSegment_TestRebuildTargetFindFail)
{
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    SegmentCtx segmentCtx(nullptr, nullptr, rebuildCtx, nullptr);

    EXPECT_CALL(*rebuildCtx, GetRebuildTargetSegment).WillOnce(Return(UNMAP_SEGMENT));

    SegmentId ret = segmentCtx.GetRebuildTargetSegment();
    EXPECT_EQ(ret, UINT32_MAX);

    delete rebuildCtx;
}

TEST(SegmentCtx, GetRebuildTargetSegment_TestRebuildTargetIsAlreadyFree)
{
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    SegmentCtx segmentCtx(nullptr, nullptr, segStates, segStateLock, nullptr, rebuildCtx, nullptr);

    EXPECT_CALL(*rebuildCtx, GetRebuildTargetSegment).WillOnce(Return(0)).WillOnce(Return(UNMAP_SEGMENT));
    std::mutex lock;
    EXPECT_CALL(*segStateLock, GetLock).WillOnce(ReturnRef(lock));
    EXPECT_CALL(*segStates, GetState).WillOnce(Return(SegmentState::FREE));
    EXPECT_CALL(*rebuildCtx, EraseRebuildTargetSegment(0));

    SegmentId ret = segmentCtx.GetRebuildTargetSegment();
    EXPECT_EQ(ret, UINT32_MAX);

    delete rebuildCtx;
    delete segStates;
    delete segStateLock;
}

TEST(SegmentCtx, GetRebuildTargetSegment_TestRebuildTargetFindSuccess)
{
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segStateLock = new NiceMock<MockSegmentLock>();
    SegmentCtx segmentCtx(nullptr, nullptr, segStates, segStateLock, nullptr, rebuildCtx, nullptr);

    EXPECT_CALL(*rebuildCtx, GetRebuildTargetSegment).WillOnce(Return(0));
    std::mutex lock;
    EXPECT_CALL(*segStateLock, GetLock).WillOnce(ReturnRef(lock));
    EXPECT_CALL(*segStates, GetState).WillOnce(Return(SegmentState::SSD));

    SegmentId ret = segmentCtx.GetRebuildTargetSegment();
    EXPECT_EQ(ret, 0);

    delete rebuildCtx;
    delete segStates;
    delete segStateLock;
}

TEST(SegmentCtx, MakeRebuildTarget_TestMakeRebuildTarget)
{
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockBitMapMutex>* segmentBitmap = new NiceMock<MockBitMapMutex>(1);
    SegmentCtx segmentCtx(nullptr, nullptr, nullptr, nullptr, segmentBitmap, rebuildCtx, nullptr);

    EXPECT_CALL(*rebuildCtx, ClearRebuildTargetList);
    EXPECT_CALL(*segmentBitmap, FindFirstSetBit).WillOnce(Return(0)).WillOnce(Return(1));
    EXPECT_CALL(*segmentBitmap, IsValidBit).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*rebuildCtx, AddRebuildTargetSegment(0));

    int ret = segmentCtx.MakeRebuildTarget();
    EXPECT_EQ(ret, 1);

    delete rebuildCtx;
    delete segmentBitmap;
}
} // namespace pos
