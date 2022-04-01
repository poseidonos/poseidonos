#include "src/array/service/io_locker/io_locker.h"
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"

#include <gtest/gtest.h>

using ::testing::Return;
namespace pos
{
TEST(IOLocker, IOLocker_testConstructor)
{
    // Given

    // When
    IOLocker ioLocker;

    // Then
}

TEST(IOLocker, Register_testIfArgumentsAreValid)
{
    // Given
    IOLocker ioLocker;
    vector<ArrayDevice*> devs;
    UblockSharedPtr ublock = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    ArrayDeviceState state = ArrayDeviceState::NORMAL;
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(ublock, state);
    devs.push_back(mockArrayDevice);

    // When
    bool actual = ioLocker.Register(devs);

    // Then
    ASSERT_TRUE(actual);
}

TEST(IOLocker, Unregister_testIfArgumentsAreValid)
{
    // Given
    IOLocker ioLocker;
    vector<ArrayDevice*> devs;
    UblockSharedPtr ublock = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    ArrayDeviceState state = ArrayDeviceState::NORMAL;
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(ublock, state);
    devs.push_back(mockArrayDevice);
    ioLocker.Register(devs);

    // When
    ioLocker.Unregister(devs);

    // Then
}

TEST(IOLocker, TryBusyLock_testIfArgumentsAreValid)
{
    // Given
    IOLocker ioLocker;
    vector<ArrayDevice*> devs;
    UblockSharedPtr ublock = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    ArrayDeviceState state = ArrayDeviceState::NORMAL;
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(ublock, state);
    devs.push_back(mockArrayDevice);
    ioLocker.Register(devs);

    // When
    StripeId startId = 0;
    StripeId endId = 1;
    bool actual = ioLocker.TryBusyLock(mockArrayDevice, startId, endId);

    // Then
    ASSERT_TRUE(actual);
}

TEST(IOLocker, ResetBusyLock_testIfArgumentsAreValid)
{
    // Given
    IOLocker ioLocker;
    vector<ArrayDevice*> devs;
    UblockSharedPtr ublock = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    ArrayDeviceState state = ArrayDeviceState::NORMAL;
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(ublock, state);
    devs.push_back(mockArrayDevice);
    ioLocker.Register(devs);

    // When
    bool actual = ioLocker.ResetBusyLock(mockArrayDevice);

    // Then
    ASSERT_TRUE(actual);
}

TEST(IOLocker, TryLock_testIfArgumentsAreValid)
{
    // Given
    IOLocker ioLocker;
    vector<ArrayDevice*> devs;
    UblockSharedPtr ublock = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    ArrayDeviceState state = ArrayDeviceState::NORMAL;
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(ublock, state);
    devs.push_back(mockArrayDevice);
    ioLocker.Register(devs);
    std::set<IArrayDevice*> lockDevs;
    lockDevs.insert(mockArrayDevice);

    // When
    StripeId sId = 0;
    bool actual = ioLocker.TryLock(lockDevs, sId);

    // Then
    ASSERT_TRUE(actual);
}

TEST(IOLocker, TryLock_testForRollback)
{
    // Given
    IOLocker ioLocker;
    UblockSharedPtr ublock1 = make_shared<MockUBlockDevice>("mock-dev1", 1024, nullptr);
    UblockSharedPtr ublock2 = make_shared<MockUBlockDevice>("mock-dev2", 1024, nullptr);
    UblockSharedPtr ublock3 = make_shared<MockUBlockDevice>("mock-dev3", 1024, nullptr);
    UblockSharedPtr ublock4 = make_shared<MockUBlockDevice>("mock-dev4", 1024, nullptr);
    ArrayDeviceState state = ArrayDeviceState::NORMAL;
    MockArrayDevice* mockArrayDevice1 = new MockArrayDevice(ublock1, state);
    MockArrayDevice* mockArrayDevice2 = new MockArrayDevice(ublock2, state);
    MockArrayDevice* mockArrayDevice3 = new MockArrayDevice(ublock3, state);
    MockArrayDevice* mockArrayDevice4 = new MockArrayDevice(ublock4, state);

    ioLocker.Register({mockArrayDevice1, mockArrayDevice2, mockArrayDevice3, mockArrayDevice4});
    std::set<IArrayDevice*> alreadyLockedDevs;
    alreadyLockedDevs.insert(mockArrayDevice4);
    StripeId sId = 0;
    bool ret = ioLocker.TryBusyLock(mockArrayDevice4, sId, sId);
    ASSERT_TRUE(ret);

    // When
    std::set<IArrayDevice*> rollbackTestDevs;
    rollbackTestDevs.insert(mockArrayDevice1);
    rollbackTestDevs.insert(mockArrayDevice2);
    rollbackTestDevs.insert(mockArrayDevice3);
    rollbackTestDevs.insert(mockArrayDevice4);
    ret = ioLocker.TryLock(rollbackTestDevs, sId);
    ASSERT_FALSE(ret);

    bool alreadyLocked = ioLocker.TryLock(alreadyLockedDevs, sId);
    std::set<IArrayDevice*> canLockDevs;
    canLockDevs.insert(mockArrayDevice1);
    canLockDevs.insert(mockArrayDevice3);
    bool canLock = ioLocker.TryLock(canLockDevs, sId);

    // Then
    ASSERT_FALSE(alreadyLocked);
    ASSERT_TRUE(canLock);
}

TEST(IOLocker, Unlock_testForSingleDev)
{
    // Given
    IOLocker ioLocker;
    UblockSharedPtr ublock1 = make_shared<MockUBlockDevice>("mock-dev1", 1024, nullptr);
    UblockSharedPtr ublock2 = make_shared<MockUBlockDevice>("mock-dev2", 1024, nullptr);
    ArrayDeviceState state = ArrayDeviceState::NORMAL;
    MockArrayDevice* mockArrayDevice1 = new MockArrayDevice(ublock1, state);
    MockArrayDevice* mockArrayDevice2 = new MockArrayDevice(ublock2, state);
    ioLocker.Register({mockArrayDevice1, mockArrayDevice2});
    std::set<IArrayDevice*> lockDevs;
    lockDevs.insert(mockArrayDevice1);
    StripeId sId = 0;
    ioLocker.TryLock(lockDevs, sId);

    // When
    ioLocker.Unlock(mockArrayDevice1, sId);

    // Then
}

TEST(IOLocker, Unlock_testForMultipleDevs)
{
    // Given
    IOLocker ioLocker;
    UblockSharedPtr ublock1 = make_shared<MockUBlockDevice>("mock-dev1", 1024, nullptr);
    UblockSharedPtr ublock2 = make_shared<MockUBlockDevice>("mock-dev2", 1024, nullptr);
    ArrayDeviceState state = ArrayDeviceState::NORMAL;
    MockArrayDevice* mockArrayDevice1 = new MockArrayDevice(ublock1, state);
    MockArrayDevice* mockArrayDevice2 = new MockArrayDevice(ublock2, state);
    ioLocker.Register({mockArrayDevice1, mockArrayDevice2});
    std::set<IArrayDevice*> lockDevs;
    lockDevs.insert(mockArrayDevice1);
    StripeId sId = 0;
    ioLocker.TryLock(lockDevs, sId);

    // When
    ioLocker.Unlock(lockDevs, sId);

    // Then
}

} // namespace pos
