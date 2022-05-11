#include "src/array/device/array_device.h"

#include <gtest/gtest.h>

#include "src/device/base/ublock_device.h"
#include "src/device/unvme/unvme_ssd.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/utils/spdk_util.h"

using ::testing::Return;

namespace pos
{
// These are trival tests.
TEST(ArrayDevice, ArrayDevice_testIfGettersSettersAreProperlyInvoked)
{
    // Given
    string devName = "mock-dev";
    string devSn = "mock-sn";
    shared_ptr<MockUBlockDevice> mockUblock = make_shared<MockUBlockDevice>(
            devName, 1024, nullptr);
    EXPECT_CALL(*mockUblock, GetName).WillRepeatedly(Return(devName.c_str()));
    EXPECT_CALL(*mockUblock, GetSN).WillRepeatedly(Return(devSn));
    ArrayDevice arrDev(mockUblock, ArrayDeviceState::NORMAL);

    // When
    UblockSharedPtr actualPtr = arrDev.GetUblock();

    // Then
    ASSERT_EQ(mockUblock, actualPtr);

    // When
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>("mock-dev-2", 1024, nullptr, fakeNs, "mock-addr");
    arrDev.SetUblock(fakeUblockSharedPtr);
    string prevUblockInfo = devName + "(" + devSn + ")";

    // Then
    ASSERT_EQ(fakeUblockSharedPtr, arrDev.GetUblock());
    ASSERT_EQ(prevUblockInfo, arrDev.PrevUblockInfo());

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
