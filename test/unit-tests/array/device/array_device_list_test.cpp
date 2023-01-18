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

TEST(ArrayDeviceList, SetNvm_testWhenArgumentsAreValid)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string nvmDevName = "mock-uram";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(nvmDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(nullptr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::NVM);
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::NVM));
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
    MockArrayDevice* mockDev = new MockArrayDevice(nullptr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::NVM);
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::NVM));
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.SetNvm(mockDev);
    // When
    int result = arrayDeviceList.SetNvm(mockDev);
    // Then
    int ADD_FAIL = EID(UNABLE_TO_SET_NVM_MORE_THAN_ONE);
    EXPECT_EQ(ADD_FAIL, result);
    delete mockDev;
}

TEST(ArrayDeviceList, SetNvm_testWhenSettingNullUblock)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string nvmDevName = "mock-uram";
    MockArrayDevice* mockDev = new MockArrayDevice(nullptr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::NVM);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(nullptr));
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::NVM));
    // When
    int result = arrayDeviceList.SetNvm(mockDev);
    // Then
    int ADD_FAIL = EID(UNABLE_TO_SET_NVM_NO_OR_NULL);
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
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::DATA);
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    // When
    int result = arrayDeviceList.SetNvm(mockDev);
    // Then
    int ADD_FAIL = EID(ARRAY_DEVICE_TYPE_NOT_MATCHED);
    EXPECT_EQ(ADD_FAIL, result);
    delete mockDev;
}

TEST(ArrayDeviceList, AddSsd_testWhenAddDataAndArgumentsAreValid)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string dataDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(dataDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::DATA);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    // When
    int result = arrayDeviceList.AddSsd(mockDev);
    // Then
    int SUCCESS = 0;
    EXPECT_EQ(SUCCESS, result);
    delete mockDev;
}

TEST(ArrayDeviceList, AddSsd_testWhenAddSpareAndArgumentsAreValid)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::SPARE));
    // When
    int result = arrayDeviceList.AddSsd(mockDev);
    // Then
    int SUCCESS = 0;
    EXPECT_EQ(SUCCESS, result);
    delete mockDev;
}

TEST(ArrayDeviceList, AddSsd_testWhenAddTheSameDeviceTwice)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::DATA);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    arrayDeviceList.AddSsd(mockDev);
    // When
    int result = arrayDeviceList.AddSsd(mockDev);
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
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::SPARE));
    arrayDeviceList.AddSsd(mockDev);
    // When
    int result = arrayDeviceList.RemoveSpare(mockDev);
    // Then
    int SUCCESS = 0;
    EXPECT_EQ(SUCCESS, result);
}

TEST(ArrayDeviceList, RemoveSpare_testWhenNoSpare)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE);
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::SPARE));
    // When
    int result = arrayDeviceList.RemoveSpare(mockDev);
    // Then
    int REMOVE_FAIL = EID(REMOVE_DEV_SSD_NAME_NOT_FOUND);
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
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE);
    MockArrayDevice* brokenDataDev = new MockArrayDevice(nullptr, ArrayDeviceState::FAULT, 0, ArrayDeviceType::DATA);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::SPARE));
    EXPECT_CALL(*brokenDataDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*brokenDataDev, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    arrayDeviceList.AddSsd(mockDev);
    arrayDeviceList.AddSsd(brokenDataDev);
    // When
    ArrayDevice* out;
    int result = arrayDeviceList.SpareToData(brokenDataDev, out);
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
    MockArrayDevice* brokenDataDev = new MockArrayDevice(nullptr, ArrayDeviceState::FAULT, 0, ArrayDeviceType::DATA);
    EXPECT_CALL(*brokenDataDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*brokenDataDev, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    arrayDeviceList.AddSsd(brokenDataDev);
    // When
    ArrayDevice* out;
    int result = arrayDeviceList.SpareToData(brokenDataDev, out);
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
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::DATA);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    arrayDeviceList.AddSsd(mockDev);
    // When
    arrayDeviceList.Clear();
    // Then
    auto numOfDevs = arrayDeviceList.GetDevs().size();
    EXPECT_EQ(0, numOfDevs);
}

TEST(ArrayDeviceList, GetDevs_testWhenThereIsOneDataDevice)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string dataDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(dataDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::DATA);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    int actual = arrayDeviceList.AddSsd(mockDev);
    // When
    auto devs = arrayDeviceList.GetDevs();
    // Then
    ArrayDeviceType type = devs.front()->GetType();
    EXPECT_EQ(0, actual);
    EXPECT_EQ(ArrayDeviceType::DATA, type);
    delete mockDev;
}
} // namespace pos
