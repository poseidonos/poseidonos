/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <thread>

#include "src/device/base/ublock_device.h"
#include "src/event_scheduler/event_factory.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/io_completer.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/i_array_device.h"
#include "src/include/pos_event_id.hpp"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/io_scheduler/io_worker.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/accel_engine_api.h"
#include "test/unit-tests/bio/ubio_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/event_scheduler/event_factory_mock.h"
#include "test/unit-tests/include/i_array_device_mock.h"
#include "test/unit-tests/io_scheduler/io_worker_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::AtLeast;
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
    EXPECT_EQ(ioDispatcher.SizeIOWorker(), 0);
}

TEST(IODispatcher, AddIOWorker_ValidCPUSet)
{
    // Given: Set IODispatcher, cpu_set_t
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    CPU_SET(3, &cpuSet);

    // When: Call AddIOWorker with valid cpu_set_t
    ioDispatcher.AddIOWorker(cpuSet);

    // Then: Expect ioWorkerMap inserted properly
    EXPECT_EQ(ioDispatcher.SizeIOWorker(), 2);
}

TEST(IODispatcher, AddIOWorker_IntersectionCPUSet)
{
    // Given: Set IODispatcher, cpu_set_t
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet1, cpuSet2;
    CPU_ZERO(&cpuSet1);
    CPU_SET(1, &cpuSet1);
    CPU_SET(3, &cpuSet1);
    CPU_ZERO(&cpuSet2);
    CPU_SET(3, &cpuSet2);
    CPU_SET(5, &cpuSet2);

    // When: Call twice AddIOWorker with intersection cpu_set_t
    ioDispatcher.AddIOWorker(cpuSet1);
    ioDispatcher.AddIOWorker(cpuSet2);

    // Then: Expect ioWorkerMap inserted properly
    EXPECT_EQ(ioDispatcher.SizeIOWorker(), 3);
}

TEST(IODispatcher, RemoveIOWorker_ZeroCPUSet)
{
    // Given: Set IODispatcher, cpu_set_t, AddIOWorker
    IODispatcher ioDispatcher;
    cpu_set_t cpuSetAdd, cpuSetRemove;
    CPU_ZERO(&cpuSetAdd);
    CPU_ZERO(&cpuSetRemove);
    CPU_SET(1, &cpuSetAdd);
    ioDispatcher.AddIOWorker(cpuSetAdd);

    // When: Call RemoveIOWorker with zero cpu_set_t
    ioDispatcher.RemoveIOWorker(cpuSetRemove);

    // Then: Expect ioWorkerMap skip erase
    EXPECT_EQ(ioDispatcher.SizeIOWorker(), 1);
}

TEST(IODispatcher, RemoveIOWorker_ValidCPUSet)
{
    // Given: Set IODispatcher, cpu_set_t, AddIOWorker
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    CPU_SET(3, &cpuSet);
    ioDispatcher.AddIOWorker(cpuSet);

    // When: Call RemoveIOWorker with valid cpu_set_t
    ioDispatcher.RemoveIOWorker(cpuSet);

    // Then: Expect ioWorkerMap erased properly
    EXPECT_EQ(ioDispatcher.SizeIOWorker(), 0);
}

TEST(IODispatcher, RemoveIOWorker_IntersectionCPUSet)
{
    // Given: Set IODispatcher, cpu_set_t, AddIOWorker
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet1, cpuSet2;
    CPU_ZERO(&cpuSet1);
    CPU_SET(1, &cpuSet1);
    CPU_SET(3, &cpuSet1);
    CPU_ZERO(&cpuSet2);
    CPU_SET(3, &cpuSet2);
    CPU_SET(5, &cpuSet2);
    ioDispatcher.AddIOWorker(cpuSet1);

    // When: Call RemoveIOWorker with intersection cpu_set_t
    ioDispatcher.RemoveIOWorker(cpuSet2);

    // Then: Expect ioWorkerMap erased properly
    EXPECT_EQ(ioDispatcher.SizeIOWorker(), 1);
}

TEST(IODispatcher, SizeIOWorker_SimpleCall)
{
    // Given: Set IODispatcher, cpu_set_t, AddIOWorker
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);

    // When 1: Call SizeIOWorker with empty ioWorker
    // Then 1: Expect to return zero
    EXPECT_EQ(ioDispatcher.SizeIOWorker(), 0);

    // When 2: Call SizeIOWorker with one ioWorker
    ioDispatcher.AddIOWorker(cpuSet);

    // Then 2: Expect to return one
    EXPECT_EQ(ioDispatcher.SizeIOWorker(), 1);
}

TEST(IODispatcher, AddDeviceForReactor_NotFirstReactorCore_SendEventFail)
{
    // Given: MockEventFrameworkApi, IODispatcher, MockUBlockDevice
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, GetFirstReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(1));
    ON_CALL(mockEventFrameworkApi, SendSpdkEvent(_, _, _)).WillByDefault(Return(false));
    AccelEngineApi::SetReactorCount(1);
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    auto ublock = std::make_shared<MockUBlockDevice>("", 0, nullptr);

    // When: Call AddDeviceForReactor with MockUBlockDevice
    ioDispatcher.AddDeviceForReactor(ublock);

    // Then: Do nothing
}

TEST(IODispatcher, AddDeviceForReactor_NotFirstReactorCore_SendEventSuccess)
{
    // Given: MockEventFrameworkApi, IODispatcher, MockUBlockDevice, t
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, GetFirstReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(1));
    ON_CALL(mockEventFrameworkApi, SendSpdkEvent(_, _, _)).WillByDefault(Return(false));
    AccelEngineApi::SetReactorCount(1);
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    auto ublock = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    std::thread t([&](void) -> void {
        usleep(100000); // 100ms, expect to reach while loop in _CallForFrontend
        IODispatcher::SetFrontendDone(true);
    });

    // When: Call AddDeviceForReactor with MockUBlockDevice
    ioDispatcher.AddDeviceForReactor(ublock);
    t.join();

    // Then: Do nothing
}

TEST(IODispatcher, AddDeviceForReactor_FirstReactorCore_LastReactor)
{
    // Given: MockEventFrameworkApi, IODispatcher, MockUBlockDevice
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, GetFirstReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, IsLastReactorNow()).WillByDefault(Return(true));
    AccelEngineApi::SetReactorCount(1);
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    auto ublock = std::make_shared<MockUBlockDevice>("", 0, nullptr);

    // When 1: Call AddDeviceForReactor with Opened BlockDevice
    ON_CALL(*ublock.get(), Open()).WillByDefault(Return(true));
    EXPECT_CALL(*ublock.get(), Open()).Times(AtLeast(1));
    ioDispatcher.AddDeviceForReactor(ublock);

    // When 2: Call AddDeviceForReactor with Closed Non-SSD BlockDevice
    ON_CALL(*ublock.get(), Open()).WillByDefault(Return(false));
    ON_CALL(*ublock.get(), GetType()).WillByDefault(Return(DeviceType::NVRAM));
    EXPECT_CALL(*ublock.get(), GetType()).Times(AtLeast(1));
    ioDispatcher.AddDeviceForReactor(ublock);

    // When 3: Call AddDeviceForReactor with Closed SSD BlockDevice
    ON_CALL(*ublock.get(), Open()).WillByDefault(Return(false));
    ON_CALL(*ublock.get(), GetType()).WillByDefault(Return(DeviceType::SSD));
    EXPECT_CALL(*ublock.get(), GetSN()).Times(AtLeast(1));
    ioDispatcher.AddDeviceForReactor(ublock);

    // Then: Do nothing

    // Call RemoveDivceForReactor for restoring original state of static threadLocalDeviceList value
    ON_CALL(*ublock.get(), Close()).WillByDefault(Return(0));
    EXPECT_CALL(*ublock.get(), Close()).Times(AtLeast(1));
    ioDispatcher.RemoveDeviceForReactor(ublock);
}

TEST(IODispatcher, AddDeviceForReactor_FirstReactorCore_NotLastReactor_SendEventFail)
{
    // Given: MockEventFrameworkApi, IODispatcher, MockUBlockDevice
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, GetFirstReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, IsLastReactorNow()).WillByDefault(Return(false));
    ON_CALL(mockEventFrameworkApi, GetNextReactor()).WillByDefault(Return(0));
    AccelEngineApi::SetReactorCount(1);
    ON_CALL(mockEventFrameworkApi, SendSpdkEvent(_, _, _)).WillByDefault([this](uint32_t core, EventFuncOneParam func, void* arg1) -> bool {
        delete static_cast<UblockSharedPtr*>(arg1);
        return false;
    });
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    auto ublock = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*ublock.get(), Open()).WillByDefault(Return(true));
    EXPECT_CALL(*ublock.get(), Open()).Times(AtLeast(1));

    // When: Call AddDeviceForReactor with SendEvent Fail
    ioDispatcher.AddDeviceForReactor(ublock);

    // Then: Do nothing

    // Call RemoveDivceForReactor for restoring original state of static threadLocalDeviceList value
    ON_CALL(*ublock.get(), Close()).WillByDefault(Return(0));
    EXPECT_CALL(*ublock.get(), Close()).Times(AtLeast(1));
    ioDispatcher.RemoveDeviceForReactor(ublock);
}

TEST(IODispatcher, AddDeviceForReactor_FirstReactorCore_NotLastReactor_SendEventSuccess)
{
    // Given: MockEventFrameworkApi, IODispatcher, MockUBlockDevice
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, GetFirstReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, IsLastReactorNow()).WillByDefault(Return(false));
    ON_CALL(mockEventFrameworkApi, GetNextReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, SendSpdkEvent(_, _, _)).WillByDefault([this](uint32_t core, EventFuncOneParam func, void* arg1) -> bool {
        delete static_cast<UblockSharedPtr*>(arg1);
        return true;
    });
    AccelEngineApi::SetReactorCount(1);
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    auto ublock = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*ublock.get(), Open()).WillByDefault(Return(true));
    EXPECT_CALL(*ublock.get(), Open()).Times(AtLeast(1));
    std::thread t([&](void) -> void {
        usleep(100000); // 100ms, expect to reach while loop in _CallForFrontend
        IODispatcher::SetFrontendDone(true);
    });

    // When: Call AddDeviceForReactor with SendEvent Success
    ioDispatcher.AddDeviceForReactor(ublock);
    t.join();

    // Then: Do nothing

    // Call RemoveDivceForReactor for restoring original state of static threadLocalDeviceList value
    ON_CALL(*ublock.get(), Close()).WillByDefault(Return(0));
    EXPECT_CALL(*ublock.get(), Close()).Times(AtLeast(1));
    ON_CALL(mockEventFrameworkApi, IsLastReactorNow()).WillByDefault(Return(true));
    ioDispatcher.RemoveDeviceForReactor(ublock);
}

TEST(IODispatcher, RemoveDeviceForReactor_FirstReactorCore_LastReactor)
{
    // Given: MockEventFrameworkApi, IODispatcher, MockUBlockDevice
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, GetFirstReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, IsLastReactorNow()).WillByDefault(Return(true));
    AccelEngineApi::SetReactorCount(1);
    IODispatcher ioDispatcher{&mockEventFrameworkApi};

    auto ublock1 = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*ublock1.get(), Open()).WillByDefault(Return(true));
    ON_CALL(*ublock1.get(), Close()).WillByDefault(Return(0));
    EXPECT_CALL(*ublock1.get(), Open()).Times(AtLeast(1));
    EXPECT_CALL(*ublock1.get(), Close()).Times(AtLeast(1));
    ioDispatcher.AddDeviceForReactor(ublock1);

    auto ublock2 = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*ublock2.get(), Open()).WillByDefault(Return(true));
    ON_CALL(*ublock2.get(), Close()).WillByDefault(Return(0));
    EXPECT_CALL(*ublock2.get(), Open()).Times(AtLeast(1));
    EXPECT_CALL(*ublock2.get(), Close()).Times(AtLeast(1));
    ioDispatcher.AddDeviceForReactor(ublock2);

    // When: Call RemoveDeviceForReactor
    ioDispatcher.RemoveDeviceForReactor(ublock2);
    ioDispatcher.RemoveDeviceForReactor(ublock1);
}

TEST(IODispatcher, AddDeviceForIOWorker_ZeroCPUSet)
{
    // Given: IODispatcher, cpu_set_t
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);

    // When: Call AddDeviceForIOWorker with zero cpuSet
    ioDispatcher.AddDeviceForIOWorker(nullptr, cpuSet);

    // Then: Do nothing
}

TEST(IODispatcher, AddDeviceForIOWorker_MissMatchedCPUSet)
{
    // Given: IODispatcher, cpu_set_t
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet1, cpuSet2;
    CPU_ZERO(&cpuSet1);
    CPU_SET(3, &cpuSet1);
    CPU_ZERO(&cpuSet2);
    CPU_SET(1, &cpuSet2);
    ioDispatcher.AddIOWorker(cpuSet1);

    // When: Call AddDeviceForIOWorker with nullptr and miss-matched cpuSet
    ioDispatcher.AddDeviceForIOWorker(nullptr, cpuSet2);

    // Then: Do nothing
}

TEST(IODispatcher, AddDeviceForIOWorker_MatchedCpuSet)
{
    // Given: IODispatcher, cpu_set_t, MockUBlockDevice
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(3, &cpuSet);
    ioDispatcher.AddIOWorker(cpuSet);
    auto ublock = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    EXPECT_CALL(*ublock.get(), SetDedicatedIOWorker(_)).Times(AtLeast(1));
    EXPECT_CALL(*ublock.get(), GetName()).Times(AtLeast(1));
    EXPECT_CALL(*ublock.get(), Open()).Times(AtLeast(1));
    EXPECT_CALL(*ublock.get(), GetType()).Times(AtLeast(1));
    EXPECT_CALL(*ublock.get(), GetSN()).Times(AtLeast(1));

    // When: Call AddDeviceForIOWorker with MockUBlockDevice and matched cpuSet
    ioDispatcher.AddDeviceForIOWorker(ublock, cpuSet);

    // Then: Do nothing
}

TEST(IODispatcher, RemoveDeviceForIOWorker_NullIOWorker)
{
    // Given: Set IODispatcher, MockUBlockDevice
    IODispatcher ioDispatcher;
    auto ublock = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*ublock.get(), GetDedicatedIOWorker()).WillByDefault(Return(nullptr));
    EXPECT_CALL(*ublock.get(), GetDedicatedIOWorker()).Times(1);

    // When: Call RemoveDeviceForIOWorker with MockUBlockDevice
    ioDispatcher.RemoveDeviceForIOWorker(ublock);

    // Then: Do nothing
}

TEST(IODispatcher, RemoveDeviceForIOWorker_NormalIOWorker)
{
    // Given: Set IODispatcher, cpu_set_t, MockUBlockDevice, MockIOWorker
    IODispatcher ioDispatcher;
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(3, &cpuSet);
    NiceMock<MockIOWorker> mockIOWorker(cpuSet, 0);
    ON_CALL(mockIOWorker, RemoveDevice(_)).WillByDefault(Return(0));
    auto ublock = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*ublock.get(), GetDedicatedIOWorker()).WillByDefault(Return(&mockIOWorker));
    EXPECT_CALL(*ublock.get(), GetDedicatedIOWorker()).Times(1);

    // When: Call RemoveDeviceForIOWorker with MockUBlockDevice
    ioDispatcher.RemoveDeviceForIOWorker(ublock);

    // Then: Do nothing
}

} // namespace pos
