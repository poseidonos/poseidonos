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
using ::testing::Invoke;

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
    ASSERT_EQ(EID(UNABLE_TO_ADD_DEV_ALREADY_OCCUPIED), actual);
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
    MockArrayDevice dataDev1(ptrMockUblockDev1, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::DATA);
    MockArrayDevice dataDev2(ptrMockUblockDev2, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::DATA);
    MockArrayDevice dataDev3(ptrMockUblockDev3, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::DATA);
    vector<ArrayDevice*> deviceSet;
    deviceSet.push_back(&dataDev1);
    deviceSet.push_back(&dataDev2);
    deviceSet.push_back(&dataDev3);

    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);
    arrDevMgr.Import(deviceSet);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    EXPECT_CALL(mockSysDevMgr, GetDev).WillRepeatedly(Return(ptrMockSpare));
    EXPECT_CALL(*ptrMockSpare, GetClass).WillOnce(Return(DeviceClass::SYSTEM));
    EXPECT_CALL(*ptrMockSpare, GetSize).WillRepeatedly(Return(SMALLER_DEV_SIZE)); // intentionally passing in smaller capacity
    EXPECT_CALL(dataDev1, GetSize).WillRepeatedly(Return(EXPECTED_DEV_SIZE));
    EXPECT_CALL(dataDev2, GetSize).WillRepeatedly(Return(EXPECTED_DEV_SIZE));
    EXPECT_CALL(dataDev3, GetSize).WillRepeatedly(Return(EXPECTED_DEV_SIZE));
    EXPECT_CALL(dataDev1, GetUblock).WillRepeatedly(Return(ptrMockUblockDev1));
    EXPECT_CALL(dataDev2, GetUblock).WillRepeatedly(Return(ptrMockUblockDev2));
    EXPECT_CALL(dataDev3, GetUblock).WillRepeatedly(Return(ptrMockUblockDev3));
    EXPECT_CALL(dataDev1, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(dataDev2, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(dataDev3, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(dataDev1, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(dataDev2, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(dataDev3, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));

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
    shared_ptr<MockUBlockDevice> data1 = make_shared<MockUBlockDevice>("mock-dataDev1", EXPECTED_DEV_SIZE, nullptr);
    shared_ptr<MockUBlockDevice> spare1 = MockUblockDevice(devName.c_str());

    MockArrayDevice data1Dev(data1, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::DATA);
    vector<ArrayDevice*> deviceSet;
    deviceSet.push_back(&data1Dev);
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);
    arrDevMgr.Import(deviceSet);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    auto deleteAndReturnSuccess = [](auto arg){ delete arg; return 0; };
    EXPECT_CALL(*mockArrayDeviceList, AddSsd).WillOnce(Invoke(deleteAndReturnSuccess));
    EXPECT_CALL(mockSysDevMgr, GetDev).WillOnce(Return(spare1));
    EXPECT_CALL(data1Dev, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(data1Dev, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(data1Dev, GetSize).WillRepeatedly(Return(EXPECTED_DEV_SIZE));
    EXPECT_CALL(*data1.get(), GetSize).WillRepeatedly(Return(EXPECTED_DEV_SIZE));
    EXPECT_CALL(*spare1.get(), GetClass).WillOnce(Return(DeviceClass::SYSTEM));
    EXPECT_CALL(*spare1.get(), GetSize).WillRepeatedly(Return(EXPECTED_DEV_SIZE));

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
    vector<ArrayDevice*> deviceSet;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    EXPECT_CALL(*mockArrayDeviceList, RemoveSsd).Times(0);

    // When
    int actual = arrDevMgr.RemoveSpare("spare-that-doesn't-exist");

    // Then
    ASSERT_EQ(EID(REMOVE_DEV_SSD_NAME_NOT_FOUND), actual);
}

TEST(ArrayDeviceManager, RemoveSpare_testIfSpareDeviceRemovalIsSuccessful)
{
    // Given
    MockDeviceManager mockSysDevMgr;
    ArrayDeviceManager arrDevMgr(&mockSysDevMgr, "mockArrayName");

    auto spare1 = MockUblockDevice("spare1");
    MockArrayDevice spare1Dev(spare1, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE);
    vector<ArrayDevice*> deviceSet;
    deviceSet.push_back(&spare1Dev);
    MockArrayDeviceList* mockArrayDeviceList = new MockArrayDeviceList;
    arrDevMgr.SetArrayDeviceList(mockArrayDeviceList);

    EXPECT_CALL(*mockArrayDeviceList, GetDevs).WillOnce(ReturnRef(deviceSet));
    EXPECT_CALL(*mockArrayDeviceList, RemoveSsd).WillOnce(Return(0));
    EXPECT_CALL(spare1Dev, GetType).WillOnce(Return(ArrayDeviceType::SPARE));
    EXPECT_CALL(spare1Dev, GetName).WillOnce(Return("spare1"));

    // When
    int actual = arrDevMgr.RemoveSpare("spare1");

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
    ArrayDevice* out;
    int actual = arrDevMgr.ReplaceWithSpare(nullptr, out);

    // Then
    ASSERT_EQ(REPLACE_SUCCESS, actual);
}
} // namespace pos
