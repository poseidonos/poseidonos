#include "src/array/device/array_device_list.h"

#include <gtest/gtest.h>

#include "src/device/unvme/unvme_ssd.h"
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/utils/spdk_util.h"
#include "src/include/pos_event_id.h"

using ::testing::Return;
namespace pos
{
TEST(ArrayDeviceList, ArrayDeviceList_testconstructor)
{
    // Given

    // When
    ArrayDeviceList arrayDeviceList;
    // Then
}

TEST(ArrayDeviceList, Exists_testWhenThereIsNoDev)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    // When
    ArrayDeviceType actual = arrayDeviceList.Exists("noDev");
    // Then
    EXPECT_EQ(ArrayDeviceType::NONE, actual);
}

TEST(ArrayDeviceList, Exists_testWhenDataDevExists)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string dataDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(dataDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr)); // Setting expectation should come before .AddData() below; otherwise, we'd see GMOCK warning uninteresting call.
    arrayDeviceList.AddData(mockDev);

    // When
    ArrayDeviceType actual = arrayDeviceList.Exists("mock-unvme");
    // Then
    EXPECT_EQ(ArrayDeviceType::DATA, actual);
    delete mockDev;
}

TEST(ArrayDeviceList, Exists_testWhenNvmDevExists)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string dataDevName = "mock-uram";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(dataDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(nullptr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.SetNvm(mockDev);

    // When
    ArrayDeviceType actual = arrayDeviceList.Exists("mock-uram");
    // Then
    EXPECT_EQ(ArrayDeviceType::NVM, actual);
    delete mockDev;
}

TEST(ArrayDeviceList, SetNvm_testWhenArgumentsAreValid)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string nvmDevName = "mock-uram";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(nvmDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(nullptr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    // When
    int result = arrayDeviceList.SetNvm(mockDev);
    // Then
    int SUCCESS = 0;
    EXPECT_EQ(SUCCESS, result);
    delete mockDev;
}

TEST(ArrayDeviceList, SetNvm_testWhenSettingNvmTwice)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string nvmDevName = "mock-uram";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(nvmDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(nullptr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.SetNvm(mockDev);
    // When
    int result = arrayDeviceList.SetNvm(mockDev);
    // Then
    int ADD_FAIL = EID(UNABLE_TO_SET_NVM_MORE_THAN_ONE);
    EXPECT_EQ(ADD_FAIL, result);
    delete mockDev;
}

TEST(ArrayDeviceList, SetNvm_testWhenSettingWrongNvm)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string nvmDevName = "mock-uram";
    MockArrayDevice* mockDev = new MockArrayDevice(nullptr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(nullptr));
    // When
    int result = arrayDeviceList.SetNvm(mockDev);
    // Then
    int ADD_FAIL = EID(UNABLE_TO_SET_NULL_NVM);
    EXPECT_EQ(ADD_FAIL, result);
    delete mockDev;
}

TEST(ArrayDeviceList, SetNvm_testWhenSettingNvmWithDataDevice)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string dataDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(dataDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.AddData(mockDev);
    // When
    int result = arrayDeviceList.SetNvm(mockDev);
    // Then
    int ADD_FAIL = EID(UNABLE_TO_SET_NVM_ALREADY_OCCUPIED);
    EXPECT_EQ(ADD_FAIL, result);
    delete mockDev;
}

TEST(ArrayDeviceList, AddData_testWhenArgumentsAreValid)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string dataDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(dataDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    // When
    int result = arrayDeviceList.AddData(mockDev);
    // Then
    int SUCCESS = 0;
    EXPECT_EQ(SUCCESS, result);
    delete mockDev;
}

TEST(ArrayDeviceList, AddSpare_testWhenArgumentsAreValid)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    // When
    int result = arrayDeviceList.AddSpare(mockDev);
    // Then
    int SUCCESS = 0;
    EXPECT_EQ(SUCCESS, result);
    delete mockDev;
}

TEST(ArrayDeviceList, AddSpare_testWhenAddTwice)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.AddSpare(mockDev);
    // When
    int result = arrayDeviceList.AddSpare(mockDev);
    // Then
    int ADD_FAIL = EID(UNABLE_TO_ADD_SSD_ALREADY_OCCUPIED);
    EXPECT_EQ(ADD_FAIL, result);
    delete mockDev;
}

TEST(ArrayDeviceList, RemoveSpare_testWhenArgumentsAreValid)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.AddSpare(mockDev);
    // When
    int result = arrayDeviceList.RemoveSpare(mockDev);
    // Then
    int SUCCESS = 0;
    EXPECT_EQ(SUCCESS, result);
    delete mockDev;
}

TEST(ArrayDeviceList, RemoveSpare_testWhenNoSpare)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    // When
    int result = arrayDeviceList.RemoveSpare(mockDev);
    // Then
    int REMOVE_FAIL = EID(REMOVE_SPARE_SSD_NAME_NOT_FOUND);
    EXPECT_EQ(REMOVE_FAIL, result);
    delete mockDev;
}

TEST(ArrayDeviceList, SpareToData_testReplacingBroken)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    MockArrayDevice* brokenDataDev = new MockArrayDevice(nullptr, ArrayDeviceState::FAULT);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*brokenDataDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.AddSpare(mockDev);
    arrayDeviceList.AddData(brokenDataDev);
    // When
    int result = arrayDeviceList.SpareToData(brokenDataDev);
    // Then
    int SUCCESS = 0;
    EXPECT_EQ(SUCCESS, result);
    EXPECT_NE(nullptr, brokenDataDev);
    delete mockDev;
    delete brokenDataDev;
}

TEST(ArrayDeviceList, SpareToData_testIfThereIsNoSpare)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* brokenDataDev = new MockArrayDevice(nullptr, ArrayDeviceState::FAULT);
    EXPECT_CALL(*brokenDataDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.AddData(brokenDataDev);
    // When
    int result = arrayDeviceList.SpareToData(brokenDataDev);
    // Then
    int NOSPARE = EID(NO_SPARE_SSD_TO_REPLACE);
    EXPECT_EQ(NOSPARE, result);
    delete brokenDataDev;
}

TEST(ArrayDeviceList, Clear_testWhenArgumentsAreValid)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string dataDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(dataDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.AddData(mockDev);
    // When
    arrayDeviceList.Clear();
    // Then
    ArrayDeviceType actual = arrayDeviceList.Exists(dataDevName);
    EXPECT_EQ(ArrayDeviceType::NONE, actual);
}

TEST(ArrayDeviceList, GetDevs_testWhenThereIsOneDataDevice)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string dataDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(dataDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.AddData(mockDev);
    // When
    auto devs = arrayDeviceList.GetDevs();
    // Then
    int devNum = devs.data.size();
    EXPECT_EQ(1, devNum);
    delete mockDev;
}

TEST(ArrayDeviceList, ExportNames_testWhenThereIsOneDataDevice)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string dataDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(dataDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.AddData(mockDev);
    // When
    auto devNames = arrayDeviceList.ExportNames();
    // Then
    int devNum = devNames.data.size();
    EXPECT_EQ(1, devNum);
    delete mockDev;
}

TEST(ArrayDeviceList, ExportNames_testWhenThereIsOneBrokenDataDevice)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string dataDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(dataDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockBrokenDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockBrokenDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.AddData(mockBrokenDev);
    // When
    auto devNames = arrayDeviceList.ExportNames();
    // Then
    int devNum = devNames.data.size();
    EXPECT_EQ(1, devNum);
    delete mockBrokenDev;
}

TEST(ArrayDeviceList, ExportNames_testWhenThereAreManyDevices)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string nvmDevName = "mock-unvme1";
    string dataDevName = "mock-unvme2";
    string spareDevName = "mock-unvme3";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr1 = make_shared<UnvmeSsd>(nvmDevName, 1024, nullptr, fakeNs, "mock-addr");
    UblockSharedPtr fakeUblockSharedPtr2 = make_shared<UnvmeSsd>(dataDevName, 1024, nullptr, fakeNs, "mock-addr");
    UblockSharedPtr fakeUblockSharedPtr3 = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockNvmDev = new MockArrayDevice(nullptr, ArrayDeviceState::NORMAL);
    MockArrayDevice* mockDataDev = new MockArrayDevice(fakeUblockSharedPtr2, ArrayDeviceState::NORMAL);
    MockArrayDevice* mockSpareDev = new MockArrayDevice(fakeUblockSharedPtr3, ArrayDeviceState::NORMAL);
    EXPECT_CALL(*mockNvmDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr1));
    EXPECT_CALL(*mockDataDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr2));
    EXPECT_CALL(*mockSpareDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr3));
    arrayDeviceList.SetNvm(mockNvmDev);
    arrayDeviceList.AddData(mockDataDev);
    arrayDeviceList.AddSpare(mockSpareDev);
    // When
    auto devNames = arrayDeviceList.ExportNames();
    // Then
    int nvmNum = devNames.nvm.size();
    int dataNum = devNames.data.size();
    int spareNum = devNames.spares.size();
    EXPECT_EQ(1, nvmNum);
    EXPECT_EQ(1, dataNum);
    EXPECT_EQ(1, spareNum);
    delete mockNvmDev;
    delete mockDataDev;
    delete mockSpareDev;
}

} // namespace pos
