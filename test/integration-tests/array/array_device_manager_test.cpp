#include "src/array/device/array_device_manager.h"

#include <gtest/gtest.h>

#include "src/include/array_config.h"
#include "test/unit-tests/array/device/array_device_list_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/device/device_manager_mock.h"

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
    EXPECT_CALL(*rawPtr, GetType).WillOnce(Return(type));
    EXPECT_CALL(*rawPtr, GetName).WillRepeatedly(Return(devName));
    EXPECT_CALL(*rawPtr, SetClass).Times(AtLeast(1));
    EXPECT_CALL(*rawPtr, IsAlive).WillOnce(Return(true));
    EXPECT_CALL(*rawPtr, GetSize).WillRepeatedly(Return(devSize));
    return shared_ptr<MockUBlockDevice>(rawPtr);
}

shared_ptr<MockUBlockDevice>
MockUblockDevice(const char* devName)
{
    MockUBlockDevice* rawPtr = new MockUBlockDevice(devName, 1024, nullptr);
    return shared_ptr<MockUBlockDevice>(rawPtr);
}

TEST(ArrayDeviceManager, Import_testIfDeviceSetsAreSuccessfullyImported)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);

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

    // When
    int actual = arrDevMgr.Import(nameSet);

    // Then
    ASSERT_EQ(0, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, Import_testIfDeviceSetsAreSuccessfullyImportedWithMetaSetInformation)
{
    // Used when loading array
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);

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

    arrDevMgr.Import(nameSet);
    ArrayMeta arrayMeta;
    arrayMeta.devs = arrDevMgr.ExportToMeta();
    arrDevMgr.Clear();

    // When
    uint32_t missingCnt = 0;
    uint32_t brokenCnt = 0;
    int actual = arrDevMgr.Import(arrayMeta.devs, missingCnt, brokenCnt);

    // Then
    ASSERT_EQ(0, actual);
    arrDevMgr.Clear(); // to avoid the leakage of mocks
}

TEST(ArrayDeviceManager, Export_testIfArrayDevMgrIsQueriedAgainst)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    ArrayDeviceManager adm(&mockSysDevMgr);
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
    ArrayDeviceManager adm(nullptr);

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
    ArrayDeviceManager adm(nullptr);
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
    ArrayDeviceManager adm(nullptr);
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
    ArrayDeviceManager adm(nullptr);

    // When
    adm.Clear();

    // Then
}

TEST(ArrayDeviceManager, AddSpare_testIfWrongDevnameIsHandled)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr);
    string devName = "spare1";

    EXPECT_CALL(mockSysDevMgr, GetDev).WillOnce(Return(nullptr));

    // When
    int actual = arrDevMgr.AddSpare(devName);

    // Then
    ASSERT_EQ(EID(ARRAY_DEVICE_WRONG_NAME), actual);
}

TEST(ArrayDeviceManager, AddSpare_testIfAddingSpareAgainIsHandled)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr);
    string devName = "spare1";

    auto spare1 = MockUblockDevice(devName.c_str());
    EXPECT_CALL(mockSysDevMgr, GetDev).WillOnce(Return(spare1));
    EXPECT_CALL(*spare1.get(), GetClass).WillOnce(Return(DeviceClass::ARRAY));

    // When
    int actual = arrDevMgr.AddSpare(devName);

    // Then
    ASSERT_EQ(EID(ARRAY_DEVICE_ALREADY_ADDED), actual);
}

TEST(ArrayDeviceManager, AddSpare_testIfNotAliveSpareIsHandled)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr);
    string devName = "spare1";

    auto spare1 = MockUblockDevice(devName.c_str());
    EXPECT_CALL(mockSysDevMgr, GetDev).WillOnce(Return(spare1));
    EXPECT_CALL(*spare1.get(), GetClass).WillOnce(Return(DeviceClass::SYSTEM));
    EXPECT_CALL(*spare1.get(), IsAlive).WillOnce(Return(false));

    // When
    int actual = arrDevMgr.AddSpare(devName);

    // Then
    ASSERT_EQ(-2, actual);
}

TEST(ArrayDeviceManager, AddSpare_testIfWrongCapacityIsHandled)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr);
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
    EXPECT_CALL(*spare1.get(), IsAlive).WillOnce(Return(true));
    EXPECT_CALL(*spare1.get(), GetSize).WillOnce(Return(EXPECTED_DEV_SIZE + 1)); // intentionally passing in wrong capacity
    EXPECT_CALL(*data1.get(), GetSize).WillOnce(Return(EXPECTED_DEV_SIZE));
    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));

    // When
    int actual = arrDevMgr.AddSpare(devName);

    // Then
    ASSERT_EQ(EID(ARRAY_SSD_SAME_CAPACITY_ERROR), actual);
}

TEST(ArrayDeviceManager, AddSpare_testIfSpareIsAddedToArrayDeviceList)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr);
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
    EXPECT_CALL(*spare1.get(), IsAlive).WillOnce(Return(true));
    EXPECT_CALL(*spare1.get(), GetSize).WillOnce(Return(EXPECTED_DEV_SIZE));
    EXPECT_CALL(*data1.get(), GetSize).WillOnce(Return(EXPECTED_DEV_SIZE));
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
    MockDeviceManager mockSysDevMgr(nullptr);
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr);
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    EXPECT_CALL(mockSysDevMgr, GetDev).WillOnce(Return(nullptr));
    EXPECT_CALL(*mockArrayDeviceList, GetDevs).Times(0);
    EXPECT_CALL(*mockArrayDeviceList, RemoveSpare).Times(0);

    // When
    int actual = arrDevMgr.RemoveSpare("spare-that-doesn't-exist");

    // Then
    ASSERT_EQ(EID(ARRAY_DEVICE_REMOVE_FAIL), actual);
}

TEST(ArrayDeviceManager, RemoveSpare_testIfSpareDeviceRemovalIsSuccessful)
{
    // Given
    MockDeviceManager mockSysDevMgr(nullptr);
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr);

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

TEST(ArrayDeviceManager, ReplaceWithSpare_testIfArrayDeviceListIsQueriedAgainst)
{
    // Given
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;

    ArrayDeviceManager arrDevMgr(nullptr);
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
    ArrayDeviceManager arrDevMgr(nullptr);
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
    ArrayDeviceManager arrDevMgr(nullptr);
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
    ArrayDeviceManager arrDevMgr(nullptr);
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
    ArrayDeviceManager arrDevMgr(nullptr);
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
    ArrayDeviceManager arrDevMgr(nullptr);
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
    ArrayDeviceManager arrDevMgr(mockSysDevMgr);
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
    ArrayDeviceManager arrDevMgr(nullptr);
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
    ArrayDeviceManager arrDevMgr(nullptr);
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
    ArrayDeviceManager arrDevMgr(nullptr);
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

} // namespace pos
