#include "src/gc/copier_write_completion.h"

#include <gtest/gtest.h>

namespace pos
{
// FIXME: The same (class, method) exists in copier_write_completion_test.cpp and stripe_copy_submission_test.cpp.
// Commenting out doesn't work since gtest actually parses test files to extract test cases.
// As a quick workaround, we rename those test cases manually (2021/03/04)
TEST(GcChunkWriteCompletion, CopierWriteCompletionGcChunkWriteCompletion_)
{
}

TEST(GcChunkWriteCompletion, CopierWriteCompletion_DoSpecificJob_)
{
}

} // namespace pos

namespace pos
{
TEST(GcBlkWriteCompletion, CopierWriteCompletionGcBlkWriteCompletion_)
{
}

TEST(GcBlkWriteCompletion, CopierWriteCompletion_DoSpecificJob_)
{
}

} // namespace pos
