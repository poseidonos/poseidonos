#include "src/array/device/array_device.h"

#include <gtest/gtest.h>

#include "src/device/base/ublock_device.h"
#include "src/device/unvme/unvme_ssd.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/utils/spdk_util.h"

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

TEST(ArrayDevice, GetUblockPtr_testWhenArgumentsAreValid)
{
    // Given
    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    ArrayDevice arrayDev(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    // When
    UBlockDevice* actual = arrayDev.GetUblockPtr();

    // Then
    EXPECT_EQ(spareDevName, actual->GetName());
}

} // namespace pos
