#include "src/array/device/array_device.h"

#include <gtest/gtest.h>

#include "test/unit-tests/device/base/ublock_device_mock.h"

namespace pos
{
// These are trival tests.
TEST(ArrayDevice, ArrayDevice_testIfGettersSettersAreProperlyInvoked)
{
    // Given
    UblockSharedPtr mockUblockDevPtr = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    ArrayDevice arrDev(mockUblockDevPtr);

    // When
    UblockSharedPtr actualPtr = arrDev.GetUblock();

    // Then
    ASSERT_EQ(mockUblockDevPtr, actualPtr);

    // When
    UblockSharedPtr mockUblockDevPtrVer2 = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    arrDev.SetUblock(mockUblockDevPtrVer2);

    // Then
    ASSERT_EQ(mockUblockDevPtrVer2, arrDev.GetUblock());

    // When
    arrDev.SetState(ArrayDeviceState::REBUILD);

    // Then
    ASSERT_EQ(ArrayDeviceState::REBUILD, arrDev.GetState());

}

} // namespace pos
