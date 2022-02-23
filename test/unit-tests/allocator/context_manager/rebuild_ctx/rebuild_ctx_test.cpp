#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/pos_event_id.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_file_io_mock.h"

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
    RebuildCtx rebuildCtx(nullptr, &addrInfo);
    // when
    rebuildCtx.Init();
}

TEST(RebuildCtx, Close_TestCallEmptyFunc)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    // when
    rebuildCtx.Dispose();
}

TEST(RebuildCtx, GetRebuildTargetSegment_TestFailtoGetLockCase)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
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
}

TEST(RebuildCtx, ReleaseRebuildSegment_TestIfSuccessOrNot)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    rebuildCtx.AddRebuildTargetSegment(0);
    rebuildCtx.AddRebuildTargetSegment(1);        
    // Use another methods for UT function coverage
    rebuildCtx.EmplaceRebuildTargetSegment(2);
    rebuildCtx.SetTargetSegmentCnt(3);

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
}

TEST(RebuildCtx, NeedRebuildAgain_TestSimpleGetter)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    // when
    rebuildCtx.NeedRebuildAgain();
}

TEST(RebuildCtx, FreeSegmentInRebuildTarget_TestWithSegmentState)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    // when 1.
    int ret = rebuildCtx.FreeSegmentInRebuildTarget(0);
    // then 1.
    EXPECT_EQ(0, ret);
    // given
    rebuildCtx.AddRebuildTargetSegment(0);
    rebuildCtx.AddRebuildTargetSegment(1);
    rebuildCtx.AddRebuildTargetSegment(2);
    // when 2.
    ret = rebuildCtx.FreeSegmentInRebuildTarget(5);
    // then 1.
    EXPECT_EQ(0, ret);
    // given 2.
    SegmentId segmentId = rebuildCtx.GetRebuildTargetSegment();
    EXPECT_EQ(segmentId, 0);
    // when 2.
    ret = rebuildCtx.FreeSegmentInRebuildTarget(0);
    // then 2.
    EXPECT_EQ(0, ret);
    // when 3.
    ret = rebuildCtx.FreeSegmentInRebuildTarget(2);
    // then 2.
    EXPECT_EQ(1, ret);
}

TEST(RebuildCtx, IsRebuidTargetSegmentsEmpty_Test)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    rebuildCtx.AddRebuildTargetSegment(0);
    rebuildCtx.AddRebuildTargetSegment(1);
    rebuildCtx.AddRebuildTargetSegment(2);
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
}

TEST(RebuildCtx, GetRebuildTargetSegmentsBegin_TestSimpleGetter)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    rebuildCtx.AddRebuildTargetSegment(0);
    rebuildCtx.AddRebuildTargetSegment(1);
    rebuildCtx.AddRebuildTargetSegment(2);

    // when
    auto iter = rebuildCtx.GetRebuildTargetSegmentsBegin();
    EXPECT_EQ(*iter, 0);
}

TEST(RebuildCtx, RebuildTargetSegmentsEnd_TestSimpleGetter)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    rebuildCtx.AddRebuildTargetSegment(0);
    rebuildCtx.AddRebuildTargetSegment(1);
    rebuildCtx.AddRebuildTargetSegment(2);

    // when
    rebuildCtx.GetRebuildTargetSegmentsEnd();
}

TEST(RebuildCtx, GetRebuildTargetSegmentCount_TestSimpleGetter)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    rebuildCtx.AddRebuildTargetSegment(0);
    rebuildCtx.AddRebuildTargetSegment(1);
    rebuildCtx.AddRebuildTargetSegment(2);
    // when
    int ret = rebuildCtx.GetRebuildTargetSegmentCount();
    // then
    EXPECT_EQ(3, ret);
}

TEST(RebuildCtx, ClearRebuildTargetList_TestClear)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    rebuildCtx.AddRebuildTargetSegment(0);
    rebuildCtx.AddRebuildTargetSegment(1);
    rebuildCtx.AddRebuildTargetSegment(2);

    int ret = rebuildCtx.GetRebuildTargetSegmentCount();
    EXPECT_EQ(ret, 3);

    rebuildCtx.ClearRebuildTargetList();

    ret = rebuildCtx.GetRebuildTargetSegmentCount();
    EXPECT_EQ(ret, 0);
}

TEST(RebuildCtx, StopRebuilding_TestRetry)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    rebuildCtx.AddRebuildTargetSegment(0);
    rebuildCtx.AddRebuildTargetSegment(1);
    rebuildCtx.AddRebuildTargetSegment(2);
    // when 1.
    int ret = rebuildCtx.StopRebuilding();
    // then 1.
    EXPECT_EQ(1, ret);
    // when 2.
    ret = rebuildCtx.StopRebuilding();
    // then 2.
    EXPECT_EQ((int)-EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY), ret);
}

TEST(RebuildCtx, GetSectionSize_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    RebuildCtx rebuildCtx(nullptr, &addrInfo);
    // when 1.
    int ret = rebuildCtx.GetSectionSize(RC_HEADER);
    // then 1.
    EXPECT_EQ((int)sizeof(RebuildCtxHeader), ret);
    // when 2.
    ret = rebuildCtx.GetSectionSize(RC_REBUILD_SEGMENT_LIST);
    // then 2.
    EXPECT_EQ((int)sizeof(SegmentId) * 10, ret);
}

TEST(RebuildCtx, GetSectionAddr_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    RebuildCtx rebuildCtx(nullptr, nullptr);
    // when 1.
    char* ret = rebuildCtx.GetSectionAddr(RC_HEADER);
    // when 2.
    ret = rebuildCtx.GetSectionAddr(RC_REBUILD_SEGMENT_LIST);
}

TEST(RebuildCtx, GetStoredVersion_TestSimpleGetter)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    // when
    int ret = rebuildCtx.GetStoredVersion();
}

TEST(RebuildCtx, ResetDirtyVersion_TestSimpleSetter)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    // when
    rebuildCtx.ResetDirtyVersion();
}

TEST(RebuildCtx, IsRebuildTargetSegment_TestSimpleSetter)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    rebuildCtx.AddRebuildTargetSegment(0);
    rebuildCtx.AddRebuildTargetSegment(1);
    rebuildCtx.AddRebuildTargetSegment(2);
    // when 1.
    bool ret = rebuildCtx.IsRebuildTargetSegment(5);
    // then 1.
    EXPECT_EQ(false, ret);
    // when 2.
    ret = rebuildCtx.IsRebuildTargetSegment(0);
    // then 2.
    EXPECT_EQ(true, ret);
}

TEST(RebuildCtx, GetCtxLock_TestSimpleGetter)
{
    RebuildCtx rebuildCtx(nullptr, nullptr, nullptr);
    std::mutex& m = rebuildCtx.GetCtxLock();
}

TEST(RebuildCtx, FinalizeIo_TestSimpleSetter)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    AsyncMetaFileIoCtx ctx;
    char buf[100];
    ctx.buffer = buf;
    // when
    rebuildCtx.FinalizeIo(&ctx);
}

TEST(RebuildCtx, BeforeFlush_TestSimpleSetter)
{
    // given
    RebuildCtx rebuildCtx(nullptr, nullptr);
    rebuildCtx.AddRebuildTargetSegment(0);
    rebuildCtx.AddRebuildTargetSegment(1);
    rebuildCtx.AddRebuildTargetSegment(2);

    char buf[100];
    RebuildCtxHeader* header = reinterpret_cast<RebuildCtxHeader*>(buf);
    // when
    rebuildCtx.BeforeFlush(buf);
    // then
    EXPECT_EQ(3, header->numTargetSegments);
    int* segId = reinterpret_cast<int*>(buf + sizeof(RebuildCtxHeader));
    for (int i = 0; i < 3; i++)
    {
        EXPECT_EQ(i, segId[i]);
    }
}

TEST(RebuildCtx, AfterLoad_testIfFalseDataHandling)
{
    RebuildCtxHeader header;

    header.sig = RebuildCtx::SIG_REBUILD_CTX;
    header.numTargetSegments = 3;
    char buf[sizeof(RebuildCtxHeader) + 3 * sizeof(int)];
    char* buf2 = buf + sizeof(RebuildCtxHeader);
    for (int i = 0; i < 3; i++)
    {
        buf2[i] = 1;
    }
    RebuildCtx rebuildCtx(nullptr, &header, nullptr);
    // when 1.
    rebuildCtx.AfterLoad(buf);
}

TEST(RebuildCtx, AfterLoad_testIfSegmentSignatureSuccessAndSetBuf)
{
    // given
    RebuildCtxHeader header;

    header.sig = RebuildCtx::SIG_REBUILD_CTX;
    header.numTargetSegments = 3;
    char buf[sizeof(RebuildCtxHeader) + 3 * sizeof(int)];
    char* buf2 = buf + sizeof(RebuildCtxHeader);
    for (int i = 0; i < 3; i++)
    {
        buf2[i] = i;
    }
    RebuildCtx rebuildCtx(nullptr, &header, nullptr);
    // when 1.
    rebuildCtx.AfterLoad(buf);
}

TEST(RebuildCtx, AfterLoad_testIfStoredVersionIsUpdated)
{
    // given
    RebuildCtxHeader header;

    header.sig = RebuildCtx::SIG_REBUILD_CTX;
    header.numTargetSegments = 3;
    header.ctxVersion = 2;

    char buf[sizeof(RebuildCtxHeader) + 3 * sizeof(int)];
    char* buf2 = buf + sizeof(RebuildCtxHeader);
    for (int i = 0; i < 3; i++)
    {
        buf2[i] = i;
    }
    RebuildCtx rebuildCtx(nullptr, &header, nullptr);
    // when 1.
    rebuildCtx.AfterLoad(buf);

    EXPECT_EQ(rebuildCtx.GetStoredVersion(), header.ctxVersion);
}

TEST(RebuildCtx, GetLock_TestSimpleGetter)
{
    // given
    RebuildCtxHeader header;
    RebuildCtx rebuildCtx(nullptr, nullptr);
    // when
    rebuildCtx.GetLock();
}

} // namespace pos
