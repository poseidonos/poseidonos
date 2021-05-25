#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

#include <gtest/gtest.h>

#include "src/allocator/include/allocator_const.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_info_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(SegmentCtx, AfterLoad_testIfSegmentSignatureIsMatchedOrNot)
{
    // given
    SegmentCtxHeader* buf = new SegmentCtxHeader();
    SegmentCtx segCtx(nullptr, nullptr, "");

    // given 1.
    buf->sig = SegmentCtx::SIG_SEGMENT_CTX;
    // when 1.
    segCtx.AfterLoad((char*)buf);

    // given 2.
    buf->sig = 0;
    // when 2.
    EXPECT_DEATH(segCtx.AfterLoad((char*)buf), "");

    delete buf;
}

TEST(SegmentCtx, BeforeFlush_TestSimpleSetter)
{
    // given
    SegmentCtxHeader* buf = new SegmentCtxHeader();
    SegmentCtx segCtx(nullptr, nullptr, "");
    buf->sig = SegmentCtx::SIG_SEGMENT_CTX;

    // when
    segCtx.BeforeFlush(SC_HEADER, (char*)buf);

    delete buf;
}

TEST(SegmentCtx, FinalizeIo_TestSimpleSetter)
{
    // given
    SegmentCtxHeader* buf = new SegmentCtxHeader();
    SegmentCtx segCtx(nullptr, nullptr, "");
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
    AllocatorAddressInfo addrInfo;
    addrInfo.SetblksPerSegment(100);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(segInfos, &addrInfo, "");

    // given 1.
    EXPECT_CALL(*segInfos, IncreaseValidBlockCount).WillOnce(Return(6));
    // when 1.
    int ret = segCtx.IncreaseValidBlockCount(0, 1);
    // then 1.
    EXPECT_EQ(6, ret);

    delete segInfos;
}

TEST(SegmentCtx, DecreaseValidBlockCount_TestDecreaseValue)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(segInfos, nullptr, "");

    // given 1.
    EXPECT_CALL(*segInfos, DecreaseValidBlockCount).WillOnce(Return(4));
    // when 1.
    int ret = segCtx.DecreaseValidBlockCount(0, 1);
    // then 1.
    EXPECT_EQ(4, ret);

    delete segInfos;
}

TEST(SegmentCtx, GetValidBlockCount_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(segInfos, nullptr, "");

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
    SegmentCtx segCtx(segInfos, nullptr, "");

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
    SegmentCtx segCtx(segInfos, nullptr, "");

    // given 1.
    EXPECT_CALL(*segInfos, IncreaseOccupiedStripeCount).WillOnce(Return(6));
    // when 1.
    int ret = segCtx.IncreaseOccupiedStripeCount(0);
    // then 1.
    EXPECT_EQ(6, ret);

    delete segInfos;
}

TEST(SegmentCtx, IsSegmentCtxIo_TestIfSegmentCtxIoOrNot)
{
    // given
    SegmentCtxHeader* buf = new SegmentCtxHeader();
    SegmentCtx segCtx(nullptr, nullptr, "");

    // given 1.
    buf->sig = SegmentCtx::SIG_SEGMENT_CTX;
    // when 1.
    bool isSegCtx = segCtx.IsSegmentCtxIo((char*)buf);
    // then 1.
    EXPECT_EQ(true, isSegCtx);

    // given 2.
    buf->sig = 0;
    // when 2.
    isSegCtx = segCtx.IsSegmentCtxIo((char*)buf);
    // then 2.
    EXPECT_EQ(false, isSegCtx);

    delete buf;
}

TEST(SegmentCtx, CopySegmentInfoToBufferforWBT_CheckCopiedBuffer)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(1);
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(segInfos, &addrInfo, "");

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
    SegmentCtx segCtx(nullptr, nullptr, "");

    // when
    std::mutex& m = segCtx.GetSegmentCtxLock();
}

TEST(SegmentCtx, Init_testInitAndClose)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    SegmentCtx segCtx(&addrInfo, "");

    // when
    segCtx.Init();

    // then
    segCtx.Close();
}

TEST(SegmentCtx, SetOccupiedStripeCount_TestSimpleSetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(segInfos, nullptr, "");

    EXPECT_CALL(*segInfos, SetOccupiedStripeCount(5));
    // when
    segCtx.SetOccupiedStripeCount(0, 5);

    delete segInfos;
}

TEST(SegmentCtx, GetSectionAddr_TestSimpleGetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(segInfos, nullptr, "");

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
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(segInfos, &addrInfo, "");

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
    SegmentCtx segCtx(segInfos, nullptr, "");

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
    SegmentCtx segCtx(segInfos, &addrInfo, "");
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
    SegmentCtx segCtx(segInfos, nullptr, "");
    // when
    segCtx.GetSegmentInfo();
}

TEST(SegmentCtx, ResetDirtyVersion_TestSimpleSetter)
{
    // given
    NiceMock<MockSegmentInfo>* segInfos = new NiceMock<MockSegmentInfo>();
    SegmentCtx segCtx(segInfos, nullptr, "");
    // when
    segCtx.ResetDirtyVersion();
}

} // namespace pos
