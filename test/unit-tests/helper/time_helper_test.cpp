#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include "src/helper/time_helper.h"

using ::testing::_;
using ::testing::Return;

TEST(TimeHelper, GetCurrentTimeStr_testIfCurrentTimeShouldBeANotEmptyString)
{
    // Given
    string currTime = "";

    // When
    currTime = GetCurrentTimeStr("%Y-%m-%d %X %z", 32);

    // Then
    ASSERT_NE("", currTime);
}

TEST(TimeHelper, GetTimeT_testIfConvertStringToTimeTWellByCompareTime)
{
    // Given
    char testTime1[32] = "2021-01-01 23:59";
    char testTime2[32] = "2021-05-01 23:59";
    time_t time_t_testTime1 = GetTimeT(testTime1, "%Y-%m-%d %H:%M");
    time_t time_t_testTime2 = GetTimeT(testTime2, "%Y-%m-%d %H:%M");

    // When
    bool actual = time_t_testTime1 < time_t_testTime2;

    // Then
    ASSERT_TRUE(actual);
}
