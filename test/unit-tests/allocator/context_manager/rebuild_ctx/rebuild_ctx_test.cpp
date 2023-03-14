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
void
InitializeRebuildCtxHeader(char* buf, int numSegments = 0, uint64_t version = 0)
{
    RebuildCtxHeader* headerPtr = reinterpret_cast<RebuildCtxHeader*>(buf);
    headerPtr->sig = SIG_REBUILD_CTX;
    headerPtr->numTargetSegments = numSegments;
    headerPtr->ctxVersion = version;

    SegmentId* segmentListPtr = reinterpret_cast<SegmentId*>(buf + sizeof(RebuildCtxHeader));
    for (int segmentId = 0; segmentId < numSegments; segmentId++)
    {
        *(segmentListPtr + segmentId) = segmentId;
    }
}

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

TEST(RebuildCtx, GetSectionInfo_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    RebuildCtx rebuildCtx(nullptr, &addrInfo);
    rebuildCtx.Init();

    uint64_t expectedOffset = 0;
    // when 1.
    auto ret = rebuildCtx.GetSectionInfo(RC_HEADER);
    // then 1.
    EXPECT_EQ(expectedOffset, ret.offset);
    EXPECT_EQ(sizeof(RebuildCtxHeader), ret.size);
    expectedOffset += sizeof(RebuildCtxHeader);
    
    // when 2.
    ret = rebuildCtx.GetSectionInfo(RC_REBUILD_SEGMENT_LIST);
    // then 2.
    EXPECT_EQ(expectedOffset, ret.offset);
    EXPECT_EQ((int)sizeof(SegmentId) * 10, ret.size);
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

TEST(RebuildCtx, AfterFlush_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    RebuildCtx rebuildCtx(nullptr, &addrInfo);
    AsyncMetaFileIoCtx ctx;
    char buf[rebuildCtx.GetTotalDataSize()];
    InitializeRebuildCtxHeader(buf);
    ctx.SetIoInfo(MetaFsIoOpcode::Write, 0, sizeof(buf), buf);
    // when
    rebuildCtx.AfterFlush(ctx.GetBuffer());
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
    char buf[rebuildCtx.GetTotalDataSize()];
    InitializeRebuildCtxHeader(buf);

    rebuildCtx.BeforeFlush(buf);

    // then
    RebuildCtxHeader* headerPtr = reinterpret_cast<RebuildCtxHeader*>(buf);
    EXPECT_EQ(3, headerPtr->numTargetSegments);
    SegmentId* segIdPtr = reinterpret_cast<SegmentId*>(buf + sizeof(RebuildCtxHeader));
    for (int i = 0; i < 3; i++)
    {
        EXPECT_EQ(i, segIdPtr[i]);
    }

    rebuildCtx.Dispose();
}

TEST(RebuildCtx, AfterLoad_testIfFalseDataHandling)
{
    RebuildCtxHeader header;
    header.numTargetSegments = 3;

    NiceMock<MockAllocatorAddressInfo> addrInfo;
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(5));

    RebuildCtx rebuildCtx(nullptr, &header, &addrInfo);
    rebuildCtx.Init();

    char buf[rebuildCtx.GetTotalDataSize()];
    InitializeRebuildCtxHeader(buf, 3);

    // when 1.
    rebuildCtx.AfterLoad(buf);
}

TEST(RebuildCtx, AfterLoad_testIfSegmentSignatureSuccessAndSetBuf)
{
    // given
    RebuildCtxHeader header;
    header.numTargetSegments = 3;

    NiceMock<MockAllocatorAddressInfo> addrInfo;
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(5));

    RebuildCtx rebuildCtx(nullptr, &header, &addrInfo);
    rebuildCtx.Init();

    char buf[rebuildCtx.GetTotalDataSize()];
    InitializeRebuildCtxHeader(buf, 3);

    // when 1.
    rebuildCtx.AfterLoad(buf);
}

TEST(RebuildCtx, AfterLoad_testIfStoredVersionIsUpdated)
{
    // given
    RebuildCtxHeader header;
    header.numTargetSegments = 3;
    header.ctxVersion = 2;

    NiceMock<MockAllocatorAddressInfo> addrInfo;
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(5));

    RebuildCtx rebuildCtx(nullptr, &header, &addrInfo);
    rebuildCtx.Init();

    char buf[rebuildCtx.GetTotalDataSize()];
    InitializeRebuildCtxHeader(buf, 3, header.ctxVersion + 1);

    // when 1.
    rebuildCtx.AfterLoad(buf);

    EXPECT_EQ(rebuildCtx.GetStoredVersion(), header.ctxVersion + 1);
}

} // namespace pos
