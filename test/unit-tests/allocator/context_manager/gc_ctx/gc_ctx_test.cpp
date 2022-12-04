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
    GcCtx gcCtx(&allocStatus, 0);

    gcCtx.SetNormalGcThreshold(10);
    gcCtx.SetUrgentThreshold(5);

    // when 1
    uint32_t numFreeSegments = 8;
    GcMode gcMode = gcCtx.UpdateCurrentGcMode(numFreeSegments);

    // then 2
    EXPECT_EQ(gcMode, GcMode::MODE_NORMAL_GC);
    EXPECT_EQ(gcMode, gcCtx.GetCurrentGcMode());
}

TEST(GcCtx, UpdateCurrentGcMode_ByNumberOfFreeSegment)
{
    // given
    NiceMock<MockBlockAllocationStatus> blockAllocStatus;
    GcCtx* gcCtx = new GcCtx(&blockAllocStatus, 0);

    gcCtx->SetNormalGcThreshold(10);
    gcCtx->SetUrgentThreshold(5);

    // when 1.
    uint32_t numFreeSegments = 11;
    GcMode ret = gcCtx->UpdateCurrentGcMode(numFreeSegments);

    // then 1.
    EXPECT_EQ(MODE_NO_GC, ret);

    // when 2.
    numFreeSegments = 10;
    ret = gcCtx->UpdateCurrentGcMode(numFreeSegments);
    // then 2.
    EXPECT_EQ(MODE_NORMAL_GC, ret);

    // when 3.
    numFreeSegments = 9;
    ret = gcCtx->UpdateCurrentGcMode(numFreeSegments);
    // then 3.
    EXPECT_EQ(MODE_NORMAL_GC, ret);

    EXPECT_CALL(blockAllocStatus, ProhibitUserBlockAllocation);
    // when 4.
    numFreeSegments = 5;
    ret = gcCtx->UpdateCurrentGcMode(numFreeSegments);
    // then 4.
    EXPECT_EQ(MODE_URGENT_GC, ret);

    // when 5.
    numFreeSegments = 4;
    ret = gcCtx->UpdateCurrentGcMode(numFreeSegments);
    // then 5.
    EXPECT_EQ(MODE_URGENT_GC, ret);

    EXPECT_CALL(blockAllocStatus, PermitUserBlockAllocation);
    // when 6.
    numFreeSegments = 8;
    ret = gcCtx->UpdateCurrentGcMode(numFreeSegments);
    // then 6.
    EXPECT_EQ(MODE_NORMAL_GC, ret);
}

} // namespace pos
