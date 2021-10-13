#include "src/qos/resource_nvram.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(ResourceNvramStripes, ResourceNvramStripes_Constructor_One_Stack)
{
    ResourceNvramStripes resourceNvramStripes();
}
TEST(ResourceNvramStripes, ResourceNvramStripes_Constructor_One_Heap)
{
    ResourceNvramStripes* resourceNvramStripes = new ResourceNvramStripes();
    delete resourceNvramStripes;
}
TEST(ResourceNvramStripes, Reset_Check_Setter)
{
    ResourceNvramStripes resourceNvramStripes;
    resourceNvramStripes.Reset();
}

TEST(ResourceNvramStripes, SetNvramStripesUsedCount_Check_Setter)
{
    uint32_t usedNvramStripesCount = 1;
    ResourceNvramStripes resourceNvramStripes;
    resourceNvramStripes.SetNvramStripesUsedCount(usedNvramStripesCount);
}

} // namespace pos
