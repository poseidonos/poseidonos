#include "src/array/rebuild/rebuild_handler.h"

#include <gtest/gtest.h>

#include "test/unit-tests/array/array_mock.h"
#include "test/unit-tests/array/device/array_device_mock.h"

using ::testing::Return;

namespace pos
{
TEST(RebuildHandler, RebuildHandler_testConstructor)
{
    // Given: nothing

    // When
    RebuildHandler rh(nullptr, nullptr);

    // Then
}

TEST(RebuildHandler, Execute_testIfTrueIsReturnedWhenTriggerRebuildFails)
{
    // Given
    MockArray mockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    MockArrayDevice mockArrayDevice(nullptr);
    RebuildHandler rh(&mockArray, &mockArrayDevice);

    EXPECT_CALL(mockArray, TriggerRebuild).WillOnce(Return(false));

    // When
    bool actual = rh.Execute();

    // Then
    ASSERT_TRUE(actual);
}

TEST(RebuildHandler, Execute_testIfFalseIsReturnedWhenTriggerRebuildSucceeds)
{
    // Given
    MockArray mockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    MockArrayDevice mockArrayDevice(nullptr);
    RebuildHandler rh(&mockArray, &mockArrayDevice);

    EXPECT_CALL(mockArray, TriggerRebuild).WillOnce(Return(true));

    // When
    bool actual = rh.Execute();

    // Then
    ASSERT_FALSE(actual);
}

} // namespace pos
