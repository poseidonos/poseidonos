#include "src/device/base/io_context.h"

#include <gtest/gtest.h>

using namespace pos;
TEST(IOContext, IOContextDestructor_testIfIOContextAllocatedToHeapAndStack)
{
    // Given
    IOContext ioCtxStack;
    IOContext* ioCtxHeap = new IOContext();

    // When
    delete ioCtxHeap;
}
