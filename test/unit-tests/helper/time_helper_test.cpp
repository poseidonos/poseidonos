#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include "src/helper/time/time_helper.h"

using ::testing::_;
using ::testing::Return;

TEST(TimeHelper, GetCurrentTimeStr_testIfCurrentTimeShouldBeANotEmptyString)
{
    // Given
    std::string currTime = "";

    // When
    currTime = GetCurrentTimeStr("%Y-%m-%d %X %z", 32);

    // Then
    ASSERT_NE("", currTime);
}