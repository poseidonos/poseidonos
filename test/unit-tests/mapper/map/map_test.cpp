#include "src/mapper/map/map.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(Map, Map_)
{
    Map map(0, 0);
    Map map2();
}

TEST(Map, GetMpageWithLock_)
{
    Map map(5, 4032);
    char* ret = map.GetMpageWithLock(100);
    EXPECT_EQ(nullptr, ret);

    map.GetMpageWithLock(1);
}

TEST(Map, AllocateMpage_)
{
    Map map(5, 4032);
    map.AllocateMpage(1);
    char* ret = map.AllocateMpage(1);
    EXPECT_EQ(nullptr, ret);
}

} // namespace pos
