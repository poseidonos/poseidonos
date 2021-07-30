#include "src/metafs/include/meta_file_extent.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaFileExtent, Compare1)
{
    MetaFileExtent a(0, 100);
    MetaFileExtent b(0, 100);
    MetaFileExtent c(1, 100);
    MetaFileExtent d(0, 101);

    EXPECT_TRUE((a == b));
    EXPECT_FALSE((a == c));
    EXPECT_FALSE((a == d));
}

TEST(MetaFileExtent, Compare2)
{
    MetaFileExtent a(0, 100);
    MetaFileExtent b(0, 100);
    MetaFileExtent c(1, 100);
    MetaFileExtent d(0, 101);

    EXPECT_FALSE((a < b));
    EXPECT_TRUE((a < c));
}

TEST(MetaFileExtent, Compare3)
{
    MetaFileExtent a(0, 100);
    MetaFileExtent b(0, 100);
    MetaFileExtent c(1, 100);
    MetaFileExtent d(0, 101);

    EXPECT_FALSE((a > d));
    EXPECT_TRUE((c > a));
}

TEST(MetaFileExtent, Setter)
{
    MetaFileExtent a;
    MetaFileExtent b;

    a.SetStartLpn(1);
    b.SetStartLpn(1);

    a.SetCount(1);
    b.SetCount(1);

    EXPECT_TRUE((a == b));
}

TEST(MetaFileExtent, Getter)
{
    MetaFileExtent a(0, 100);
    MetaFileExtent b(0, 100);

    EXPECT_TRUE(a.GetStartLpn() == b.GetStartLpn());
    EXPECT_TRUE(a.GetCount() == b.GetCount());
    EXPECT_TRUE(a.GetLast() == b.GetLast());
}

} // namespace pos
