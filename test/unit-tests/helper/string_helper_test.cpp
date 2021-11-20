#include "src/helper/string/string_helper.h"

#include <gtest/gtest.h>

TEST(StringHelper, Trim_testIfBothLTrimAndRTrimWorksWell)
{
    // Given
    string testStr = " This is a string for trim test.    ";

    // When
    string outStr = trim(testStr);
    string expectedTrimmedStr = "This is a string for trim test.";

    // Then
    ASSERT_EQ(expectedTrimmedStr, outStr);
}
