#include "src/array/device/array_device_list.h"

#include <gtest/gtest.h>

#include "src/device/unvme/unvme_ssd.h"
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/utils/spdk_util.h"

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

TEST(ArrayDeviceList, Exists_testWhenDevExists)
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

TEST(ArrayDeviceList, SpareToData_)
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

} // namespace pos
