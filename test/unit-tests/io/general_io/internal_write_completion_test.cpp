#include "src/io/general_io/internal_write_completion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/array/ft/buffer_entry_mock.h"
#include "test/unit-tests/utils/mock_builder.h"

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

TEST(InternalWriteCompletion, DISABLED_DoSpecificJob_Normal)
{
    //Given: InternalWriteCompletion is given a valid bufferEntry
    MockBufferEntry mockBufferEntry(nullptr, 0, false);

    /*
        TODO : BufferEntry is real object of InternalWriteCompletion.
            So cannot inject the mock and check the call time.
            If bufferEntry changed to pointer, it's better to unit-test
    */
    // EXPECT_CALL(mockBufferEntry, ReturnBuffer).Times(1);

    InternalWriteCompletion internalWriteCompletion(mockBufferEntry);

    //When: Execute InternalWriteCompletion
    bool actual = internalWriteCompletion.Execute();

    //Then: InternalWriteCompletion should return the given buffer and return success
    ASSERT_EQ(actual, true);
}

} // namespace pos
