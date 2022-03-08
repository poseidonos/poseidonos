#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/context_manager/block_allocation_status_mock.h"

using testing::NiceMock;

namespace pos
{
TEST(GcCtx, GetCurrentGcMode_TestModeNoGC)
{
    // given
    NiceMock<MockBlockAllocationStatus> allocStatus;
    GcCtx gcCtx(&allocStatus);

    gcCtx.SetNormalGcThreshold(10);
    gcCtx.SetUrgentThreshold(5);
    gcCtx.GetCurrentGcMode(8);
    // when
    gcCtx.GetCurrentGcMode(13);
}

TEST(GcCtx, GetCurrentGcMode_TestGetCurrentGcMode_ByNumberOfFreeSegment)
{
    // given
    NiceMock<MockBlockAllocationStatus> blockAllocStatus;
    GcCtx* gcCtx = new GcCtx(&blockAllocStatus);

    gcCtx->SetNormalGcThreshold(10);
    gcCtx->SetUrgentThreshold(5);

    // when 1.
    GcMode ret = gcCtx->GetCurrentGcMode(11);

    // then 1.
    EXPECT_EQ(MODE_NO_GC, ret);

    // when 2.
    ret = gcCtx->GetCurrentGcMode(10);
    // then 2.
    EXPECT_EQ(MODE_NORMAL_GC, ret);

    // when 3.
    ret = gcCtx->GetCurrentGcMode(9);
    // then 3.
    EXPECT_EQ(MODE_NORMAL_GC, ret);

    EXPECT_CALL(blockAllocStatus, ProhibitUserBlockAllocation);
    // when 4.
    ret = gcCtx->GetCurrentGcMode(5);
    // then 4.
    EXPECT_EQ(MODE_URGENT_GC, ret);

    // when 5.
    ret = gcCtx->GetCurrentGcMode(4);
    // then 5.
    EXPECT_EQ(MODE_URGENT_GC, ret);

    EXPECT_CALL(blockAllocStatus, PermitUserBlockAllocation);
    // when 6.
    ret = gcCtx->GetCurrentGcMode(8);
    // then 6.
    EXPECT_EQ(MODE_NORMAL_GC, ret);
}

} // namespace pos
