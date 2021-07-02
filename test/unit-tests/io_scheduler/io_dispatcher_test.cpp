#include "src/io_scheduler/io_dispatcher.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/spdk_wrapper/event_framework_api.h"
#include "src/io_scheduler/io_worker.h"
#include "src/event_scheduler/io_completer.h"
#include "src/include/i_array_device.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/device/base/ublock_device.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/event_factory.h"

#include "test/unit-tests/bio/ubio_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/event_scheduler/event_factory_mock.h"
#include "test/unit-tests/include/i_array_device_mock.h"
#include "test/unit-tests/io_scheduler/io_worker_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{

TEST(IODispatcher, IODispatcher_Stack)
{
    // Given: Do nothing

    // When: Create IODispatcher
    IODispatcher ioDispatcher;

    // Then: Do nothing
}

TEST(IODispatcher, IODispatcher_Heap)
{
    // Given: Do nothing

    // When: Create IODispatcher
    IODispatcher* ioDispatcher = new IODispatcher;
    delete ioDispatcher;

    // Then: Do nothing
}

TEST(IODispatcher, AddIOWorker_ZeroCPUSet)
{
    // Given: Set IODispatcher, cpu_set_t
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);

    // When: Call AddIOWorker with zero cpu_set_t
    ioDispatcher.AddIOWorker(cpuSet);

    // Then: Expect ioWorkerMap inserted properly
    ASSERT_EQ(ioDispatcher.SizeIOWorker(), 0);
}

TEST(IODispatcher, AddIOWorker_ValidCPUSet)
{
    // Given: Set IODispatcher, cpu_set_t
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(4, &cpuSet);
    CPU_SET(7, &cpuSet);

    // When: Call AddIOWorker with valid cpu_set_t
    ioDispatcher.AddIOWorker(cpuSet);

    // Then: Expect ioWorkerMap inserted properly
    ASSERT_EQ(ioDispatcher.SizeIOWorker(), 2);
}

TEST(IODispatcher, AddIOWorker_IntersectionCPUSet)
{
    // Given: Set IODispatcher, cpu_set_t
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet1, cpuSet2;
    CPU_ZERO(&cpuSet1);
    CPU_SET(4, &cpuSet1);
    CPU_SET(7, &cpuSet1);
    CPU_ZERO(&cpuSet2);
    CPU_SET(7, &cpuSet2);
    CPU_SET(13, &cpuSet2);

    // When: Call twice AddIOWorker with intersection cpu_set_t
    ioDispatcher.AddIOWorker(cpuSet1);
    ioDispatcher.AddIOWorker(cpuSet2);

    // Then: Expect ioWorkerMap inserted properly
    ASSERT_EQ(ioDispatcher.SizeIOWorker(), 3);
}

TEST(IODispatcher, RemoveIOWorker_ValidCPUSet)
{
    // Given: Set IODispatcher, cpu_set_t, AddIOWorker
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(4, &cpuSet);
    CPU_SET(7, &cpuSet);
    ioDispatcher.AddIOWorker(cpuSet);

    // When: Call RemoveIOWorker with valid cpu_set_t
    ioDispatcher.RemoveIOWorker(cpuSet);

    // Then: Expect ioWorkerMap erased properly
    ASSERT_EQ(ioDispatcher.SizeIOWorker(), 0);
}

TEST(IODispatcher, RemoveIOWorker_IntersectionCPUSet)
{
    // Given: Set IODispatcher, cpu_set_t, AddIOWorker
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet1, cpuSet2;
    CPU_ZERO(&cpuSet1);
    CPU_SET(4, &cpuSet1);
    CPU_SET(7, &cpuSet1);
    CPU_ZERO(&cpuSet2);
    CPU_SET(7, &cpuSet2);
    CPU_SET(13, &cpuSet2);
    ioDispatcher.AddIOWorker(cpuSet1);

    // When: Call RemoveIOWorker with intersection cpu_set_t
    ioDispatcher.RemoveIOWorker(cpuSet2);

    // Then: Expect ioWorkerMap erased properly
    ASSERT_EQ(ioDispatcher.SizeIOWorker(), 1);
}

TEST(IODispatcher, AddDeviceForReactor_SendSpdkEvent_Fail)
{
    // Given: Set IODispatcher, MockEventFrameworkApi
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    ON_CALL(mockEventFrameworkApi, GetFirstReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(1));
    ON_CALL(mockEventFrameworkApi, SendSpdkEvent(_, _, _)).WillByDefault(Return(false));

    // When: Call AddDeviceForReactor with nullptr
    ioDispatcher.AddDeviceForReactor(nullptr);

    // Then: Do nothing
}

TEST(IODispatcher, RemoveDeviceForReactor_SendSpdkEvent_Fail)
{
    // Given: Set IODispatcher, MockEventFrameworkApi
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    ON_CALL(mockEventFrameworkApi, GetFirstReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(1));
    ON_CALL(mockEventFrameworkApi, SendSpdkEvent(_, _, _)).WillByDefault(Return(false));

    // When: Call RemoveDeviceForReactor with nullptr
    ioDispatcher.RemoveDeviceForReactor(nullptr);

    // Then: Do nothing
}

TEST(IODispatcher, AddDeviceForIOWorker_InvalidCpuSet)
{
    // Given: Set IODispatcher, cpu_set_t, AddIOWorker
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet1, cpuSet2;
    CPU_ZERO(&cpuSet1);
    CPU_SET(3, &cpuSet1);
    CPU_ZERO(&cpuSet2);
    CPU_SET(1, &cpuSet2);
    ioDispatcher.AddIOWorker(cpuSet1);

    // When: Call AddDeviceForIOWorker with nullptr and invalid cpuSet
    ioDispatcher.AddDeviceForIOWorker(nullptr, cpuSet2);

    // Then: Do nothing
}

TEST(IODispatcher, AddDeviceForIOWorker_ValidCpuSet)
{
    // Given: Set IODispatcher, cpu_set_t, AddIOWorker, MockUBlockDevice
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(3, &cpuSet);
    ioDispatcher.AddIOWorker(cpuSet);
    auto ublockSharedPtr = std::make_shared<MockUBlockDevice>("", 4096, nullptr);
    EXPECT_CALL(*ublockSharedPtr.get(), SetDedicatedIOWorker(_)).Times(1);
    EXPECT_CALL(*ublockSharedPtr.get(), GetName()).Times(1);
    EXPECT_CALL(*ublockSharedPtr.get(), Open()).Times(1);

    // When: Call AddDeviceForIOWorker with ublockSharedPtr and valid cpuSet
    ioDispatcher.AddDeviceForIOWorker(ublockSharedPtr, cpuSet);

    // Then: Do nothing
}

TEST(IODispatcher, RemoveDeviceForIOWorker_NullIOWorker)
{
    // Given: Set IODispatcher, MockUBlockDevice
    IODispatcher ioDispatcher;
    auto ublockSharedPtr = std::make_shared<MockUBlockDevice>("", 4096, nullptr);
    ON_CALL(*ublockSharedPtr.get(), GetDedicatedIOWorker()).WillByDefault(Return(nullptr));
    EXPECT_CALL(*ublockSharedPtr.get(), GetDedicatedIOWorker()).Times(1);

    // When: Call RemoveDeviceForIOWorker with ublockSharedPtr
    ioDispatcher.RemoveDeviceForIOWorker(ublockSharedPtr);

    // Then: Do nothing
}

TEST(IODispatcher, RemoveDeviceForIOWorker_NormalIOWorker)
{
    // Given: Set IODispatcher, cpu_set_t, MockUBlockDevice, MockIOWorker
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(3, &cpuSet);
    auto ublockSharedPtr = std::make_shared<MockUBlockDevice>("", 4096, nullptr);
    EXPECT_CALL(*ublockSharedPtr.get(), GetDedicatedIOWorker()).Times(1);
    NiceMock<MockIOWorker> mockIOWorker(cpuSet, 0);
    ON_CALL(*ublockSharedPtr.get(), GetDedicatedIOWorker()).WillByDefault(Return(&mockIOWorker));
    ON_CALL(mockIOWorker, RemoveDevice(_)).WillByDefault(Return(0));

    // When: Call RemoveDeviceForIOWorker with ublockSharedPtr
    ioDispatcher.RemoveDeviceForIOWorker(ublockSharedPtr);

    // Then: Do nothing
}

TEST(IODispatcher, CompleteForThreadLocalDeviceList_Dummy)
{
    // Given: Set IODispatcher
    IODispatcher ioDispatcher;

    // When: Call CompleteForThreadLocalDeviceList
    ioDispatcher.CompleteForThreadLocalDeviceList();

    // Then: Do nothing
}

TEST(IODispatcher, Submit_ReactorAndSyncAndNotRecovery)
{
    // Given: Set IODispatcher, MockEventFrameworkApi, MockUbio, MockUBlockDevice
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    ON_CALL(mockEventFrameworkApi, IsReactorNow()).WillByDefault(Return(true));
    auto ubioSharedPtr = std::make_shared<MockUbio>(nullptr, 0, "");
    ON_CALL(*ubioSharedPtr.get(), IsSyncMode()).WillByDefault(Return(true));
    EXPECT_CALL(*ubioSharedPtr.get(), IsSyncMode()).Times(1);
    int actual, expected = -2;

    // When: Call Submit with ubioSharedPtr, true(sync) and false(isRecoveryNeeded)
    actual = ioDispatcher.Submit(ubioSharedPtr, true, false);

    // Then: Expect actual value and expected value should be same
    ASSERT_EQ(actual, expected);
}

TEST(IODispatcher, Submit_NotReactorAndSyncAndNotRecovery)
{
    // Given: Set IODispatcher, MockEventFrameworkApi, MockUbio, MockIArrayDevice
    // MockUBlockDevice, MockIOWorker
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    ON_CALL(mockEventFrameworkApi, IsReactorNow()).WillByDefault(Return(false));
    auto ubioSharedPtr = std::make_shared<MockUbio>(nullptr, 0, "");
    ON_CALL(*ubioSharedPtr.get(), SetSyncMode()).WillByDefault(Return());
    EXPECT_CALL(*ubioSharedPtr.get(), SetSyncMode()).Times(2);
    NiceMock<MockIArrayDevice> mockIArrayDevice;
    ON_CALL(*ubioSharedPtr.get(), GetArrayDev()).WillByDefault(Return(&mockIArrayDevice));
    EXPECT_CALL(*ubioSharedPtr.get(), GetArrayDev()).Times(2);
    NiceMock<MockUBlockDevice> mockUBlockDevice("", 4096, nullptr);
    ON_CALL(mockIArrayDevice, GetUblockPtr()).WillByDefault(Return(&mockUBlockDevice));
    ON_CALL(mockUBlockDevice, GetDedicatedIOWorker()).WillByDefault(Return(nullptr));
    ON_CALL(*ubioSharedPtr.get(), WaitDone()).WillByDefault(Return());
    EXPECT_CALL(*ubioSharedPtr.get(), WaitDone()).Times(2);
    int actual, expected = 0;

    // When: Call Submit with ubioSharedPtr, true(sync) and false(isRecoveryNeeded)
    // GetDedicatedIOWorker returns nullptr
    actual = ioDispatcher.Submit(ubioSharedPtr, true, false);

    // Then: Expect actual value and expected value should be same
    ASSERT_EQ(actual, expected);

    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    NiceMock<MockIOWorker> mockIOWorker(cpuSet, 0);
    ON_CALL(mockUBlockDevice, GetDedicatedIOWorker()).WillByDefault(Return(&mockIOWorker));
    EXPECT_CALL(mockIOWorker, EnqueueUbio(_)).Times(1);
    // When: Call Submit with ubioSharedPtr, true(sync) and false(isRecoveryNeeded)
    // GetDedicatedIOWorker returns mockIOWorker
    actual = ioDispatcher.Submit(ubioSharedPtr, true, false);

    // Then: Expect actual value and expected value should be same
    ASSERT_EQ(actual, expected);
}

TEST(IODispatcher, Submit_ReactorAndAsyncAndNotRecovery)
{
    // Given: Set IODispatcher, MockEventFrameworkApi, MockUbio, MockUBlockDevice
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    ON_CALL(mockEventFrameworkApi, IsReactorNow()).WillByDefault(Return(true));
    auto ubioSharedPtr = std::make_shared<MockUbio>(nullptr, 0, "");
    NiceMock<MockIArrayDevice> mockIArrayDevice;
    ON_CALL(*ubioSharedPtr.get(), GetArrayDev()).WillByDefault(Return(&mockIArrayDevice));
    EXPECT_CALL(*ubioSharedPtr.get(), GetArrayDev()).Times(1);
    NiceMock<MockUBlockDevice> mockUBlockDevice("", 4096, nullptr);
    ON_CALL(mockIArrayDevice, GetUblockPtr()).WillByDefault(Return(&mockUBlockDevice));
    ON_CALL(mockUBlockDevice, SubmitAsyncIO(_)).WillByDefault(Return(3));
    int actual, expected = 3;

    // When: Call Submit with ubioSharedPtr, false(sync) and false(isRecoveryNeeded)
    actual = ioDispatcher.Submit(ubioSharedPtr, false, false);

    // Then: Expect actual value and expected value should be same
    ASSERT_EQ(actual, expected);
}

TEST(IODispatcher, RegisterRecoveryEventFactory_Normal)
{
    // Given: Set IODispatcher
    IODispatcher ioDispatcher;

    // When: Call RegisterRecoveryEventFactory with nullptr
    ioDispatcher.RegisterRecoveryEventFactory(nullptr);

    // Then: Do nothing
}

} // namespace pos
