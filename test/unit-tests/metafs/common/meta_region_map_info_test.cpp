#include "src/metafs/common/meta_region_map_info.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaRegionMap, CreateObj0)
{
    MetaRegionMap map;

    EXPECT_EQ(map.baseLpn, UINT64_MAX);
    EXPECT_EQ(map.maxLpn, UINT64_MAX);
}

TEST(MetaRegionMap, CreateObj1)
{
    MetaRegionMap map(0, 1);

    EXPECT_EQ(map.baseLpn, 0);
    EXPECT_EQ(map.maxLpn, 1);
}
} // namespace pos
