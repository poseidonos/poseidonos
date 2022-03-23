#include "src/allocator/context_manager/context_replayer.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/context_replayer_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"

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
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();

    ContextReplayer ctxReplayer(allocCtx, segCtx, nullptr);

    // given 1.
    EXPECT_CALL(*allocCtx, ResetDirtyVersion);
    // when 1.
    ctxReplayer.ResetDirtyContextVersion(ALLOCATOR_CTX);

    // given 2.
    EXPECT_CALL(*segCtx, ResetDirtyVersion);
    // when 2.
    ctxReplayer.ResetDirtyContextVersion(SEGMENT_CTX);

    // when 3.
    ctxReplayer.ResetDirtyContextVersion(11);

    delete allocCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ReplaySsdLsid_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();

    ContextReplayer ctxReplayer(allocCtx, segCtx, nullptr);
    EXPECT_CALL(*allocCtx, SetCurrentSsdLsid);

    // when
    ctxReplayer.ReplaySsdLsid(10);

    delete allocCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ReplaySegmentAllocation_TestAllocSegmentWithSegmentState)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();

    ContextReplayer ctxReplayer(allocCtx, segCtx, &addrInfo);

    // given 1.
    addrInfo.SetstripesPerSegment(100);
    EXPECT_CALL(*segCtx, GetSegmentState).WillOnce(Return(SegmentState::SSD));
    // when 1.
    ctxReplayer.ReplaySegmentAllocation(10);

    // given 2.
    addrInfo.SetstripesPerSegment(100);
    EXPECT_CALL(*segCtx, GetSegmentState).WillOnce(Return(SegmentState::FREE));
    EXPECT_CALL(*segCtx, AllocateSegment);

    // when 2.
    ctxReplayer.ReplaySegmentAllocation(10);

    delete allocCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ReplayStripeAllocation_TestSimpleSetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();

    ContextReplayer ctxReplayer(allocCtx, segCtx, &addrInfo);
    EXPECT_CALL(*allocCtx, AllocWbStripe(0));
    EXPECT_CALL(*segCtx, AllocateSegment(0));

    // when
    ctxReplayer.ReplayStripeAllocation(0, 0);

    delete allocCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ReplayStripeRelease_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();

    ContextReplayer ctxReplayer(allocCtx, segCtx, nullptr);
    EXPECT_CALL(*allocCtx, ReleaseWbStripe);

    // when
    ctxReplayer.ReplayStripeRelease(0);

    delete allocCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ReplayStripeFlushed_TestStripeFlushedWithSeveralConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();

    ContextReplayer ctxReplayer(allocCtx, segCtx, &addrInfo);

    // given
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount(1)).WillOnce(Return(false));
    // when
    ctxReplayer.ReplayStripeFlushed(10);

    delete allocCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ResetActiveStripeTail_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();

    ContextReplayer ctxReplayer(allocCtx, segCtx, nullptr);
    EXPECT_CALL(*allocCtx, SetActiveStripeTail);

    // when
    ctxReplayer.ResetActiveStripeTail(10);

    delete allocCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, GetAllActiveStripeTail_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();

    ContextReplayer ctxReplayer(allocCtx, segCtx, nullptr);
    EXPECT_CALL(*allocCtx, GetAllActiveStripeTail);

    // when
    ctxReplayer.GetAllActiveStripeTail();

    delete allocCtx;
    delete segCtx;
    delete reCtx;
}

TEST(ContextReplayer, ResetSegmentsStates_TestSimpleSetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(6);
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();

    ContextReplayer ctxReplayer(allocCtx, segCtx, &addrInfo);
    EXPECT_CALL(*segCtx, ResetSegmentsStates).Times(1);

    // when
    ctxReplayer.ResetSegmentsStates();

    delete allocCtx;
    delete segCtx;
    delete reCtx;
}

} // namespace pos
