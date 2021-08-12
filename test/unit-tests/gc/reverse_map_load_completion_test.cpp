#include "src/gc/reverse_map_load_completion.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(ReverseMapLoadCompletion, Execute_Invoke)
{
    ReverseMapLoadCompletion* revMapLoadCompletion = new ReverseMapLoadCompletion();
    EXPECT_TRUE(revMapLoadCompletion->Execute() == true); // trival no op in revMapLoadCompletion
    delete revMapLoadCompletion;
}

} // namespace pos
