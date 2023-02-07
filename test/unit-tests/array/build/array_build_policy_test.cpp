#include "src/array/build/array_build_policy.h"

#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"

namespace pos
{
TEST(ArrayBuildPolicy, CheckArrayName_testIfWhiteSpaceIsDisallowed)
{
    // Given
    string badName = " arraystartingwhitespace";

    // When
    int actual = CheckArrayName(badName);

    // Then
    ASSERT_EQ(EID(ARRAY_NAME_START_OR_END_WITH_SPACE), actual);
}

TEST(ArrayBuildPolicy, CheckArrayName_testIfTrailingWhitespaceIsDisallowed)
{
    // Given
    string badName = "array ";

    // When
    int actual = CheckArrayName(badName);

    // Then
    ASSERT_EQ(EID(ARRAY_NAME_START_OR_END_WITH_SPACE), actual);
}

TEST(ArrayBuildPolicy, CheckArrayName_testIfNonAlphanumericIsDisallowed)
{
    // Given
    string badName = "array!";

    // When
    int actual = CheckArrayName(badName);

    // Then
    ASSERT_EQ(EID(ARRAY_NAME_INCLUDES_SPECIAL_CHAR), actual);
}

} // namespace pos
