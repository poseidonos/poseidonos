#include "src/array/service/io_device_checker/io_device_checker.h"
#include "test/unit-tests/array/service/io_device_checker/i_device_checker_mock.h"
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"

#include <gtest/gtest.h>

using ::testing::Return;
namespace pos
{
TEST(IODeviceChecker, IODeviceChecker_testConstructor)
{
    // Given

    // When
    IODeviceChecker ioDeviceChecker;
    // Then
}

TEST(IODeviceChecker, Register_testIfCheckerRegistered)
{
    // Given
    IODeviceChecker ioDeviceChecker;
    MockIDeviceChecker* mockIDeviceChecker = new MockIDeviceChecker();
    UblockSharedPtr ublock = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    ArrayDeviceState state = ArrayDeviceState::NORMAL;
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(ublock, state);

    // When
    bool actual = ioDeviceChecker.Register(0, mockIDeviceChecker);

    // Then
    EXPECT_TRUE(actual);
}

TEST(IODeviceChecker, Unregister_testIfCheckerIsDeletedProperly)
{
    // Given
    IODeviceChecker ioDeviceChecker;
    MockIDeviceChecker* mockIDeviceChecker = new MockIDeviceChecker();
    UblockSharedPtr ublock = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    ArrayDeviceState state = ArrayDeviceState::NORMAL;
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(ublock, state);
    EXPECT_CALL(*mockIDeviceChecker, FindDevice).WillOnce(Return(mockArrayDevice));
    ioDeviceChecker.Register(0, mockIDeviceChecker);
    IArrayDevice* dev = ioDeviceChecker.FindDevice(0, "mock-dev");
    EXPECT_EQ(mockArrayDevice, dev);

    // When
    ioDeviceChecker.Unregister(0);
    dev = ioDeviceChecker.FindDevice(0, "mock-dev");

    // Then
    EXPECT_EQ(nullptr, dev);

    // CleanUp
    delete mockIDeviceChecker;
}

TEST(IODeviceChecker, IsRecoverable_testIfCheckerWorks)
{
    // Given
    IODeviceChecker ioDeviceChecker;
    MockIDeviceChecker* mockIDeviceChecker = new MockIDeviceChecker();
    UblockSharedPtr ublock = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    ArrayDeviceState state = ArrayDeviceState::NORMAL;
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(ublock, state);
    EXPECT_CALL(*mockIDeviceChecker, IsRecoverable).WillOnce(Return(static_cast<int>(IoRecoveryRetType::SUCCESS)));
    EXPECT_CALL(*mockArrayDevice, GetUblockPtr).Times(1);
    ioDeviceChecker.Register(0, mockIDeviceChecker);

    // When
    int actual = ioDeviceChecker.IsRecoverable(0 , mockArrayDevice, mockArrayDevice->GetUblockPtr());

    // Then
    ASSERT_EQ(actual, static_cast<int>(IoRecoveryRetType::SUCCESS));

    // CleanUp
    delete mockArrayDevice;
}

TEST(IODeviceChecker, FindDevice_testIfCheckerCanBeFound)
{
    // Given
    IODeviceChecker ioDeviceChecker;
    MockIDeviceChecker* mockIDeviceChecker = new MockIDeviceChecker();
    UblockSharedPtr ublock = make_shared<MockUBlockDevice>("mock-dev", 1024, nullptr);
    ArrayDeviceState state = ArrayDeviceState::NORMAL;
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(ublock, state);
    EXPECT_CALL(*mockIDeviceChecker, FindDevice).WillOnce(Return(mockArrayDevice));

    // When
    IArrayDevice* dev = ioDeviceChecker.FindDevice(0, "mock-dev");
    EXPECT_EQ(nullptr, dev);
    bool actual = ioDeviceChecker.Register(0, mockIDeviceChecker);
    dev = ioDeviceChecker.FindDevice(0, "mock-dev");

    // Then
    EXPECT_TRUE(actual);
    EXPECT_EQ(mockArrayDevice, dev);
}

} // namespace pos
