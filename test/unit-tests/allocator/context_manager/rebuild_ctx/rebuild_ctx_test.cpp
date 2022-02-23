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
    addrInfo.SetnumUserAreaSegments(10);
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
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(5));

    RebuildCtx rebuildCtx(nullptr, nullptr, &addrInfo);
    rebuildCtx.Init();

    NiceMock<MockAllocatorFileIo> fileIo;
    rebuildCtx.SetAllocatorFileIo(&fileIo);
    EXPECT_CALL(fileIo, Flush).WillRepeatedly(Return(0));

    // Update rebuild segment list with flush request
    std::set<SegmentId> targetSegments = {0, 1, 2};
    rebuildCtx.FlushRebuildSegmentList(targetSegments);

    // when
    char buf[100];
    RebuildCtxHeader* header = reinterpret_cast<RebuildCtxHeader*>(buf);
    rebuildCtx.BeforeFlush(buf);

    // then
    EXPECT_EQ(3, header->numTargetSegments);
    int* segId = reinterpret_cast<int*>(buf + sizeof(RebuildCtxHeader));
    for (int i = 0; i < 3; i++)
    {
        EXPECT_EQ(i, segId[i]);
    }

    rebuildCtx.Dispose();
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

} // namespace pos
