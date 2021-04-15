#include "src/io/general_io/internal_write_completion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "src/spdk_wrapper/free_buffer_pool.h"
#include "test/unit-tests/array/ft/buffer_entry_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(InternalWriteCompletion, InternalWriteCompletion_OneArgument_Stack)
{
    //Given

    //When: Create new InternalWriteCompletion with single argument
    BufferEntry bufferEntry(nullptr, 1, false);
    InternalWriteCompletion internalWriteCompletion(bufferEntry);

    //Then: Do nothing
}

TEST(InternalWriteCompletion, InternalWriteCompletion_OneArgument_Heap)
{
    //Given

    //When: Create new InternalWriteCompletion with single argument
    BufferEntry bufferEntry(nullptr, 1, false);
    InternalWriteCompletion* internalWriteCompletion = new InternalWriteCompletion(bufferEntry);
    delete internalWriteCompletion;

    //Then: Do nothing
}

TEST(InternalWriteCompletion, _DoSpecificJob_Normal)
{
    //Given: InternalWriteCompletion is given a valid bufferEntry
    FreeBufferPool freeBufferPool(1, 2 * 1024 * 1024);
    void* orgBuffer = freeBufferPool.GetBuffer();
    ASSERT_NE(nullptr, orgBuffer);

    BufferEntry bufferEntry(orgBuffer, 1, false);
    bufferEntry.SetFreeBufferPool(&freeBufferPool);

    //No other buffer is remaining
    ASSERT_EQ(nullptr, freeBufferPool.GetBuffer());

    InternalWriteCompletion internalWriteCompletion(bufferEntry);

    //When: Execute InternalWriteCompletion
    bool actual = internalWriteCompletion.Execute();

    //Then: InternalWriteCompletion should return the given buffer and return success
    ASSERT_EQ(actual, true);
    ASSERT_EQ(orgBuffer, freeBufferPool.GetBuffer());
}

} // namespace pos
