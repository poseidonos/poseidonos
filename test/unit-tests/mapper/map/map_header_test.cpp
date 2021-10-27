#include "src/mapper/map/map_header.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MapHeader, TestSimpleGetter)
{
    MapHeader header(0);
    int ret = header.GetMapId();
    EXPECT_EQ(0, ret);
    ret = header.GetSize();
    EXPECT_EQ(0, ret);
}

} // namespace pos
