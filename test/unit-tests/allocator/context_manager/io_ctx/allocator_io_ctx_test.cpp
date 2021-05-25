#include "src/allocator/context_manager/io_ctx/allocator_io_ctx.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(AllocatorIoCtx, AllocatorIoCtx_TestConstructor)
{
    AllocatorIoCtx* IoCtx = new AllocatorIoCtx(MetaFsIoOpcode::Write, 0, 0, 10, nullptr, nullptr);
    delete IoCtx;
}

} // namespace pos
