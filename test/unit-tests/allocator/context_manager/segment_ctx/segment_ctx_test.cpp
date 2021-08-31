#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

#include <gtest/gtest.h>

#include "src/allocator/include/allocator_const.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_info_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_lock_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_states_mock.h"

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
    SegmentCtx segCtx(&header, nullptr, nullptr);
    // when
    segCtx.AfterLoad(nullptr);
}

TEST(SegmentCtx, BeforeFlush_TestSimpleSetter)
{
    // given
    SegmentCtxHeader* buf = new SegmentCtxHeader();
    SegmentCtx segCtx(nullptr, nullptr, nullptr);
    buf->sig = SegmentCtx::SIG_SEGMENT_CTX;

    // when
    segCtx.BeforeFlush(SC_HEADER, (char*)buf);

    delete buf;
}

TEST(SegmentCtx, FinalizeIo_TestSimpleSetter)
{
    // given
    SegmentCtx segCtx(nullptr, nullptr, nullptr);
    SegmentCtxHeader* buf = new SegmentCtxHeader();
    buf->sig = SegmentCtx::SIG_SEGMENT_CTX;
    AsyncMetaFileIoCtx ctx;
    ctx.buffer = (char*)buf;
    // when
    segCtx.FinalizeIo(&ctx);

    delete buf;
}

TEST(SegmentCtx, IncreaseValidBlockCount_TestIncreaseValue)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, addrInfo);

    // given 1.
    EXPECT_CALL(*segInfos, IncreaseValidBlockCount).WillOnce(Return(3));
    EXPECT_CALL(*addrInfo, GetblksPerSegment).WillOnce(Return(5));
    // when 1.
    int ret = segCtx.IncreaseValidBlockCount(0, 1);
    // then 1.
    EXPECT_EQ(3, ret);
    // given 2.
    EXPECT_CALL(*segInfos, IncreaseValidBlockCount).WillOnce(Return(6));
    EXPECT_CALL(*addrInfo, GetblksPerSegment).WillOnce(Return(5));
    EXPECT_CALL(*addrInfo, IsUT).WillOnce(Return(false)).WillOnce(Return(true));
    // when 2.
    ret = segCtx.IncreaseValidBlockCount(0, 1);

    delete segInfos;
    delete addrInfo;
}

TEST(SegmentCtx, DecreaseValidBlockCount_TestDecreaseValue)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, addrInfo);

    // given 1.
    EXPECT_CALL(*segInfos, DecreaseValidBlockCount).WillOnce(Return(4));
    // when 1.
    int ret = segCtx.DecreaseValidBlockCount(0, 1);
    // then 1.
    EXPECT_EQ(4, ret);
    // given 2.
    EXPECT_CALL(*segInfos, DecreaseValidBlockCount).WillOnce(Return(-1));
    EXPECT_CALL(*addrInfo, IsUT).WillOnce(Return(false)).WillOnce(Return(true));
    // when 2.
    ret = segCtx.DecreaseValidBlockCount(0, 1);

    delete segInfos;
    delete addrInfo;
}

TEST(SegmentCtx, GetValidBlockCount_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr);

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
    SegmentCtx segCtx(nullptr, segInfos, nullptr);

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
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr);

    // given 1.
    EXPECT_CALL(*segInfos, IncreaseOccupiedStripeCount).WillOnce(Return(6));
    // when 1.
    int ret = segCtx.IncreaseOccupiedStripeCount(0);
    // then 1.
    EXPECT_EQ(6, ret);

    delete segInfos;
}

TEST(SegmentCtx, GetSegmentState_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    SegmentCtx segCtx(nullptr, segInfos, segStates, segLocks, &addrInfo);

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

    delete segStates;
    delete segLocks;
}

TEST(SegmentCtx, SetSegmentState_TestSimpleSetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    SegmentCtx segCtx(nullptr, segInfos, segStates, segLocks, &addrInfo);

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

    delete segStates;
    delete segLocks;
}

TEST(SegmentCtx, GetSegStateLock_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    SegmentCtx allocCtx(nullptr, segInfos, segStates, segLocks, nullptr);

    std::mutex m;
    EXPECT_CALL(*segLocks, GetLock).WillOnce(ReturnRef(m));
    // when
    allocCtx.GetSegStateLock(0);

    delete segStates;
    delete segLocks;
}

TEST(SegmentCtx, CopySegmentInfoToBufferforWBT_CheckCopiedBuffer)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(1);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, &addrInfo);

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
    SegmentCtx segCtx(nullptr, nullptr, nullptr);

    // when
    std::mutex& m = segCtx.GetSegmentCtxLock();
}

TEST(SegmentCtx, Init_testInitAndClose)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    SegmentCtx segCtx(&addrInfo);

    // when
    segCtx.Init();

    // then
    segCtx.Dispose();
}

TEST(SegmentCtx, SetOccupiedStripeCount_TestSimpleSetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr);

    EXPECT_CALL(*segInfos, SetOccupiedStripeCount(5));
    // when
    segCtx.SetOccupiedStripeCount(0, 5);

    delete segInfos;
}

TEST(SegmentCtx, GetSectionAddr_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    SegmentCtx segCtx(nullptr, segInfos, segStates, segLocks, nullptr);

    // when 1.
    char* buf = segCtx.GetSectionAddr(SC_HEADER);

    // when 2.
    buf = segCtx.GetSectionAddr(SC_SEGMENT_INFO);
    EXPECT_EQ(reinterpret_cast<char*>(segInfos), buf);

    // when 3.
    buf = segCtx.GetSectionAddr(AC_SEGMENT_STATES);
    EXPECT_EQ(reinterpret_cast<char*>(segStates), buf);

    delete segInfos;
    delete segStates;
    delete segLocks;
}

TEST(SegmentCtx, GetSectionSize_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentStates>* segStates = new NiceMock<MockSegmentStates>();
    NiceMock<MockSegmentLock>* segLocks = new NiceMock<MockSegmentLock>();
    SegmentCtx segCtx(nullptr, segInfos, &addrInfo);

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

    delete segInfos;
    delete segStates;
    delete segLocks;
}

TEST(SegmentCtx, GetStoredVersion_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr);

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
    SegmentCtx segCtx(nullptr, segInfos, &addrInfo);
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
    SegmentCtx segCtx(nullptr, segInfos, nullptr);
    // when
    segCtx.GetSegmentInfo();
}

TEST(SegmentCtx, ResetDirtyVersion_TestSimpleSetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, segInfos, nullptr);
    // when
    segCtx.ResetDirtyVersion();
}

} // namespace pos
