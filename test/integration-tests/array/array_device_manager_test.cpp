#include "src/array/device/array_device_manager.h"

#include <gtest/gtest.h>

#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/device/device_manager_mock.h"
#include "src/include/array_config.h"

using ::testing::Return;
using ::testing::AtLeast;

namespace pos
{

shared_ptr<MockUBlockDevice> mockUblockDevice(const char* devName, DeviceType type, size_t devSize)
{
    MockUBlockDevice* rawPtr = new MockUBlockDevice(devName, 1024, nullptr);
    EXPECT_CALL(*rawPtr, GetType).WillOnce(Return(type));
    EXPECT_CALL(*rawPtr, GetName).WillRepeatedly(Return(devName));
    EXPECT_CALL(*rawPtr, SetClass).Times(AtLeast(1));
    EXPECT_CALL(*rawPtr, IsAlive).WillOnce(Return(true));
    EXPECT_CALL(*rawPtr, GetSize).WillRepeatedly(Return(devSize));
    return shared_ptr<MockUBlockDevice>(rawPtr);
}

TEST(ArrayDeviceManager, ArrayDeviceManager_)
{
}

TEST(ArrayDeviceManager, Import_testIfDeviceSetsAreSuccessfullyImported)
{
    // Given
    MockDeviceManager mockSysDevMgr;

    ArrayDeviceManager arrDevMgr(&mockSysDevMgr);
    DeviceSet<string> nameSet;
    string nvm1 = "mock-nvm1";
    string data1 = "mock-data1", data2 = "mock-data2", data3 = "mock-data3";
    string spare1 = "mock-spare1";

    nameSet.nvm.push_back(nvm1);
    nameSet.data.push_back(data1);
    nameSet.data.push_back(data2);
    nameSet.data.push_back(data3);
    nameSet.spares.push_back(spare1);
    DevName nvm1Id(nvm1), data1Id(data1), data2Id(data2), data3Id(data3), spare1Id(spare1);

    // std::shared_ptr<MockUBlockDevice>
    auto nvm1UblockDevPtr = mockUblockDevice(nvm1.c_str(), DeviceType::NVRAM, 805830656); // minNvmSize when logicalChunkCount is 2
    auto data1UblockDevPtr = mockUblockDevice(data1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data2UblockDevPtr = mockUblockDevice(data2.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data3UblockDevPtr = mockUblockDevice(data3.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto spare1UblockDevPtr = mockUblockDevice(spare1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr))
        .WillOnce(Return(data2UblockDevPtr))
        .WillOnce(Return(data3UblockDevPtr))
        .WillOnce(Return(spare1UblockDevPtr));

    // When
    int actual = arrDevMgr.Import(nameSet);

    // Then
    ASSERT_EQ(0, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, Export_)
{
}

TEST(ArrayDeviceManager, ExportToName_)
{
}

TEST(ArrayDeviceManager, ExportToMeta_)
{
}

TEST(ArrayDeviceManager, Clear_)
{
}

TEST(ArrayDeviceManager, AddSpare_)
{
}

TEST(ArrayDeviceManager, RemoveSpare_)
{
}

TEST(ArrayDeviceManager, ReplaceWithSpare_)
{
}

TEST(ArrayDeviceManager, GetDev_)
{
}

TEST(ArrayDeviceManager, GetFaulty_)
{
}

TEST(ArrayDeviceManager, GetRebuilding_)
{
}

TEST(ArrayDeviceManager, _CheckDevs_)
{
}

TEST(ArrayDeviceManager, _CheckConstraints_)
{
}

TEST(ArrayDeviceManager, _CheckDevsCount_)
{
}

TEST(ArrayDeviceManager, _CheckFaultTolerance_)
{
}

TEST(ArrayDeviceManager, _CheckSsdsCapacity_)
{
}

TEST(ArrayDeviceManager, _CheckNvmCapacity_)
{
}

TEST(ArrayDeviceManager, _ComputeMinNvmCapacity_)
{
}

TEST(ArrayDeviceManager, _GetBaseline_)
{
}

} // namespace pos
