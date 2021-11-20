#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include "src/helper/string/string_checker.h"

using ::testing::_;
using ::testing::Return;

TEST(StringChecker, StringLength_testLengthOfEmptyString)
{
    // Given
    string emptyStr = "";
    StringChecker sc(emptyStr);

    // When
    size_t actual = sc.Length();

    // Then
    ASSERT_EQ(0, actual);
}

TEST(StringChecker, StringLength_testLengthOf11Characters)
{
    // Given
    string strLenIs11 = "abcdef ghij";
    StringChecker sc(strLenIs11);

    // When
    size_t actual = sc.Length();

    // Then
    ASSERT_EQ(11, actual);
}

TEST(StringChecker, StringStartWith_testIfStringStartsWithSpace)
{
    // Given
    string strStartWithSpace = " test";
    StringChecker sc(strStartWithSpace);

    // When
    const char SPACE = ' ';
    bool actual = sc.StartWith(SPACE);

    // Then
    ASSERT_TRUE(actual);
}

TEST(StringChecker, StringStartWith_testIfFalseQuery)
{
    // Given
    string str = "test";
    StringChecker sc(str);

    // When
    const char wrongChar = 's';
    bool actual = sc.StartWith(wrongChar);

    // Then
    ASSERT_FALSE(actual);
}

TEST(StringChecker, StringEndWith_testIfStringEndsWithSpace)
{
    // Given
    string strEndWithSpace = "test ";
    StringChecker sc(strEndWithSpace);

    // When
    const char SPACE = ' ';
    bool actual = sc.EndWith(SPACE);

    // Then
    ASSERT_TRUE(actual);
}

TEST(StringChecker, StringEndWith_testIfFalseQuery)
{
    // Given
    string str = "test";
    StringChecker sc(str);

    // When
    const char wrongChar = 's';
    bool actual = sc.EndWith(wrongChar);

    // Then
    ASSERT_FALSE(actual);
}

TEST(StringChecker, StringOnlyContains_testIfStringOnlyContainsAllowedCharacter)
{
    // Given
    const char* ALLOWED_CHAR = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_- ";
    string test_str = "it is a unit-test of string checker";
    StringChecker sc(test_str);

    // When
    bool actual = sc.OnlyContains(ALLOWED_CHAR);

    // Then
    ASSERT_TRUE(actual);
}

TEST(StringChecker, StringOnlyContains_testIfStringContainsNotAllowedCharacterAlso)
{
    // Given
    const char* ALLOWED_CHAR = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_- ";
    string notallowedcharisincluded = "it is include question mark ? which is not allowed.";
    StringChecker sc(notallowedcharisincluded);

    // When
    bool actual = sc.OnlyContains(ALLOWED_CHAR);

    // Then
    ASSERT_FALSE(actual);
}
