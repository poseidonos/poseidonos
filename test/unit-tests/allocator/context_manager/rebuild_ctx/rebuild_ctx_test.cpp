#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/pos_event_id.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(RebuildCtx, Init_TestSimpleSetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, &addrInfo);
    // when
    rebuildCtx.Init();
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, Close_TestCallEmptyFunc)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);
    // when
    rebuildCtx.Dispose();
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, GetRebuildTargetSegment_TestFailtoGetLockCase)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);
    // given 1.
    rebuildCtx.GetLock().lock();
    // when 1.
    int ret = rebuildCtx.GetRebuildTargetSegment();
    // then 1.
    EXPECT_EQ((int)UINT32_MAX, ret);
    // given 2.
    rebuildCtx.GetLock().unlock();
    // when 1.
    ret = rebuildCtx.GetRebuildTargetSegment();
    // then 1.
    EXPECT_EQ((int)UINT32_MAX, ret);
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, ReleaseRebuildSegment_TestIfSuccessOrNot)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);
    EXPECT_CALL(*allocCtx, GetUsedSegment).WillOnce(Return(0)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(UNMAP_SEGMENT));
    rebuildCtx.MakeRebuildTarget();
    // when 1.
    int ret = rebuildCtx.ReleaseRebuildSegment(10);
    // then 1.
    EXPECT_EQ(0, ret);
    // when 2.
    ret = rebuildCtx.ReleaseRebuildSegment(0);
    // then 2.
    EXPECT_EQ(1, ret);
    // given 3.
    rebuildCtx.GetLock().try_lock();
    // when 3.
    ret = rebuildCtx.ReleaseRebuildSegment(0);
    // then 3.
    EXPECT_EQ(0, ret);
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, NeedRebuildAgain_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);
    // when
    rebuildCtx.NeedRebuildAgain();
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, FreeSegmentInRebuildTarget_TestWithSegmentState)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);
    // when 1.
    int ret = rebuildCtx.FreeSegmentInRebuildTarget(0);
    // then 1.
    EXPECT_EQ(0, ret);
    // given
    EXPECT_CALL(*allocCtx, GetUsedSegment).WillOnce(Return(0)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(UNMAP_SEGMENT));
    rebuildCtx.MakeRebuildTarget();
    // when 2.
    ret = rebuildCtx.FreeSegmentInRebuildTarget(5);
    // then 1.
    EXPECT_EQ(0, ret);
    // given 2.
    EXPECT_CALL(*segmentCtx, GetSegmentState).WillOnce(Return(SegmentState::FREE)).WillOnce(Return(SegmentState::SSD));
    rebuildCtx.GetRebuildTargetSegment();
    // when 2.
    ret = rebuildCtx.FreeSegmentInRebuildTarget(1);
    // then 2.
    EXPECT_EQ(0, ret);
    // when 3.
    ret = rebuildCtx.FreeSegmentInRebuildTarget(2);
    // then 2.
    EXPECT_EQ(1, ret);
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, IsRebuidTargetSegmentsEmpty_Test)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);

    EXPECT_CALL(*allocCtx, GetUsedSegment).WillOnce(Return(0)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(UNMAP_SEGMENT));
    rebuildCtx.MakeRebuildTarget();
    // when 1.
    bool ret = rebuildCtx.IsRebuidTargetSegmentsEmpty();
    // then 1.
    EXPECT_EQ(false, ret);
    // given 2.
    rebuildCtx.StopRebuilding();
    // when 2.
    ret = rebuildCtx.IsRebuidTargetSegmentsEmpty();
    // then 2.
    EXPECT_EQ(true, ret);
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, GetRebuildTargetSegmentsBegin_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);

    EXPECT_CALL(*allocCtx, GetUsedSegment).WillOnce(Return(0)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(UNMAP_SEGMENT));
    rebuildCtx.MakeRebuildTarget();
    // when
    rebuildCtx.GetRebuildTargetSegmentsBegin();
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, RebuildTargetSegmentsEnd_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);

    EXPECT_CALL(*allocCtx, GetUsedSegment).WillOnce(Return(0)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(UNMAP_SEGMENT));
    rebuildCtx.MakeRebuildTarget();
    // when
    rebuildCtx.GetRebuildTargetSegmentsEnd();
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, GetRebuildTargetSegmentCount_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);

    EXPECT_CALL(*allocCtx, GetUsedSegment).WillOnce(Return(0)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(UNMAP_SEGMENT));
    rebuildCtx.MakeRebuildTarget();
    // when
    int ret = rebuildCtx.GetRebuildTargetSegmentCount();
    // then
    EXPECT_EQ(3, ret);
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, MakeRebuildTarget_TestRetry)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);

    EXPECT_CALL(*allocCtx, GetUsedSegment).WillOnce(Return(0)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(UNMAP_SEGMENT));
    // when 1.
    int ret = rebuildCtx.MakeRebuildTarget();
    // then 1.
    EXPECT_EQ(1, ret);
    // given 2.
    EXPECT_CALL(*allocCtx, GetUsedSegment).WillOnce(Return(0)).WillOnce(Return(1)).WillOnce(Return(UNMAP_SEGMENT));
    // when 2.
    ret = rebuildCtx.MakeRebuildTarget();
    // then 2.
    EXPECT_EQ(1, ret);
    // given 3.
    EXPECT_CALL(*allocCtx, GetUsedSegment).WillOnce(Return(1)).WillOnce(Return(1));
    // when 3.
    ret = rebuildCtx.MakeRebuildTarget();
    // then 3.
    EXPECT_EQ((int)-EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE), ret);
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, StopRebuilding_TestRetry)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);

    EXPECT_CALL(*allocCtx, GetUsedSegment).WillOnce(Return(0)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(UNMAP_SEGMENT));
    rebuildCtx.MakeRebuildTarget();
    // when 1.
    int ret = rebuildCtx.StopRebuilding();
    // then 1.
    EXPECT_EQ(1, ret);
    // when 2.
    ret = rebuildCtx.StopRebuilding();
    // then 2.
    EXPECT_EQ((int)-EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY), ret);
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, GetSectionSize_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, &addrInfo);
    // when 1.
    int ret = rebuildCtx.GetSectionSize(RC_HEADER);
    // then 1.
    EXPECT_EQ((int)sizeof(RebuildCtxHeader), ret);
    // when 2.
    ret = rebuildCtx.GetSectionSize(RC_REBUILD_SEGMENT_LIST);
    // then 2.
    EXPECT_EQ((int)sizeof(SegmentId) * 10, ret);
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, GetSectionAddr_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);
    // when 1.
    char* ret = rebuildCtx.GetSectionAddr(RC_HEADER);
    // when 2.
    ret = rebuildCtx.GetSectionAddr(RC_REBUILD_SEGMENT_LIST);
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, GetStoredVersion_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);
    // when
    int ret = rebuildCtx.GetStoredVersion();
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, ResetDirtyVersion_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);
    // when
    rebuildCtx.ResetDirtyVersion();
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, IsRebuildTargetSegment_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);
    EXPECT_CALL(*allocCtx, GetUsedSegment).WillOnce(Return(0)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(UNMAP_SEGMENT));
    rebuildCtx.MakeRebuildTarget();
    // when 1.
    bool ret = rebuildCtx.IsRebuildTargetSegment(5);
    // then 1.
    EXPECT_EQ(false, ret);
    // when 2.
    ret = rebuildCtx.IsRebuildTargetSegment(0);
    // then 2.
    EXPECT_EQ(true, ret);

    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, FinalizeIo_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);
    AsyncMetaFileIoCtx ctx;
    char buf[100];
    ctx.buffer = buf;
    // when
    rebuildCtx.FinalizeIo(&ctx);
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, BeforeFlush_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);
    EXPECT_CALL(*allocCtx, GetUsedSegment).WillOnce(Return(0)).WillOnce(Return(1)).WillOnce(Return(2)).WillOnce(Return(UNMAP_SEGMENT));
    rebuildCtx.MakeRebuildTarget();
    char buf[100];
    RebuildCtxHeader* header = reinterpret_cast<RebuildCtxHeader*>(buf);
    // when
    rebuildCtx.BeforeFlush(0, buf);
    // then
    EXPECT_EQ(3, header->numTargetSegments);
    int* segId = reinterpret_cast<int*>(buf + sizeof(RebuildCtxHeader));
    for (int i = 0; i < 3; i++)
    {
        EXPECT_EQ(i, segId[i]);
    }
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, AfterLoad_testIfFalseDataHandling)
{
    RebuildCtxHeader header;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();

    header.sig = RebuildCtx::SIG_REBUILD_CTX;
    header.numTargetSegments = 3;
    char buf[sizeof(RebuildCtxHeader) + 3 * sizeof(int)];
    char* buf2 = buf + sizeof(RebuildCtxHeader);
    for (int i = 0; i < 3; i++)
    {
        buf2[i] = 1;
    }
    RebuildCtx rebuildCtx(&header, allocCtx, segmentCtx, nullptr);
    // when 1.
    rebuildCtx.AfterLoad(buf);
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, AfterLoad_testIfSegmentSignatureSuccessAndSetBuf)
{
    // given
    RebuildCtxHeader header;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();

    header.sig = RebuildCtx::SIG_REBUILD_CTX;
    header.numTargetSegments = 3;
    char buf[sizeof(RebuildCtxHeader) + 3 * sizeof(int)];
    char* buf2 = buf + sizeof(RebuildCtxHeader);
    for (int i = 0; i < 3; i++)
    {
        buf2[i] = i;
    }
    RebuildCtx rebuildCtx(&header, allocCtx, segmentCtx, nullptr);
    // when 1.
    rebuildCtx.AfterLoad(buf);
    delete allocCtx;
    delete segmentCtx;
}

TEST(RebuildCtx, GetLock_TestSimpleGetter)
{
    // given
    RebuildCtxHeader header;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segmentCtx = new NiceMock<MockSegmentCtx>();
    RebuildCtx rebuildCtx(allocCtx, segmentCtx, nullptr);
    // when
    rebuildCtx.GetLock();
    delete allocCtx;
}

} // namespace pos
