#include "src/logger/filter.h"

#include <gtest/gtest.h>

namespace pos_logger
{
TEST(Filter, Clear_)
{
}

TEST(Filter, IsFiltered_)
{
}

TEST(Filter, ApplyFilter_)
{
}

TEST(Filter, ShouldLog_testIfEventIdIsZero)
{
    // Given
    Filter filter;
    // When
    bool ret = filter.ShouldLog(0);
    // Then
    EXPECT_TRUE(ret);
}

TEST(Filter, IncludeRule_)
{
}

TEST(Filter, ExcludeRule_)
{
}

TEST(Filter, _Decode_)
{
}

TEST(Filter, _Split_)
{
}

TEST(Filter, _Valid_)
{
}

TEST(Filter, _Included_)
{
}

TEST(Filter, _NotExcluded_)
{
}

TEST(Filter, _Include_)
{
}

TEST(Filter, _Exclude_)
{
}

} // namespace pos_logger
