#include "src/array/array_name_policy.h"

#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"

namespace pos
{
TEST(ArrayNamePolicy, CheckArrayName_testIfWhiteSpaceIsDisallowed)
{
    // Given
    string badName = " arraystartingwhitespace";
    ArrayNamePolicy p;

    // When
    int actual = p.CheckArrayName(badName);

    // Then
    ASSERT_EQ(EID(CREATE_ARRAY_NAME_START_OR_END_WITH_SPACE), actual);
}

TEST(ArrayNamePolicy, CheckArrayName_testIfTrailingWhitespaceIsDisallowed)
{
    // Given
    string badName = "array ";
    ArrayNamePolicy p;

    // When
    int actual = p.CheckArrayName(badName);

    // Then
    ASSERT_EQ(EID(CREATE_ARRAY_NAME_START_OR_END_WITH_SPACE), actual);
}

TEST(ArrayNamePolicy, CheckArrayName_testIfNonAlphanumericIsDisallowed)
{
    // Given
    string badName = "array!";
    ArrayNamePolicy p;

    // When
    int actual = p.CheckArrayName(badName);

    // Then
    ASSERT_EQ(EID(CREATE_ARRAY_NAME_INCLUDES_SPECIAL_CHAR), actual);
}

} // namespace pos
