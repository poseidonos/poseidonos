#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(GcCtx, GetCurrentGcMode_TestModeNoGC)
{
    // given
    GcCtx gcCtx;
    gcCtx.SetNormalGcThreshold(10);
    gcCtx.SetUrgentThreshold(5);
    gcCtx.GetCurrentGcMode(8);
    // when
    gcCtx.GetCurrentGcMode(13);
}

} // namespace pos
