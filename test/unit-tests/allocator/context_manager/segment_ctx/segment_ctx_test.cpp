#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

#include <gtest/gtest.h>

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

TEST(SegmentCtx, BeforeFlush_)
{
    // given
    SegmentCtxHeader* buf = new SegmentCtxHeader();
    SegmentCtx segCtx(nullptr, nullptr, "");
    buf->sig = SegmentCtx::SIG_SEGMENT_CTX;

    // when
    segCtx.BeforeFlush(SC_HEADER, (char*)buf);

    delete buf;
}

TEST(SegmentCtx, FinalizeIo_)
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

TEST(SegmentCtx, IncreaseValidBlockCount_)
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
}

TEST(SegmentCtx, DecreaseValidBlockCount_)
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

TEST(SegmentCtx, GetValidBlockCount_)
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

TEST(SegmentCtx, GetOccupiedStripeCount_)
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

TEST(SegmentCtx, IsSegmentCtxIo_)
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

TEST(SegmentCtx, CopySegmentInfoToBufferforWBT_)
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

} // namespace pos
