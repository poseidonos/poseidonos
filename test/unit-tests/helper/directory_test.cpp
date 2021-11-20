#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include "src/helper/file/directory.h"

using ::testing::_;
using ::testing::Return;

TEST(Directory, MakeDir_testIfNoDirectoryBefore)
{
     // Given
    string testDirName = "test";
    if (DirExists(testDirName) == true)
    {
        rmdir(testDirName.c_str());
    }

    // When
    bool actual = MakeDir(testDirName);

    // Then
    ASSERT_TRUE(actual);

    // Clean Up
    rmdir(testDirName.c_str());
}

TEST(Directory, MakeDir_testIfNoParentDirectory)
{
     // Given
    string parentDirName = "no_parents";
    string testDirName = parentDirName + "/test";
    if (DirExists(parentDirName) == true)
    {
        rmdir(parentDirName.c_str());
    }

    // When
    bool actual = MakeDir(testDirName);

    // Then
    ASSERT_TRUE(actual);

    // Clean Up
    rmdir(testDirName.c_str());
    rmdir(parentDirName.c_str());
}

TEST(Directory, DirExists_testIfAlreadyExists)
{
    // Given
    string testDirName = "test";

    // When
    bool result1 = MakeDir(testDirName);
    bool result2 = MakeDir(testDirName);

    // Then
    ASSERT_TRUE(result1);
    ASSERT_TRUE(result2);

    // Clean Up
    rmdir(testDirName.c_str());
}

TEST(Directory, DirExists_testIfDirNotExist)
{
    // Given
    string testDirName = "test";
    if (DirExists(testDirName) == true)
    {
        rmdir(testDirName.c_str());
    }

    // When
    bool actual = DirExists(testDirName);

    // Then
    ASSERT_FALSE(actual);
}

TEST(Directory, DirExists_testIfDirExists)
{
    // Given
    string testDirName = "test";
    if (DirExists(testDirName) == false)
    {
        MakeDir(testDirName);
    }

    // When
    bool actual = DirExists(testDirName);

    // Then
    ASSERT_TRUE(actual);
}
