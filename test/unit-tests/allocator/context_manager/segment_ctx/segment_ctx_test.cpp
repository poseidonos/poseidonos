#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

#include <gtest/gtest.h>

#include "src/allocator/include/allocator_const.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_info_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_list_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

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
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    SegmentCtxHeader header;
    header.sig = SegmentCtx::SIG_SEGMENT_CTX;
    header.ctxVersion = 5;
    header.numValidSegment = 100;

    NiceMock<MockSegmentList> freeSegmentList;
    SegmentCtx segCtx(nullptr, &header, nullptr, &freeSegmentList, nullptr, &addrInfo);

    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(0));

    // when
    segCtx.AfterLoad(nullptr);

    EXPECT_EQ(segCtx.GetStoredVersion(), header.ctxVersion);
}

TEST(SegmentCtx, AfterLoad_testIfSegmentListIsRebuilt)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    SegmentCtxHeader header;
    header.sig = SegmentCtx::SIG_SEGMENT_CTX;
    SegmentInfo* segInfos = new SegmentInfo[4](0, 0, SegmentState::FREE);

    NiceMock<MockSegmentList> freeSegmentList;
    SegmentCtx segCtx(nullptr, &header, segInfos, &freeSegmentList, nullptr, &addrInfo);

    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(4));
    EXPECT_CALL(freeSegmentList, AddToList).Times(4);

    // when
    segCtx.AfterLoad(nullptr);

    EXPECT_EQ(segCtx.GetStoredVersion(), header.ctxVersion);

    delete[] segInfos;
}

TEST(SegmentCtx, BeforeFlush_TestSimpleSetter)
{
    // given
    SegmentCtxHeader* buf = new SegmentCtxHeader();
    SegmentCtx segCtx(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    buf->sig = SegmentCtx::SIG_SEGMENT_CTX;

    // when
    segCtx.BeforeFlush((char*)buf);

    delete buf;
}

TEST(SegmentCtx, GetCtxLock_TestSimpleGetter)
{
    SegmentCtx segCtx(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    std::mutex& m = segCtx.GetCtxLock();
}

TEST(SegmentCtx, FinalizeIo_TestSimpleSetter)
{
    // given
    SegmentCtx segCtx(nullptr, nullptr, nullptr, nullptr, nullptr);
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
    SegmentCtx segCtx(nullptr, nullptr, segInfos, nullptr, &addrInfo);

    EXPECT_CALL(*segInfos, IncreaseOccupiedStripeCount).WillOnce(Return(1));

    // when
    bool segmentFreed = segCtx.IncreaseOccupiedStripeCount(0);
    // then
    EXPECT_EQ(segmentFreed, false);

    delete segInfos;
}

TEST(SegmentCtx, IncreaseOccupiedStripeCount_IfOccupiedStripeCountIsMax)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    addrInfo.SetnumUserAreaSegments(1);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentList> freeSegmentList;
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, &freeSegmentList, nullptr, &addrInfo);

    // when
    EXPECT_CALL(*segInfos, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*segInfos, MoveToSsdStateOrFreeStateIfItBecomesEmpty).WillOnce(Return(true));
    EXPECT_CALL(freeSegmentList, AddToList).Times(1);
    bool segmentFreed = segCtx.IncreaseOccupiedStripeCount(0);
    // then
    EXPECT_EQ(segmentFreed, true);

    delete segInfos;
    delete tp;
}

TEST(SegmentCtx, IncreaseValidBlockCount_TestIncreaseValue)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, nullptr, segInfos, nullptr, &addrInfo);

    // given 1.
    EXPECT_CALL(*segInfos, IncreaseValidBlockCount).WillOnce(Return(3));
    EXPECT_CALL(addrInfo, GetblksPerSegment).WillOnce(Return(5));
    // when 1.
    segCtx.IncreaseValidBlockCount(0, 1);

    delete segInfos;
}

TEST(SegmentCtx, DecreaseValidBlockCount_TestDecreaseValue)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, nullptr, segInfos, nullptr, addrInfo);

    // given 1.
    EXPECT_CALL(*segInfos, DecreaseValidBlockCount).WillOnce(Return(4));
    // when 1.
    bool ret = segCtx.DecreaseValidBlockCount(0, 1);
    // then 1.
    EXPECT_EQ(false, ret);

    delete addrInfo;
    delete segInfos;
}

TEST(SegmentCtx, DecreaseValidBlockCount_TestDecreaseValueWhenSegmentIsNotZero)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentList> freeSegmentList;
    SegmentCtx segCtx(nullptr, nullptr, segInfos, &freeSegmentList, nullptr, addrInfo);

    // given 1.
    EXPECT_CALL(*segInfos, DecreaseValidBlockCount).WillOnce(Return(10));

    // when 1.
    bool ret = segCtx.DecreaseValidBlockCount(0, 1);
    EXPECT_EQ(false, ret);

    delete addrInfo;
    delete segInfos;
}

TEST(SegmentCtx, DecreaseValidBlockCount_TestDecreaseValueWhenValidCountZeroAndSSDState)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentList> freeSegmentList;
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, &freeSegmentList, nullptr, addrInfo);

    // given 1.
    EXPECT_CALL(*segInfos, DecreaseValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(freeSegmentList, AddToList).Times(1);

    // when 1.
    bool ret = segCtx.DecreaseValidBlockCount(0, 1);
    EXPECT_EQ(true, ret);

    delete addrInfo;
    delete segInfos;
    delete tp;
}

TEST(SegmentCtx, GetValidBlockCount_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, nullptr, segInfos, nullptr, nullptr);

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
    SegmentCtx segCtx(nullptr, nullptr, segInfos, nullptr, nullptr);

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
    SegmentCtx segCtx(nullptr, nullptr, segInfos, nullptr, &addrInfo);

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
    NiceMock<MockSegmentList> freeSegmentList;
    SegmentCtx segCtx(nullptr, nullptr, segInfos, &freeSegmentList, nullptr, &addrInfo);

    EXPECT_CALL(*segInfos, GetState).WillOnce(Return(SegmentState::FREE));
    // when
    SegmentState ret = segCtx.GetSegmentState(0);
    // then
    EXPECT_EQ(SegmentState::FREE, ret);

    delete segInfos;
}

TEST(SegmentCtx, CopySegmentInfoToBufferforWBT_CheckCopiedBuffer)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(1);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, nullptr, segInfos, nullptr, &addrInfo);

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

TEST(SegmentCtx, Init_testInitAndClose)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    SegmentCtx segCtx(nullptr, nullptr, &addrInfo);

    // when
    segCtx.Init();

    // then
    segCtx.Dispose();
}

TEST(SegmentCtx, GetSectionAddr_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, nullptr, segInfos, nullptr, nullptr);

    // when 1.
    char* buf = segCtx.GetSectionAddr(SC_HEADER);

    // when 2.
    buf = segCtx.GetSectionAddr(SC_SEGMENT_INFO);
    EXPECT_EQ(reinterpret_cast<char*>(segInfos), buf);

    delete segInfos;
}

TEST(SegmentCtx, GetSectionSize_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    addrInfo.SetnumUserAreaSegments(10);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, nullptr, segInfos, nullptr, &addrInfo);

    // when 1.
    int ret = segCtx.GetSectionSize(SC_HEADER);
    // then 1.
    EXPECT_EQ(sizeof(SegmentCtxHeader), ret);

    // when 2.
    ret = segCtx.GetSectionSize(SC_SEGMENT_INFO);
    EXPECT_EQ(10 * sizeof(SegmentInfo), ret);

    delete segInfos;
}

TEST(SegmentCtx, GetStoredVersion_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, nullptr, segInfos, nullptr, nullptr);

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
    SegmentCtx segCtx(nullptr, nullptr, segInfos, nullptr, &addrInfo);
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

TEST(SegmentCtx, ResetDirtyVersion_TestSimpleSetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(nullptr, nullptr, segInfos, nullptr, nullptr);
    // when
    segCtx.ResetDirtyVersion();
}

TEST(SegmentCtx, AllocateSegment_TestSimpleInterfaceFunc)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, nullptr, nullptr);

    EXPECT_CALL(*segInfos, MoveToNvramState);
    // when
    segCtx.AllocateSegment(0);

    delete segInfos;
    delete tp;
}

TEST(SegmentCtx, AllocateFreeSegment_testWhenFreeListIsEmpty)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    SegmentInfo* segInfos = new SegmentInfo[100]();
    NiceMock<MockSegmentList> freeSegmentList;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, &freeSegmentList, rebuildCtx, addrInfo);

    EXPECT_CALL(freeSegmentList, PopSegment).WillOnce(Return(UNMAP_SEGMENT));
    int ret = segCtx.AllocateFreeSegment();
    EXPECT_EQ(UNMAP_SEGMENT, ret);

    delete addrInfo;
    delete[] segInfos;
    delete rebuildCtx;
    delete tp;
}

TEST(SegmentCtx, AllocateFreeSegment_testWhenAllocatedSegmentIsRebuildTarget)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    SegmentInfo* segInfos = new SegmentInfo[100]();
    NiceMock<MockSegmentList> freeSegmentList;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, &freeSegmentList, rebuildCtx, addrInfo);

    EXPECT_CALL(freeSegmentList, PopSegment).WillOnce(Return(10)).WillOnce(Return(11));
    EXPECT_CALL(*rebuildCtx, IsRebuildTargetSegment(10)).WillOnce(Return(true));
    EXPECT_CALL(*rebuildCtx, IsRebuildTargetSegment(11)).WillOnce(Return(false));
    EXPECT_CALL(freeSegmentList, AddToList(10)).Times(1);

    int ret = segCtx.AllocateFreeSegment();
    EXPECT_EQ(11, ret);

    delete addrInfo;
    delete[] segInfos;
    delete rebuildCtx;
    delete tp;
}

TEST(SegmentCtx, AllocateFreeSegment_testWhenSegmentIsAllocated)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    SegmentInfo* segInfos = new SegmentInfo[100]();
    NiceMock<MockSegmentList> freeSegmentList;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, &freeSegmentList, rebuildCtx, addrInfo);

    EXPECT_CALL(freeSegmentList, PopSegment).WillOnce(Return(8));
    EXPECT_CALL(*rebuildCtx, IsRebuildTargetSegment(8)).WillOnce(Return(false));

    EXPECT_EQ(segInfos[8].GetState(), SegmentState::FREE);

    int ret = segCtx.AllocateFreeSegment();
    EXPECT_EQ(8, ret);
    EXPECT_EQ(segInfos[8].GetState(), SegmentState::NVRAM);

    delete addrInfo;
    delete[] segInfos;
    delete rebuildCtx;
    delete tp;
}

TEST(SegmentCtx, AllocateGCVictimSegment_testWhenVictimSegmentIsFound)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    SegmentInfo* segInfos = new SegmentInfo[4](0, 0, SegmentState::SSD);
    NiceMock<MockSegmentList> freeSegmentList;
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, &freeSegmentList, nullptr, addrInfo);

    EXPECT_CALL(*addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(4));
    EXPECT_CALL(*addrInfo, GetblksPerSegment).WillRepeatedly(Return(10));

    segInfos[0].SetValidBlockCount(10);
    segInfos[1].SetValidBlockCount(6);
    segInfos[2].SetValidBlockCount(4);
    segInfos[3].SetValidBlockCount(9);

    SegmentId victimSegment = segCtx.AllocateGCVictimSegment();
    EXPECT_EQ(victimSegment, 2);

    delete addrInfo;
    delete[] segInfos;
    delete tp;
}

TEST(SegmentCtx, AllocateGCVictimSegment_testWhenVictimSegmentIsNotFound)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    SegmentInfo* segInfos = new SegmentInfo[4](10, 0, SegmentState::NVRAM);
    NiceMock<MockSegmentList> freeSegmentList;
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, &freeSegmentList, nullptr, addrInfo);

    EXPECT_CALL(*addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(4));
    EXPECT_CALL(*addrInfo, GetblksPerSegment).WillRepeatedly(Return(10));

    SegmentId victimSegment = segCtx.AllocateGCVictimSegment();
    EXPECT_EQ(victimSegment, UNMAP_SEGMENT);

    delete addrInfo;
    delete[] segInfos;
    delete tp;
}

TEST(SegmentCtx, ResetSegmentState_testIfSegmentStateChangedAsIntended)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockSegmentList> freeSegmentList;
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(1));

    {
        SegmentInfo segInfos(100, 10, SegmentState::VICTIM);
        SegmentCtx segCtx(nullptr, nullptr, &segInfos, &freeSegmentList, nullptr, &addrInfo);

        segCtx.ResetSegmentsStates();
        EXPECT_EQ(segInfos.GetState(), SegmentState::SSD);
    }
    {
        SegmentInfo segInfos(100, 10, SegmentState::SSD);
        SegmentCtx segCtx(nullptr, nullptr, &segInfos, &freeSegmentList, nullptr, &addrInfo);

        segCtx.ResetSegmentsStates();
        EXPECT_EQ(segInfos.GetState(), SegmentState::SSD);
    }
    {
        SegmentInfo segInfos(0, 10, SegmentState::SSD);
        SegmentCtx segCtx(nullptr, nullptr, &segInfos, &freeSegmentList, nullptr, &addrInfo);

        segCtx.ResetSegmentsStates();
        EXPECT_EQ(segInfos.GetState(), SegmentState::FREE);
    }
    {
        SegmentInfo segInfos(0, 0, SegmentState::FREE);
        SegmentCtx segCtx(nullptr, nullptr, &segInfos, &freeSegmentList, nullptr, &addrInfo);

        segCtx.ResetSegmentsStates();
        EXPECT_EQ(segInfos.GetState(), SegmentState::FREE);
    }
}

TEST(SegmentCtx, GetNumOfFreeSegment_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentList> freeSegmentList;
    SegmentCtx segCtx(nullptr, nullptr, segInfos, &freeSegmentList, nullptr, addrInfo);

    EXPECT_CALL(freeSegmentList, GetNumSegments).WillOnce(Return(7));
    int ret = segCtx.GetNumOfFreeSegment();
    EXPECT_EQ(7, ret);

    EXPECT_CALL(freeSegmentList, GetNumSegmentsWoLock).WillOnce(Return(7));
    ret = segCtx.GetNumOfFreeSegmentWoLock();
    EXPECT_EQ(7, ret);

    delete addrInfo;
    delete segInfos;
}

TEST(SegmentCtx, GetAllocatedSegmentCount_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    NiceMock<MockSegmentList> freeSegmentList;
    SegmentCtx segCtx(nullptr, nullptr, segInfos, &freeSegmentList, nullptr, addrInfo);

    EXPECT_CALL(*addrInfo, GetnumUserAreaSegments).WillOnce(Return(100));
    EXPECT_CALL(freeSegmentList, GetNumSegments).WillOnce(Return(10));

    // when
    int ret = segCtx.GetAllocatedSegmentCount();
    EXPECT_EQ(90, ret);

    delete addrInfo;
    delete segInfos;
}

TEST(SegmentCtx, GetRebuildTargetSegment_TestRebuildTargetFindFail)
{
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    SegmentCtx segmentCtx(nullptr, nullptr, nullptr, rebuildCtx, nullptr);

    EXPECT_CALL(*rebuildCtx, GetRebuildTargetSegment).WillOnce(Return(UNMAP_SEGMENT));

    SegmentId ret = segmentCtx.GetRebuildTargetSegment();
    EXPECT_EQ(ret, UINT32_MAX);

    delete rebuildCtx;
}

TEST(SegmentCtx, GetRebuildTargetSegment_TestRebuildTargetIsAlreadyFree)
{
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segmentCtx(nullptr, nullptr, segInfos, nullptr, rebuildCtx, nullptr);

    EXPECT_CALL(*rebuildCtx, GetRebuildTargetSegment).WillOnce(Return(0)).WillOnce(Return(UNMAP_SEGMENT));
    EXPECT_CALL(*segInfos, GetState).WillOnce(Return(SegmentState::FREE));
    EXPECT_CALL(*rebuildCtx, EraseRebuildTargetSegment(0));

    SegmentId ret = segmentCtx.GetRebuildTargetSegment();
    EXPECT_EQ(ret, UINT32_MAX);

    delete segInfos;
    delete rebuildCtx;
}

TEST(SegmentCtx, GetRebuildTargetSegment_TestRebuildTargetFindSuccess)
{
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segmentCtx(nullptr, nullptr, segInfos, nullptr, rebuildCtx, nullptr);

    EXPECT_CALL(*rebuildCtx, GetRebuildTargetSegment).WillOnce(Return(0));
    EXPECT_CALL(*segInfos, GetState).WillOnce(Return(SegmentState::SSD));

    SegmentId ret = segmentCtx.GetRebuildTargetSegment();
    EXPECT_EQ(ret, 0);

    delete segInfos;
    delete rebuildCtx;
}

TEST(SegmentCtx, MakeRebuildTarget_TestMakeRebuildTarget)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    SegmentInfo* segInfos = new SegmentInfo[4](0, 0, SegmentState::SSD);
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segmentCtx(tp, nullptr, segInfos, nullptr, rebuildCtx, &addrInfo);

    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(4));

    EXPECT_CALL(*rebuildCtx, InitializeTargetSegmentList)
        .WillOnce([&](std::set<SegmentId>& usedSegmentList)
        {
            EXPECT_TRUE(usedSegmentList.size() == 4);
            for (int i = 0; i < 4; i++)
            {
                EXPECT_TRUE(usedSegmentList.find(i) != usedSegmentList.end());
            }
            return 0;
        });

    int ret = segmentCtx.MakeRebuildTarget();
    EXPECT_EQ(ret, 0);

    delete rebuildCtx;
    delete tp;
}
} // namespace pos
