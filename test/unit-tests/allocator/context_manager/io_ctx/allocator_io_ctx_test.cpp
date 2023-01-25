#include "src/allocator/context_manager/io_ctx/allocator_io_ctx.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(AllocatorIoCtx, AllocatorIoCtx_TestConstructor)
{
    auto testCallback = [](){};
    AllocatorIoCtx* ioCtx = new AllocatorIoCtx(testCallback);

    delete ioCtx;
}

} // namespace pos
