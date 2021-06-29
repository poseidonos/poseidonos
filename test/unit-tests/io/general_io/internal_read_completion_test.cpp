#include "src/io/general_io/internal_read_completion.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(InternalReadCompletion, InternalReadCompletion_OneArgument_Stack)
{
    //Given

    //When: Create new InternalReadCompletion with single argument
    uint32_t weight = 50;
    InternalReadCompletion internalReadCompletion(weight);

    //Then: Do nothing
}

TEST(InternalReadCompletion, InternalReadCompletion_OneArgument_Heap)
{
    //Given

    //When: Create new InternalReadCompletion with single argument
     uint32_t weight = 50;
    InternalReadCompletion* internalReadCompletion = new InternalReadCompletion(weight);
    delete internalReadCompletion;

    //Then: Do nothing
}

TEST(InternalReadCompletion, _DoSpecificJob_Normal)
{
    //Given
    uint32_t weight = 50;
    InternalReadCompletion internalReadCompletion(weight);

    //When: Execute InternalReadCompletion
    bool actual = internalReadCompletion.Execute();

    //Then: InternalReadCompletion should return success
    ASSERT_EQ(actual, true);
}

} // namespace pos
