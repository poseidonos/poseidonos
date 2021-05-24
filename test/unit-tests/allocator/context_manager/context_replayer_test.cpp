#include "src/allocator/context_manager/context_replayer.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/context_replayer_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/wbstripe_ctx/wbstripe_ctx_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(ContextReplayer, ResetDirtyContextVersion_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);

    ContextReplayer ctxReplayer(allocCtx, segCtx, wbStripeCtx, nullptr);

    // given 1.
    EXPECT_CALL(*allocCtx, ResetDirtyVersion);
    // when 1.
    ctxReplayer.ResetDirtyContextVersion(ALLOCATOR_CTX);

    // given 2.
    EXPECT_CALL(*segCtx, ResetDirtyVersion);
    // when 2.
    ctxReplayer.ResetDirtyContextVersion(SEGMENT_CTX);

    delete allocCtx;
    delete wbStripeCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ReplaySsdLsid_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);

    ContextReplayer ctxReplayer(allocCtx, segCtx, wbStripeCtx, nullptr);
    EXPECT_CALL(*allocCtx, SetCurrentSsdLsid);

    // when
    ctxReplayer.ReplaySsdLsid(10);

    delete allocCtx;
    delete wbStripeCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ReplaySegmentAllocation_TestAllocSegmentWithSegmentState)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);

    ContextReplayer ctxReplayer(allocCtx, segCtx, wbStripeCtx, &addrInfo);

    // given 1.
    addrInfo.SetstripesPerSegment(100);
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::SSD));
    // when 1.
    ctxReplayer.ReplaySegmentAllocation(10);

    // given 2.
    addrInfo.SetstripesPerSegment(100);
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::FREE));
    EXPECT_CALL(*allocCtx, SetSegmentState);
    EXPECT_CALL(*allocCtx, AllocateSegment);

    // when 2.
    ctxReplayer.ReplaySegmentAllocation(10);

    delete allocCtx;
    delete wbStripeCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ReplayStripeAllocation_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);

    ContextReplayer ctxReplayer(allocCtx, segCtx, wbStripeCtx, nullptr);
    EXPECT_CALL(*wbStripeCtx, AllocWbStripe);

    // when
    ctxReplayer.ReplayStripeAllocation(0);

    delete allocCtx;
    delete wbStripeCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ReplayStripeFlushed_TestStripeFlushedWithSeveralConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);

    ContextReplayer ctxReplayer(allocCtx, segCtx, wbStripeCtx, &addrInfo);
    EXPECT_CALL(*wbStripeCtx, ReleaseWbStripe);

    // given 1.
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(3));
    // when
    ctxReplayer.ReplayStripeFlushed(50, 10);

    // given 2.
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(10));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(1));
    EXPECT_CALL(*allocCtx, SetSegmentState);
    EXPECT_CALL(*allocCtx, GetSegmentState).Times(0);
    // when
    ctxReplayer.ReplayStripeFlushed(50, 10);

    // given 3.
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(10));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::SSD));
    // when
    ctxReplayer.ReplayStripeFlushed(50, 10);

    // given 4.
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(10));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::FREE));
    EXPECT_CALL(*segCtx, SetOccupiedStripeCount);
    EXPECT_CALL(*allocCtx, SetSegmentState);
    EXPECT_CALL(*allocCtx, ReleaseSegment);
    // when
    ctxReplayer.ReplayStripeFlushed(50, 10);

    delete allocCtx;
    delete wbStripeCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ResetActiveStripeTail_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);

    ContextReplayer ctxReplayer(allocCtx, segCtx, wbStripeCtx, nullptr);
    EXPECT_CALL(*wbStripeCtx, SetActiveStripeTail);

    // when
    ctxReplayer.ResetActiveStripeTail(10);

    delete allocCtx;
    delete wbStripeCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, GetAllActiveStripeTail_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);

    ContextReplayer ctxReplayer(allocCtx, segCtx, wbStripeCtx, nullptr);
    EXPECT_CALL(*wbStripeCtx, GetAllActiveStripeTail);

    // when
    ctxReplayer.GetAllActiveStripeTail();

    delete allocCtx;
    delete wbStripeCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ResetSegmentsStates_TestSimpleSetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(6);
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);

    ContextReplayer ctxReplayer(allocCtx, segCtx, wbStripeCtx, &addrInfo);
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::VICTIM)).WillOnce(Return(SegmentState::VICTIM)).WillOnce(Return(SegmentState::FREE)).WillOnce(Return(SegmentState::NVRAM));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(0)).WillOnce(Return(5)).WillOnce(Return(0)).WillOnce(Return(5)).WillOnce(Return(0)).WillOnce(Return(5));

    // when
    ctxReplayer.ResetSegmentsStates();

    delete allocCtx;
    delete wbStripeCtx;
    delete segCtx;
    delete reCtx;
}

} // namespace pos
