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

TEST(ArrayDeviceList, AddSsd_testWhenArgumentsAreValid)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string dataDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(dataDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    // When
    int result = arrayDeviceList.AddSsd(mockDev);
    // Then
    int SUCCESS = 0;
    EXPECT_EQ(SUCCESS, result);
    delete mockDev;
}

TEST(ArrayDeviceList, AddSsd_testWhenAddTwice)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    arrayDeviceList.AddSsd(mockDev);
    // When
    int result = arrayDeviceList.AddSsd(mockDev);
    // Then
    int ADD_FAIL = EID(UNABLE_TO_ADD_SSD_ALREADY_OCCUPIED);
    EXPECT_EQ(ADD_FAIL, result);
    delete mockDev;
}

TEST(ArrayDeviceList, RemoveSsd_testWhenArgumentsAreValid)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE);
    EXPECT_CALL(*mockDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockDev, GetType).WillRepeatedly(Return(ArrayDeviceType::SPARE));
    arrayDeviceList.Import(vector<ArrayDevice*> { mockDev });
    // When
    int result = arrayDeviceList.RemoveSsd(mockDev);
    // Then
    int SUCCESS = 0;
    EXPECT_EQ(SUCCESS, result);
}

TEST(ArrayDeviceList, RemoveSsd_testWhenNotFound)
{
    // Given
    ArrayDeviceList arrayDeviceList;
    string devName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(devName, 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev = new MockArrayDevice(fakeUblockSharedPtr, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::DATA);
    // When
    int result = arrayDeviceList.RemoveSsd(mockDev);
    // Then
    int REMOVE_FAIL = EID(REMOVE_DEV_SSD_NAME_NOT_FOUND);
    EXPECT_EQ(REMOVE_FAIL, result);
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
    arrayDeviceList.Import(vector<ArrayDevice*> { mockDev, brokenDataDev });

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
    arrayDeviceList.Import(vector<ArrayDevice*> { brokenDataDev });
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
    arrayDeviceList.Import(vector<ArrayDevice*> { mockDev });
    // When
    arrayDeviceList.Clear();
    // Then
    int actual = arrayDeviceList.GetDevs().size();
    EXPECT_EQ(0, actual);
}
} // namespace pos
