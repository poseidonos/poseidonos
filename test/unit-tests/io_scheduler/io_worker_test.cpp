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

#include "src/io_scheduler/io_worker.h"

#include <gtest/gtest.h>

#include "test/unit-tests/bio/ubio_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/device/device_detach_trigger_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/qos/qos_manager_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(IOWorker, IOWorker_Stack)
{
    // Given: cpu_set_t, MockQosManager
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IOWorkerPoller(_, _)).WillByDefault(Return(0));
    ON_CALL(mockQosManager, HandleEventUbioSubmission(_, _, _, _)).WillByDefault(Return());

    // When: Create IOWorker
    IOWorker ioWorker{cpuSet, 0, nullptr, &mockQosManager};

    // Then: Do nothing
}

TEST(IOWorker, IOWorker_Heap)
{
    // Given: cpu_set_t, MockQosManager
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IOWorkerPoller(_, _)).WillByDefault(Return(0));
    ON_CALL(mockQosManager, HandleEventUbioSubmission(_, _, _, _)).WillByDefault(Return());

    // When: Create IOWorker
    IOWorker* ioWorker = new IOWorker {cpuSet, 0, nullptr, &mockQosManager};
    delete ioWorker;

    // Then: Do nothing
}

TEST(IOWorker, GetWorkerId_SimpleCall)
{
    // Given: cpu_set_t, IOWorker, MockQosManager
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IOWorkerPoller(_, _)).WillByDefault(Return(0));
    ON_CALL(mockQosManager, HandleEventUbioSubmission(_, _, _, _)).WillByDefault(Return());
    IOWorker ioWorker{cpuSet, 4444, nullptr, &mockQosManager};
    uint32_t actual, expected = 4444;

    // When: Call GetWorkerId
    actual = ioWorker.GetWorkerId();

    // Then: Expect return 4444
    EXPECT_EQ(actual, expected);
}

TEST(IOWorker, DecreaseCurrentOutstandingIoCount_SimpleCall)
{
    // Given: cpu_set_t, IOWorker, MockQosManager
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IOWorkerPoller(_, _)).WillByDefault(Return(0));
    ON_CALL(mockQosManager, HandleEventUbioSubmission(_, _, _, _)).WillByDefault(Return());
    IOWorker ioWorker{cpuSet, 0, nullptr, &mockQosManager};

    // When: Call DecreaseCurrentOutstandingIoCount
    ioWorker.DecreaseCurrentOutstandingIoCount(1);

    // Then: Do nothing
}

TEST(IOWorker, EnqueueUbio_InputNullptr)
{
    // Given: cpu_set_t, IOWorker, MockQosManager
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IOWorkerPoller(_, _)).WillByDefault(Return(0));
    ON_CALL(mockQosManager, HandleEventUbioSubmission(_, _, _, _)).WillByDefault(Return());
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, IoEnqueued(_, _)).WillByDefault(Return());
    ON_CALL(mockEventScheduler, IoDequeued(_, _)).WillByDefault(Return());
    IOWorker ioWorker{cpuSet, 0, nullptr, &mockQosManager, &mockEventScheduler};

    // When: Call EnqueueUbio with nullptr
    ioWorker.EnqueueUbio(nullptr);

    // Then: Do nothing
}

TEST(IOWorker, AddDevice_DeviceNullptr)
{
    // Given: cpu_set_t, IOWorker, MockQosManager
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IOWorkerPoller(_, _)).WillByDefault(Return(0));
    ON_CALL(mockQosManager, HandleEventUbioSubmission(_, _, _, _)).WillByDefault(Return());
    IOWorker ioWorker{cpuSet, 0, nullptr, &mockQosManager};
    uint32_t actual, expected = 0;

    // When: Call AddDevice with nullptr
    actual = ioWorker.AddDevice(nullptr);

    // Then: Expect to return zero
    EXPECT_EQ(actual, expected);
}

TEST(IOWorker, AddDevice_DeviceMockUBlock)
{
    // Given: cpu_set_t, IOWorker, MockQosManager, MockUBlockDevice
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IOWorkerPoller(_, _)).WillByDefault(Return(0));
    ON_CALL(mockQosManager, HandleEventUbioSubmission(_, _, _, _)).WillByDefault(Return());
    IOWorker ioWorker{cpuSet, 0, nullptr, &mockQosManager};
    auto device = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*device.get(), Open()).WillByDefault(Return(true));
    EXPECT_CALL(*device.get(), GetName()).Times(AtLeast(1));
    EXPECT_CALL(*device.get(), Open()).Times(AtLeast(1));
    uint32_t actual, expected = 1;

    // When: Call AddDevice with MockUBlockDevice
    actual = ioWorker.AddDevice(device);

    // Then: Expect to return one
    EXPECT_EQ(actual, expected);
}

TEST(IOWorker, AddDevices_FourDevices)
{
    // Given: cpu_set_t, IOWorker, MockQosManager, MockDeviceDetachTrigger, MockUBlockDevice, std::vector<UblockSharedPtr>
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    NiceMock<MockDeviceDetachTrigger> mockDeviceDetachTrigger;
    ON_CALL(mockDeviceDetachTrigger, Run(_)).WillByDefault(Return());
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IOWorkerPoller(_, _)).WillByDefault(Return(0));
    ON_CALL(mockQosManager, HandleEventUbioSubmission(_, _, _, _)).WillByDefault(Return());
    IOWorker ioWorker{cpuSet, 0, &mockDeviceDetachTrigger, &mockQosManager};
    std::vector<UblockSharedPtr> devices;
    auto device1 = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*device1.get(), Open()).WillByDefault(Return(true));
    ON_CALL(*device1.get(), GetType()).WillByDefault(Return(DeviceType::SSD));
    EXPECT_CALL(*device1.get(), GetName()).Times(AtLeast(1));
    EXPECT_CALL(*device1.get(), Open()).Times(AtLeast(1));
    devices.push_back(device1);
    auto device2 = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*device2.get(), Open()).WillByDefault(Return(true));
    ON_CALL(*device2.get(), GetType()).WillByDefault(Return(DeviceType::NVRAM));
    EXPECT_CALL(*device2.get(), GetName()).Times(AtLeast(1));
    EXPECT_CALL(*device2.get(), Open()).Times(AtLeast(1));
    devices.push_back(device2);
    auto device3 = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*device3.get(), Open()).WillByDefault(Return(false));
    ON_CALL(*device3.get(), GetType()).WillByDefault(Return(DeviceType::SSD));
    EXPECT_CALL(*device3.get(), GetName()).Times(AtLeast(1));
    EXPECT_CALL(*device3.get(), Open()).Times(AtLeast(1));
    EXPECT_CALL(*device3.get(), GetType()).Times(AtLeast(1));
    devices.push_back(device3);
    auto device4 = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*device4.get(), Open()).WillByDefault(Return(false));
    ON_CALL(*device4.get(), GetType()).WillByDefault(Return(DeviceType::NVRAM));
    EXPECT_CALL(*device4.get(), GetName()).Times(AtLeast(1));
    EXPECT_CALL(*device4.get(), Open()).Times(AtLeast(1));
    EXPECT_CALL(*device4.get(), GetType()).Times(AtLeast(1));
    devices.push_back(device4);
    uint32_t actual, expected = 2;

    // When: Call AddDevices with std::vector<UblockSharedPtr>
    actual = ioWorker.AddDevices(&devices);

    // Then: Expect to return one
    EXPECT_EQ(actual, expected);
}

TEST(IOWorker, RemoveDevice_SimpleCall)
{
    // Given: cpu_set_t, IOWorker, MockQosManager, MockUBlockDevice
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IOWorkerPoller(_, _)).WillByDefault(Return(0));
    ON_CALL(mockQosManager, HandleEventUbioSubmission(_, _, _, _)).WillByDefault(Return());
    IOWorker ioWorker{cpuSet, 0, nullptr, &mockQosManager};
    auto device = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*device.get(), Open()).WillByDefault(Return(true));
    ON_CALL(*device.get(), Close()).WillByDefault(Return(0));
    EXPECT_CALL(*device.get(), GetName()).Times(AtLeast(1));
    EXPECT_CALL(*device.get(), Open()).Times(AtLeast(1));
    EXPECT_CALL(*device.get(), Close()).Times(AtLeast(1));
    ioWorker.AddDevice(device);
    uint32_t actual, expected = 0;

    // When: Call AddDevice with MockUBlockDevice
    actual = ioWorker.RemoveDevice(device);

    // Then: Expect to return one
    EXPECT_EQ(actual, expected);
}

TEST(IOWorker, Run_NormalCase)
{
    // Given: cpu_set_t, IOWorker, MockQosManager, MockUBlockDevice, MockUbio
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IOWorkerPoller(_, _)).WillByDefault(Return(0));
    ON_CALL(mockQosManager, HandleEventUbioSubmission(_, _, _, _)).WillByDefault(Return());
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, IoEnqueued(_, _)).WillByDefault(Return());
    ON_CALL(mockEventScheduler, IoDequeued(_, _)).WillByDefault(Return());
    IOWorker ioWorker{cpuSet, 0, nullptr, &mockQosManager, &mockEventScheduler};
    auto device = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*device.get(), Open()).WillByDefault(Return(true));
    ON_CALL(*device.get(), CompleteIOs()).WillByDefault(Return(1));
    EXPECT_CALL(*device.get(), GetName()).Times(AtLeast(1));
    EXPECT_CALL(*device.get(), Open()).Times(AtLeast(1));
    EXPECT_CALL(*device.get(), CompleteIOs()).Times(AtLeast(1));
    ioWorker.AddDevice(device);
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);

    // When: Call EnqueueUbio with MockUbio & sleep repeatedly for 10us until ioQueue empty
    ioWorker.EnqueueUbio(ubio);
    while (0 != ioWorker.GetQueueSize())
    {
        usleep(10);
    }

    // Then: Do nothing
}

TEST(IOWorker, Run_IOCountUnderflowCase)
{
    // Given: cpu_set_t, IOWorker, MockQosManager, MockUBlockDevice, MockUbio
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    CPU_SET(1, &cpuSet);
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQosManager, IOWorkerPoller(_, _)).WillByDefault(Return(0));
    ON_CALL(mockQosManager, HandleEventUbioSubmission(_, _, _, _)).WillByDefault(Return());
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, IoEnqueued(_, _)).WillByDefault(Return());
    ON_CALL(mockEventScheduler, IoDequeued(_, _)).WillByDefault(Return());
    IOWorker ioWorker{cpuSet, 0, nullptr, &mockQosManager, &mockEventScheduler};
    auto device = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*device.get(), Open()).WillByDefault(Return(true));
    ON_CALL(*device.get(), CompleteIOs()).WillByDefault(Return(3));
    EXPECT_CALL(*device.get(), GetName()).Times(AtLeast(1));
    EXPECT_CALL(*device.get(), Open()).Times(AtLeast(1));
    EXPECT_CALL(*device.get(), CompleteIOs()).Times(AtLeast(1));
    ioWorker.AddDevice(device);
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);

    // When: Call EnqueueUbio with MockUbio & sleep repeatedly for 10us until ioQueue empty
    ioWorker.EnqueueUbio(ubio);
    while (0 != ioWorker.GetQueueSize())
    {
        usleep(10);
    }

    // Then: Do nothing
}

} // namespace pos
