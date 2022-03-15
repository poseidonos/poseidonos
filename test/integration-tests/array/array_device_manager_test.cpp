#include "src/array/device/array_device_manager.h"

#include <gtest/gtest.h>

#include "src/include/array_config.h"
#include "test/unit-tests/array/device/array_device_list_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/device/device_manager_mock.h"
#include "test/unit-tests/array/device/array_device_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
shared_ptr<MockUBlockDevice>
MockUblockDevice(const char* devName, const string& SN)
{
    MockUBlockDevice* rawPtr = new MockUBlockDevice(devName, 1024, nullptr);
    EXPECT_CALL(*rawPtr, GetName).WillRepeatedly(Return(devName));
    EXPECT_CALL(*rawPtr, GetSN).WillRepeatedly(Return(SN));
    return shared_ptr<MockUBlockDevice>(rawPtr);
}

shared_ptr<MockUBlockDevice>
MockUblockDevice(const char* devName, DeviceType type, size_t devSize)
{
    MockUBlockDevice* rawPtr = new MockUBlockDevice(devName, 1024, nullptr);
    EXPECT_CALL(*rawPtr, GetType).WillRepeatedly(Return(type));
    EXPECT_CALL(*rawPtr, GetName).WillRepeatedly(Return(devName));
    EXPECT_CALL(*rawPtr, GetSize).WillRepeatedly(Return(devSize));
    return shared_ptr<MockUBlockDevice>(rawPtr);
}

shared_ptr<MockUBlockDevice>
MockUblockDevice(const char* devName)
{
    MockUBlockDevice* rawPtr = new MockUBlockDevice(devName, 1024, nullptr);
    return shared_ptr<MockUBlockDevice>(rawPtr);
}

static void
SuppressUninterestingCalls(const std::list<shared_ptr<MockUBlockDevice>>& uBlockDevices)
{
    for (auto& uBlockDev : uBlockDevices)
    {
        if (uBlockDev == nullptr)
        {
            continue;
        }
        EXPECT_CALL(*uBlockDev, SetClass).WillRepeatedly([](DeviceClass cls){});
        EXPECT_CALL(*uBlockDev, GetSN).WillRepeatedly(Return("mock-SN"));
    }
}

TEST(ArrayDeviceManager, Import_testIfDeviceSetsAreSuccessfullyImported)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);

    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, "mockArrayName");
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

    auto nvm1UblockDevPtr = MockUblockDevice(nvm1.c_str(), DeviceType::NVRAM, 805830656); // minNvmSize when logicalChunkCount is 2
    auto data1UblockDevPtr = MockUblockDevice(data1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data2UblockDevPtr = MockUblockDevice(data2.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data3UblockDevPtr = MockUblockDevice(data3.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto spare1UblockDevPtr = MockUblockDevice(spare1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr))
        .WillOnce(Return(data2UblockDevPtr))
        .WillOnce(Return(data3UblockDevPtr))
        .WillOnce(Return(spare1UblockDevPtr));
    SuppressUninterestingCalls({nvm1UblockDevPtr, data1UblockDevPtr, data2UblockDevPtr, data3UblockDevPtr, spare1UblockDevPtr});

    // When
    int actual = arrDevMgr.ImportByName(nameSet);

    // Then
    ASSERT_EQ(0, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, ImportByName_testIfNVMDeviceHasNoUblock)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    string mockArrayname = "mockArray";

    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, mockArrayname);
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

    auto nvm1UblockDevPtr = nullptr;

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr));

    // When
    int actual = arrDevMgr.ImportByName(nameSet);

    // Then
    int expected = (int)POS_EVENT_ID::ARRAY_NVM_NOT_FOUND;
    ASSERT_EQ(expected, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, ImportByName_testIfNVMDeviceIsActuallySSDDevice)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    string mockArrayName = "mockArray";
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, mockArrayName);
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

    auto nvm1UblockDevPtr = MockUblockDevice(nvm1.c_str(), DeviceType::SSD, 805830656); // minNvmSize when logicalChunkCount is 2

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr));

    // When
    int actual = arrDevMgr.ImportByName(nameSet);

    // Then
    int expected = (int)POS_EVENT_ID::ARRAY_NVM_NOT_FOUND;
    ASSERT_EQ(expected, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, ImportByName_testIfDataDeviceHasNoUblock)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    string mockArrayName = "mockArray";
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, mockArrayName);
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

    auto nvm1UblockDevPtr = MockUblockDevice(nvm1.c_str(), DeviceType::NVRAM, 805830656); // minNvmSize when logicalChunkCount is 2
    auto data1UblockDevPtr = nullptr;

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr));
    SuppressUninterestingCalls({nvm1UblockDevPtr, data1UblockDevPtr});

    // When
    int actual = arrDevMgr.ImportByName(nameSet);

    // Then
    int expected = (int)POS_EVENT_ID::ARRAY_SSD_NOT_FOUND;
    ASSERT_EQ(expected, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, ImportByName_testIfDataDeviceIsActuallyNVMDevice)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    string mockArrayName = "mockArray";
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, mockArrayName);
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

    auto nvm1UblockDevPtr = MockUblockDevice(nvm1.c_str(), DeviceType::NVRAM, 805830656); // minNvmSize when logicalChunkCount is 2
    auto data1UblockDevPtr = MockUblockDevice(data1.c_str(), DeviceType::NVRAM, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr));
    SuppressUninterestingCalls({nvm1UblockDevPtr, data1UblockDevPtr});

    // When
    int actual = arrDevMgr.ImportByName(nameSet);

    // Then
    int expected = (int)POS_EVENT_ID::ARRAY_SSD_NOT_FOUND;
    ASSERT_EQ(expected, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, ImportByName_testIfSpareDeviceHasNoUblock)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    string mockArrayName = "mockArray";
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, mockArrayName);
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

    auto nvm1UblockDevPtr = MockUblockDevice(nvm1.c_str(), DeviceType::NVRAM, 805830656); // minNvmSize when logicalChunkCount is 2
    auto data1UblockDevPtr = MockUblockDevice(data1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data2UblockDevPtr = MockUblockDevice(data2.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data3UblockDevPtr = MockUblockDevice(data3.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto spare1UblockDevPtr = nullptr;

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr))
        .WillOnce(Return(data2UblockDevPtr))
        .WillOnce(Return(data3UblockDevPtr))
        .WillOnce(Return(spare1UblockDevPtr));
    SuppressUninterestingCalls({nvm1UblockDevPtr, data1UblockDevPtr, data2UblockDevPtr, data3UblockDevPtr, spare1UblockDevPtr});

    // When
    int actual = arrDevMgr.ImportByName(nameSet);

    // Then
    int expected = (int)POS_EVENT_ID::ARRAY_SSD_NOT_FOUND;
    ASSERT_EQ(expected, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, ImportByName_testIfSpareDeviceIsActuallyNVMDevice)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    string mockArrayName = "mockArray";
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, mockArrayName);
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

    auto nvm1UblockDevPtr = MockUblockDevice(nvm1.c_str(), DeviceType::NVRAM, 805830656); // minNvmSize when logicalChunkCount is 2
    auto data1UblockDevPtr = MockUblockDevice(data1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data2UblockDevPtr = MockUblockDevice(data2.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data3UblockDevPtr = MockUblockDevice(data3.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto spare1UblockDevPtr = MockUblockDevice(spare1.c_str(), DeviceType::NVRAM, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr))
        .WillOnce(Return(data2UblockDevPtr))
        .WillOnce(Return(data3UblockDevPtr))
        .WillOnce(Return(spare1UblockDevPtr));
    SuppressUninterestingCalls({nvm1UblockDevPtr, data1UblockDevPtr, data2UblockDevPtr, data3UblockDevPtr, spare1UblockDevPtr});

    // When
    int actual = arrDevMgr.ImportByName(nameSet);

    // Then
    int expected = (int)POS_EVENT_ID::ARRAY_SSD_NOT_FOUND;
    ASSERT_EQ(expected, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, ImportByName_testIfNVMDeviceIsTooSmall)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    string mockArrayName = "mockArray";
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, mockArrayName);
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

    auto nvm1UblockDevPtr = MockUblockDevice(nvm1.c_str(), DeviceType::NVRAM, 0); // minNvmSize when logicalChunkCount is 2
    auto data1UblockDevPtr = MockUblockDevice(data1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data2UblockDevPtr = MockUblockDevice(data2.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data3UblockDevPtr = MockUblockDevice(data3.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto spare1UblockDevPtr = MockUblockDevice(spare1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr))
        .WillOnce(Return(data2UblockDevPtr))
        .WillOnce(Return(data3UblockDevPtr))
        .WillOnce(Return(spare1UblockDevPtr));

    SuppressUninterestingCalls({nvm1UblockDevPtr, data1UblockDevPtr, data2UblockDevPtr, data3UblockDevPtr, spare1UblockDevPtr});
    // When
    int actual = arrDevMgr.ImportByName(nameSet);

    // Then
    int expected = (int)POS_EVENT_ID::UNABLE_TO_SET_NVM_CAPACITY_IS_LT_MIN;
    ASSERT_EQ(expected, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, Import_testIfDeviceSetsAreSuccessfullyImportedWithMetaSetInformation)
{
    // Used when loading array
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);

    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, "mockArrayName");
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

    auto nvm1UblockDevPtr = MockUblockDevice(nvm1.c_str(), DeviceType::NVRAM, 805830656); // minNvmSize when logicalChunkCount is 2
    auto data1UblockDevPtr = MockUblockDevice(data1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data2UblockDevPtr = MockUblockDevice(data2.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data3UblockDevPtr = MockUblockDevice(data3.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto spare1UblockDevPtr = MockUblockDevice(spare1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr))
        .WillOnce(Return(data2UblockDevPtr))
        .WillOnce(Return(data3UblockDevPtr))
        .WillOnce(Return(spare1UblockDevPtr))
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr))
        .WillOnce(Return(data2UblockDevPtr))
        .WillOnce(Return(data3UblockDevPtr))
        .WillOnce(Return(spare1UblockDevPtr));
    SuppressUninterestingCalls({nvm1UblockDevPtr, data1UblockDevPtr, data2UblockDevPtr, data3UblockDevPtr, spare1UblockDevPtr});

    arrDevMgr.ImportByName(nameSet);
    ArrayMeta arrayMeta;
    arrayMeta.devs = arrDevMgr.ExportToMeta();
    arrDevMgr.Clear();

    // When
    int actual = arrDevMgr.Import(arrayMeta.devs);

    // Then
    ASSERT_EQ(0, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, Import_testIfNVMDeviceHasNoUblockWithMetaSetInformation)
{
    // Used when loading array
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    string mockArrayName = "mockArray";
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, mockArrayName);
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

    auto nvm1UblockDevPtr = MockUblockDevice(nvm1.c_str(), DeviceType::NVRAM, 805830656); // minNvmSize when logicalChunkCount is 2
    auto data1UblockDevPtr = MockUblockDevice(data1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data2UblockDevPtr = MockUblockDevice(data2.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data3UblockDevPtr = MockUblockDevice(data3.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto spare1UblockDevPtr = MockUblockDevice(spare1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr))
        .WillOnce(Return(data2UblockDevPtr))
        .WillOnce(Return(data3UblockDevPtr))
        .WillOnce(Return(spare1UblockDevPtr))
        .WillOnce(Return(nullptr));
    SuppressUninterestingCalls({nvm1UblockDevPtr, data1UblockDevPtr, data2UblockDevPtr, data3UblockDevPtr, spare1UblockDevPtr});

    arrDevMgr.ImportByName(nameSet);
    ArrayMeta arrayMeta;
    arrayMeta.devs = arrDevMgr.ExportToMeta();
    arrDevMgr.Clear();

    // When
    uint32_t missingCnt = 0;
    uint32_t brokenCnt = 0;
    int actual = arrDevMgr.Import(arrayMeta.devs);

    // Then
    int expected = EID(ARRAY_NVM_NOT_FOUND);
    ASSERT_EQ(expected, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, Import_testIfDataDeviceIsFaultState)
{
    // Used when loading array
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    string mockArrayName = "mockArray";
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, mockArrayName);
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

    auto nvm1UblockDevPtr = MockUblockDevice(nvm1.c_str(), DeviceType::NVRAM, 805830656); // minNvmSize when logicalChunkCount is 2
    auto data1UblockDevPtr = MockUblockDevice(data1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data2UblockDevPtr = MockUblockDevice(data2.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data3UblockDevPtr = MockUblockDevice(data3.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto spare1UblockDevPtr = MockUblockDevice(spare1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr))
        .WillOnce(Return(data2UblockDevPtr))
        .WillOnce(Return(data3UblockDevPtr))
        .WillOnce(Return(spare1UblockDevPtr))
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr))
        .WillOnce(Return(data2UblockDevPtr))
        .WillOnce(Return(data3UblockDevPtr))
        .WillOnce(Return(spare1UblockDevPtr));
    SuppressUninterestingCalls({nvm1UblockDevPtr, data1UblockDevPtr, data2UblockDevPtr, data3UblockDevPtr, spare1UblockDevPtr});

    arrDevMgr.ImportByName(nameSet);
    ArrayMeta arrayMeta;
    arrayMeta.devs = arrDevMgr.ExportToMeta();
    for (DeviceMeta meta : arrayMeta.devs.data)
    {
        meta.state = ArrayDeviceState::FAULT;
    }
    arrDevMgr.Clear();

    // When
    int actual = arrDevMgr.Import(arrayMeta.devs);

    // Then
    ASSERT_EQ(0, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, Import_testIfDataDeviceHasNoUblockWithMetaSetInformation)
{
    // Used when loading array
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    string mockArrayName = "mockArray";
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, mockArrayName);
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

    auto nvm1UblockDevPtr = MockUblockDevice(nvm1.c_str(), DeviceType::NVRAM, 805830656); // minNvmSize when logicalChunkCount is 2
    auto data1UblockDevPtr = MockUblockDevice(data1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data2UblockDevPtr = MockUblockDevice(data2.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto data3UblockDevPtr = MockUblockDevice(data3.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);
    auto spare1UblockDevPtr = MockUblockDevice(spare1.c_str(), DeviceType::SSD, ArrayConfig::MINIMUM_SSD_SIZE_BYTE);

    EXPECT_CALL(mockSysDevMgr, GetDev) // currently, we don't have a good gtest matcher for DevName, hence I'm just simply chaining the expected result
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr))
        .WillOnce(Return(data2UblockDevPtr))
        .WillOnce(Return(data3UblockDevPtr))
        .WillOnce(Return(spare1UblockDevPtr))
        .WillOnce(Return(nvm1UblockDevPtr))
        .WillOnce(Return(data1UblockDevPtr))
        .WillOnce(Return(data2UblockDevPtr))
        .WillOnce(Return(data3UblockDevPtr))
        .WillOnce(Return(spare1UblockDevPtr));
    SuppressUninterestingCalls({nvm1UblockDevPtr, data1UblockDevPtr, data2UblockDevPtr, data3UblockDevPtr, spare1UblockDevPtr});

    arrDevMgr.ImportByName(nameSet);
    ArrayMeta arrayMeta;
    arrayMeta.devs = arrDevMgr.ExportToMeta();
    arrDevMgr.Clear();

    // When
    int actual = arrDevMgr.Import(arrayMeta.devs);

    // Then
    ASSERT_EQ(0, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, Export_testIfArrayDevMgrIsQueriedAgainst)
{
    // Given
    MockDeviceManager mockSysDevMgr;
    ArrayDeviceManager adm(&mockSysDevMgr, "mockArrayName");
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    DeviceSet<ArrayDevice*> emptyDevSet;

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(emptyDevSet));
    adm.SetArrayDeviceList(mockArrayDeviceList);

    // When
    adm.Export();

    // Then: GetDevs is invoked once
}

TEST(ArrayDeviceManager, ExportToName_testIfEmptyDeviceSetIsReturned)
{
    // Given
    ArrayDeviceManager adm(nullptr, "mockArrayName");

    // When
    DeviceSet<string> actual = adm.ExportToName();

    // Then
    ASSERT_EQ(0, actual.data.size());
    ASSERT_EQ(0, actual.nvm.size());
    ASSERT_EQ(0, actual.spares.size());
}

TEST(ArrayDeviceManager, ExportToName_testIfArrayDevListIsQueriedAgainst)
{
    // Given
    ArrayDeviceManager adm(nullptr, "mockArrayName");
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    adm.SetArrayDeviceList(mockArrayDeviceList);
    DeviceSet<string> emptyDevSet;
    EXPECT_CALL(*mockArrayDeviceList, ExportNames).WillOnce(Return(emptyDevSet));

    // When
    adm.ExportToName();

    // Then: GetDevs() should be called once
}

TEST(ArrayDeviceManager, ExportToMeta_testIfDeviceSetIsExtractedFromArrayDevList)
{
    // Given
    ArrayDeviceManager adm(nullptr, "mockArrayName");
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    adm.SetArrayDeviceList(mockArrayDeviceList);
    DeviceSet<ArrayDevice*> deviceSet;

    string nvm1 = "mock-nvm", data1 = "mock-data1", spare1 = "mock-spare1";
    string nvm1SN = "mock-nvm-sn", data1SN = "mock-data1-sn", spare1SN = "mock-spare1-sn";
    auto nvm1UblockDevPtr = MockUblockDevice(nvm1.c_str(), nvm1SN);
    auto data1UblockDevPtr = MockUblockDevice(data1.c_str(), data1SN);
    auto spare1UblockDevPtr = MockUblockDevice(spare1.c_str(), spare1SN);
    ArrayDevice nvmDev(nvm1UblockDevPtr), dataDev(data1UblockDevPtr), spareDev(spare1UblockDevPtr);
    deviceSet.nvm.push_back(&nvmDev);
    deviceSet.data.push_back(&dataDev);
    deviceSet.spares.push_back(&spareDev);
    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));

    // When
    DeviceSet<DeviceMeta> actual = adm.ExportToMeta();

    // Then
    ASSERT_EQ(1, actual.nvm.size());
    ASSERT_EQ(nvm1SN, actual.nvm.at(0).uid);

    ASSERT_EQ(1, actual.data.size());
    ASSERT_EQ(data1SN, actual.data.at(0).uid);

    ASSERT_EQ(1, actual.spares.size());
    ASSERT_EQ(spare1SN, actual.spares.at(0).uid);
}

TEST(ArrayDeviceManager, Clear_testIfNullPtrIsHandled)
{
    // Given
    ArrayDeviceManager adm(nullptr, "mockArrayName");

    // When
    adm.Clear();

    // Then
}

TEST(ArrayDeviceManager, AddSpare_testIfWrongDevnameIsHandled)
{
    // Given
    MockDeviceManager mockSysDevMgr;
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, "mockArrayName");
    string devName = "spare1";

    EXPECT_CALL(mockSysDevMgr, GetDev).WillOnce(Return(nullptr));

    // When
    int actual = arrDevMgr.AddSpare(devName);

    // Then
    ASSERT_EQ(EID(ADD_SPARE_SSD_NAME_NOT_FOUND), actual);
}

TEST(ArrayDeviceManager, AddSpare_testIfAddingSpareAgainIsHandled)
{
    // Given

    MockDeviceManager mockSysDevMgr;
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, "mockArrayName");
    string devName = "spare1";

    auto spare1 = MockUblockDevice(devName.c_str());
    EXPECT_CALL(mockSysDevMgr, GetDev).WillOnce(Return(spare1));
    EXPECT_CALL(*spare1.get(), GetClass).WillOnce(Return(DeviceClass::ARRAY));

    // When
    int actual = arrDevMgr.AddSpare(devName);

    // Then
    ASSERT_EQ(EID(UNABLE_TO_ADD_SSD_ALREADY_OCCUPIED), actual);
}

TEST(ArrayDeviceManager, AddSpare_testIfWrongCapacityIsHandled)
{
    // Given
    MockDeviceManager mockSysDevMgr;
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, "mockArrayName");
    uint64_t EXPECTED_DEV_SIZE = 1024*100;
    uint64_t SMALLER_DEV_SIZE = 1024*80;
    string spareName = "mock-spareDev";
    shared_ptr<MockUBlockDevice> ptrMockUblockDev1 = make_shared<MockUBlockDevice>("mock-dataDev1", EXPECTED_DEV_SIZE, nullptr);
    shared_ptr<MockUBlockDevice> ptrMockUblockDev2 = make_shared<MockUBlockDevice>("mock-dataDev2", EXPECTED_DEV_SIZE, nullptr);
    shared_ptr<MockUBlockDevice> ptrMockUblockDev3 = make_shared<MockUBlockDevice>("mock-dataDev3", EXPECTED_DEV_SIZE, nullptr);
    shared_ptr<MockUBlockDevice> ptrMockSpare = make_shared<MockUBlockDevice>(spareName, SMALLER_DEV_SIZE, nullptr);
    MockArrayDevice dataDev1(ptrMockUblockDev1), dataDev2(ptrMockUblockDev2), dataDev3(ptrMockUblockDev3);
    DeviceSet<ArrayDevice*> deviceSet;
    deviceSet.data.push_back(&dataDev1);
    deviceSet.data.push_back(&dataDev2);
    deviceSet.data.push_back(&dataDev3);

    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    EXPECT_CALL(mockSysDevMgr, GetDev).WillRepeatedly(Return(ptrMockSpare));
    EXPECT_CALL(*ptrMockSpare, GetClass).WillOnce(Return(DeviceClass::SYSTEM));
    EXPECT_CALL(*ptrMockSpare, GetSize).WillRepeatedly(Return(SMALLER_DEV_SIZE)); // intentionally passing in smaller capacity
    EXPECT_CALL(*ptrMockUblockDev1, GetSize).WillRepeatedly(Return(EXPECTED_DEV_SIZE));
    EXPECT_CALL(*ptrMockUblockDev2, GetSize).WillRepeatedly(Return(EXPECTED_DEV_SIZE));
    EXPECT_CALL(*ptrMockUblockDev3, GetSize).WillRepeatedly(Return(EXPECTED_DEV_SIZE));
    EXPECT_CALL(dataDev1, GetUblock).WillRepeatedly(Return(ptrMockUblockDev1));
    EXPECT_CALL(dataDev2, GetUblock).WillRepeatedly(Return(ptrMockUblockDev2));
    EXPECT_CALL(dataDev3, GetUblock).WillRepeatedly(Return(ptrMockUblockDev3));
    EXPECT_CALL(dataDev1, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(dataDev2, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(dataDev3, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));

    // When
    int actual = arrDevMgr.AddSpare(spareName);

    // Then
    ASSERT_EQ(EID(ADD_SPARE_CAPACITY_IS_TOO_SMALL), actual);
}

TEST(ArrayDeviceManager, AddSpare_testIfSpareIsAddedToArrayDeviceList)
{
    // Given
    MockDeviceManager mockSysDevMgr;
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, "mockArrayName");
    string devName = "spare1";
    int EXPECTED_DEV_SIZE = 121212;
    auto data1 = MockUblockDevice("data1");
    auto spare1 = MockUblockDevice(devName.c_str());

    ArrayDevice data1Dev(data1, ArrayDeviceState::NORMAL);
    DeviceSet<ArrayDevice*> deviceSet;
    deviceSet.data.push_back(&data1Dev);
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    EXPECT_CALL(mockSysDevMgr, GetDev).WillOnce(Return(spare1));
    EXPECT_CALL(*spare1.get(), GetClass).WillOnce(Return(DeviceClass::SYSTEM));
    EXPECT_CALL(*spare1.get(), GetSize).WillRepeatedly(Return(EXPECTED_DEV_SIZE));
    EXPECT_CALL(*data1.get(), GetSize).WillRepeatedly(Return(EXPECTED_DEV_SIZE));
    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    EXPECT_CALL(*mockArrayDeviceList, AddSpare(_)).WillOnce([](ArrayDevice* dev)
    {
        delete dev; // to avoid leakage
        return 0;
    });

    // When
    int actual = arrDevMgr.AddSpare(devName);

    // Then
    ASSERT_EQ(0, actual);
}

TEST(ArrayDeviceManager, RemoveSpare_testIfSpareDeviceRemovalFails)
{
    // Given
    MockDeviceManager mockSysDevMgr;
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, "mockArrayName");
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    EXPECT_CALL(mockSysDevMgr, GetDev).WillOnce(Return(nullptr));
    EXPECT_CALL(*mockArrayDeviceList, GetDevs).Times(0);
    EXPECT_CALL(*mockArrayDeviceList, RemoveSpare).Times(0);

    // When
    int actual = arrDevMgr.RemoveSpare("spare-that-doesn't-exist");

    // Then
    ASSERT_EQ(EID(REMOVE_SPARE_SSD_NAME_NOT_FOUND), actual);
}

TEST(ArrayDeviceManager, RemoveSpare_testIfSpareDeviceRemovalIsSuccessful)
{
    // Given
    MockDeviceManager mockSysDevMgr;
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, "mockArrayName");

    auto spare1 = MockUblockDevice("spare1");
    ArrayDevice spare1Dev(spare1, ArrayDeviceState::NORMAL);
    DeviceSet<ArrayDevice*> deviceSet;
    deviceSet.spares.push_back(&spare1Dev);
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    EXPECT_CALL(mockSysDevMgr, GetDev).WillOnce(Return(spare1));
    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    EXPECT_CALL(*mockArrayDeviceList, RemoveSpare).WillOnce(Return(0));

    // When
    int actual = arrDevMgr.RemoveSpare("spare1");

    // Then
    ASSERT_EQ(0, actual);
}

TEST(ArrayDeviceManager, RemoveSpare_testWithPassingArrayDevice)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    string mockArrayName = "mockArray";
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, mockArrayName);

    auto spare1 = MockUblockDevice("spare1");
    ArrayDevice spare1Dev(spare1, ArrayDeviceState::NORMAL);
    DeviceSet<ArrayDevice*> deviceSet;
    deviceSet.spares.push_back(&spare1Dev);
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    EXPECT_CALL(*mockArrayDeviceList, RemoveSpare).WillOnce(Return(0));

    // When
    int actual = arrDevMgr.RemoveSpare(&spare1Dev);

    // Then
    ASSERT_EQ(0, actual);
}

TEST(ArrayDeviceManager, ReplaceWithSpare_testIfArrayDeviceListIsQueriedAgainst)
{
    // Given
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;

    ArrayDeviceManager arrDevMgr(nullptr, "mockArrayName");
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    int REPLACE_SUCCESS = 0;
    EXPECT_CALL(*mockArrayDeviceList, SpareToData).WillOnce(Return(REPLACE_SUCCESS));

    // When
    int actual = arrDevMgr.ReplaceWithSpare(nullptr);

    // Then
    ASSERT_EQ(REPLACE_SUCCESS, actual);
}

TEST(ArrayDeviceManager, GetDev_testIfNullPtrIsHandled)
{
    // Given
    ArrayDeviceManager arrDevMgr(nullptr, "mockArrayName");
    ArrayDevice* arrDev;
    ArrayDeviceType arrDevType;
    UblockSharedPtr uBlock = nullptr;

    // When
    std::tie(arrDev, arrDevType) = arrDevMgr.GetDev(uBlock);

    // Then
    ASSERT_EQ(nullptr, arrDev);
    ASSERT_EQ(ArrayDeviceType::NONE, arrDevType);
}

TEST(ArrayDeviceManager, GetDev_testIfGetDevNVMIsHandled)
{
    // Given
    ArrayDeviceManager arrDevMgr(nullptr, "mockArrayName");
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    DeviceSet<ArrayDevice*> deviceSet;
    auto nvmUBlockDev = MockUblockDevice("mock-nvm");
    ArrayDevice nvmDev(nvmUBlockDev);
    deviceSet.nvm.push_back(&nvmDev);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    ArrayDevice* arrDev;
    ArrayDeviceType arrDevType;

    // When
    std::tie(arrDev, arrDevType) = arrDevMgr.GetDev(nvmUBlockDev);

    // Then
    ASSERT_EQ(&nvmDev, arrDev);
    ASSERT_EQ(ArrayDeviceType::NVM, arrDevType);
}

TEST(ArrayDeviceManager, GetDev_testIfGetDevDATAIsHandled)
{
    // Given
    ArrayDeviceManager arrDevMgr(nullptr, "mockArrayName");
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    DeviceSet<ArrayDevice*> deviceSet;
    auto dataUBlockDev = MockUblockDevice("mock-data");
    ArrayDevice dataDev(dataUBlockDev);
    deviceSet.data.push_back(&dataDev);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    ArrayDevice* arrDev;
    ArrayDeviceType arrDevType;

    // When
    std::tie(arrDev, arrDevType) = arrDevMgr.GetDev(dataUBlockDev);

    // Then
    ASSERT_EQ(&dataDev, arrDev);
    ASSERT_EQ(ArrayDeviceType::DATA, arrDevType);
}

TEST(ArrayDeviceManager, GetDev_testIfGetDevSPAREIsHandled)
{
    // Given
    ArrayDeviceManager arrDevMgr(nullptr, "mockArrayName");
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    DeviceSet<ArrayDevice*> deviceSet;
    auto spareUBlockDev = MockUblockDevice("mock-spare");
    ArrayDevice spareDev(spareUBlockDev);
    deviceSet.spares.push_back(&spareDev);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    ArrayDevice* arrDev;
    ArrayDeviceType arrDevType;

    // When
    std::tie(arrDev, arrDevType) = arrDevMgr.GetDev(spareUBlockDev);

    // Then
    ASSERT_EQ(&spareDev, arrDev);
    ASSERT_EQ(ArrayDeviceType::SPARE, arrDevType);
}

TEST(ArrayDeviceManager, GetDev_testIfGetDevFailedMatchIsHandled)
{
    // Given
    ArrayDeviceManager arrDevMgr(nullptr, "mockArrayName");
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    auto missingUBlockDev = MockUblockDevice("mock-data-missing");
    DeviceSet<ArrayDevice*> deviceSet;
    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    ArrayDevice* arrDev;
    ArrayDeviceType arrDevType;

    // When
    std::tie(arrDev, arrDevType) = arrDevMgr.GetDev(missingUBlockDev);

    // Then
    ASSERT_EQ(nullptr, arrDev);
    ASSERT_EQ(ArrayDeviceType::NONE, arrDevType);
}

TEST(ArrayDeviceManager, GetDev_testIfGetDevDATAIsHandledWithDeviceSerialNumber)
{
    // Given
    MockDeviceManager* mockSysDevMgr = new MockDeviceManager();
    ArrayDeviceManager arrDevMgr(mockSysDevMgr, "mockArrayName");
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    DeviceSet<ArrayDevice*> deviceSet;
    auto dataUBlockDev = MockUblockDevice("mock-data");
    ArrayDevice dataDev(dataUBlockDev);
    deviceSet.data.push_back(&dataDev);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    ArrayDevice* arrDev;
    ArrayDeviceType arrDevType;

    EXPECT_CALL(*mockSysDevMgr, GetDev).WillOnce(Return(dataUBlockDev));

    // When
    std::tie(arrDev, arrDevType) = arrDevMgr.GetDev("mock-data-sn");

    // Then
    ASSERT_EQ(&dataDev, arrDev);
    ASSERT_EQ(ArrayDeviceType::DATA, arrDevType);
    delete mockSysDevMgr;
}

TEST(ArrayDeviceManager, GetFaulty_testIfFaultyArrayDeviceIsReturned)
{
    // Given
    ArrayDeviceManager arrDevMgr(nullptr, "mockArrayName");
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    DeviceSet<ArrayDevice*> deviceSet;
    auto nonFaultyUBlockDev = MockUblockDevice("mock-data-nonfaulty");
    auto faultyUBlockDev = MockUblockDevice("mock-data-faulty");
    ArrayDevice dataNonFaulty(nonFaultyUBlockDev, ArrayDeviceState::NORMAL);
    ArrayDevice dataFaulty(faultyUBlockDev, ArrayDeviceState::FAULT);
    deviceSet.data.push_back(&dataNonFaulty);
    deviceSet.data.push_back(&dataFaulty);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));

    // When
    ArrayDevice* actual = arrDevMgr.GetFaulty();

    // Then
    ASSERT_TRUE(actual != nullptr);
    ASSERT_EQ(&dataFaulty, actual);
}

TEST(ArrayDeviceManager, GetFaulty_testIfNullptrIsReturnedWhenThereIsNoFaultyDevice)
{
    // Given
    ArrayDeviceManager arrDevMgr(nullptr, "mockArrayName");
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    DeviceSet<ArrayDevice*> deviceSet;
    auto nonFaultyUBlockDev1 = MockUblockDevice("mock-data-nonfaulty1");
    auto nonFaultyUBlockDev2 = MockUblockDevice("mock-data-nonfaulty2");
    ArrayDevice dataNonFaulty1(nonFaultyUBlockDev1, ArrayDeviceState::NORMAL);
    ArrayDevice dataNonFaulty2(nonFaultyUBlockDev2, ArrayDeviceState::NORMAL);
    deviceSet.data.push_back(&dataNonFaulty1);
    deviceSet.data.push_back(&dataNonFaulty2);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));

    // When
    ArrayDevice* actual = arrDevMgr.GetFaulty();

    // Then
    ASSERT_EQ(nullptr, actual);
}

TEST(ArrayDeviceManager, GetRebuilding_testIfRebuildDeviceIsReturned)
{
    // Given
    ArrayDeviceManager arrDevMgr(nullptr, "mockArrayName");
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    DeviceSet<ArrayDevice*> deviceSet;
    auto normalUBlockDev = MockUblockDevice("mock-data-normal");
    auto rebuildUBlockDev = MockUblockDevice("mock-data-rebuild");
    ArrayDevice normalDev(normalUBlockDev, ArrayDeviceState::NORMAL);
    ArrayDevice rebuildDev(rebuildUBlockDev, ArrayDeviceState::REBUILD);
    deviceSet.data.push_back(&normalDev);
    deviceSet.data.push_back(&rebuildDev);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    // When
    ArrayDevice* actual = arrDevMgr.GetRebuilding();

    // Then
    ASSERT_EQ(&rebuildDev, actual);
}

TEST(ArrayDeviceManager, GetRebuilding_testIfRebuildDeviceIsNotRebuildState)
{
    // Given
    string mockArrayName = "mockArray";
    ArrayDeviceManager arrDevMgr(nullptr, mockArrayName);
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    DeviceSet<ArrayDevice*> deviceSet;
    auto normalUBlockDev = MockUblockDevice("mock-data-normal");
    auto rebuildUBlockDev = MockUblockDevice("mock-data-rebuild");
    ArrayDevice normalDev(normalUBlockDev, ArrayDeviceState::NORMAL);
    ArrayDevice rebuildDev(rebuildUBlockDev, ArrayDeviceState::NORMAL);
    deviceSet.data.push_back(&normalDev);
    deviceSet.data.push_back(&rebuildDev);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    // When
    ArrayDevice* actual = arrDevMgr.GetRebuilding();

    // Then
    ASSERT_EQ(nullptr, actual);
}

} // namespace pos
