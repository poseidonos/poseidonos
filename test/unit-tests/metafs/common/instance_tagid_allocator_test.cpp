#include "src/metafs/common/instance_tagid_allocator.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(InstanceTagIdAllocator, CheckTagId)
{
    InstanceTagIdAllocator tag;

    EXPECT_EQ(1, tag());
    EXPECT_EQ(2, tag());

    tag.Reset();

    EXPECT_EQ(1, tag());
}
} // namespace pos
