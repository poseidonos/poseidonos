#include <gtest/gtest.h>
#include <vector>
#include "src/array/build/device_builder.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/device/device_manager_mock.h"
#include "src/include/pos_event_id.h"
 
using ::testing::Return;

namespace pos
{

TEST(DeviceBuilder, Create_testIfDeviceListBuiltSuccessfullyWhenInputsAreValid)
{
    // Given
    DeviceSet<string> devs;
    devs.nvm.push_back("nvm");
    devs.data.push_back("data1");
    devs.data.push_back("data2");
    devs.spares.push_back("spare");
    uint32_t EXPECTED_DEV_CNT = 4;
    shared_ptr<MockUBlockDevice> mockUblockDev = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    MockDeviceManager mockDevMgr;
    EXPECT_CALL(mockDevMgr, GetDev).WillRepeatedly(Return(mockUblockDev));

    // When
    DeviceBuilder builder;
    vector<ArrayDevice*> devList;
    int ret = builder.Create(devs, devList, &mockDevMgr);

    // Then
    ASSERT_EQ(0, ret);
    ASSERT_EQ(EXPECTED_DEV_CNT, devList.size());
}

TEST(DeviceBuilder, Create_testIfDeviceBuildingFailsWhenFailedToGetTheDevice)
{
    // Given
    DeviceSet<string> devs;
    devs.nvm.push_back("nvm");
    devs.data.push_back("data1");
    devs.data.push_back("data2");
    devs.spares.push_back("spare");
    shared_ptr<MockUBlockDevice> mockUblockDev = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    MockDeviceManager mockDevMgr;
    EXPECT_CALL(mockDevMgr, GetDev).WillRepeatedly(Return(nullptr)); // returns nullptr since there's no matching device

    // When
    DeviceBuilder builder;
    vector<ArrayDevice*> devList;
    int ret = builder.Create(devs, devList, &mockDevMgr);

    // Then
    ASSERT_EQ(EID(CREATE_ARRAY_SSD_NAME_NOT_FOUND), ret);
}

TEST(DeviceBuilder, Load_testIfDeviceListBuiltSuccessfullyWhenInputsAreValid)
{
    // Given
    DeviceSet<DeviceMeta> devs;
    devs.nvm.push_back(DeviceMeta("nvm", ArrayDeviceState::NORMAL));
    devs.data.push_back(DeviceMeta("data1", ArrayDeviceState::NORMAL));
    devs.data.push_back(DeviceMeta("data2", ArrayDeviceState::NORMAL));
    devs.spares.push_back(DeviceMeta("spare1", ArrayDeviceState::NORMAL));
    uint32_t EXPECTED_DEV_CNT = 4;
    shared_ptr<MockUBlockDevice> mockUblockDev = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    MockDeviceManager mockDevMgr;
    EXPECT_CALL(mockDevMgr, GetDev).WillRepeatedly(Return(mockUblockDev));

    // When
    DeviceBuilder builder;
    vector<ArrayDevice*> devList;
    int ret = builder.Load(devs, devList, &mockDevMgr);

    // Then
    ASSERT_EQ(0, ret);
    ASSERT_EQ(EXPECTED_DEV_CNT, devList.size());
}

TEST(DeviceBuilder, Load_testIfDataDeviceBuildingSuccessButStateIsFaultWhenFailedToGetTheDevice)
{
    // Given
    DeviceSet<DeviceMeta> devs;
    devs.nvm.push_back(DeviceMeta("nvm", ArrayDeviceState::NORMAL));
    devs.data.push_back(DeviceMeta("data1", ArrayDeviceState::NORMAL));
    devs.data.push_back(DeviceMeta("data2", ArrayDeviceState::NORMAL));
    devs.spares.push_back(DeviceMeta("spare1", ArrayDeviceState::NORMAL));

    shared_ptr<MockUBlockDevice> mockUblockDev = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    MockDeviceManager mockDevMgr;
    EXPECT_CALL(mockDevMgr, GetDev) // returns nullptr since there's no matching device
        .WillOnce(Return(nullptr)) // for data1, if the data device is nullptr, It is added, but FAULT
        .WillOnce(Return(nullptr)) // for data2, if the data device is nullptr, It is added, but FAULT
        .WillOnce(Return(mockUblockDev)) // for nvm, nvm should not be nullptr
        .WillOnce(Return(nullptr)); // for spare1, if the spare device is nullptr, it is ignored
    uint32_t EXPECTED_DEV_CNT = 3; // Four are entered, but only three are added because spare is ignored.

    // When
    DeviceBuilder builder;
    vector<ArrayDevice*> devList;
    int ret = builder.Load(devs, devList, &mockDevMgr);

    // Then
    ASSERT_EQ(0, ret);
    ASSERT_EQ(EXPECTED_DEV_CNT, devList.size());
    for (auto d : devList)
    {
        if (d->GetType() == ArrayDeviceType::NVM)
        {
            ASSERT_EQ(ArrayDeviceState::NORMAL, d->GetState());
        }
        else
        {
            ASSERT_EQ(ArrayDeviceState::FAULT, d->GetState());
        }
    }
}

}  // namespace pos
