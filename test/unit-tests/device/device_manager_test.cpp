#include "src/device/device_manager.h"

#include <gtest/gtest.h>
#include <string>

#include "test/unit-tests/spdk_wrapper/caller/spdk_nvme_caller_mock.h"

using namespace pos;

TEST(DeviceManager, GetNvmeCtrlr_testIfFailedToFindDevice)
{
    // Given
    DeviceManager devMgr;
    string deviceName = "NotExistDeviceName";

    // When
    struct spdk_nvme_ctrlr* ret = devMgr.GetNvmeCtrlr(deviceName);

    // Then
    EXPECT_EQ(ret, nullptr);
}
