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
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/bio/ubio_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/event_scheduler/event_factory_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
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
TEST(IODispatcher, CompleteForThreadLocalDeviceList_SimpleCall)
{
    // Given: MockEventFrameworkApi, IODispatcher, MockUBlockDevice
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, GetFirstReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, IsLastReactorNow()).WillByDefault(Return(true));
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    auto ublock = std::make_shared<MockUBlockDevice>("", 0, nullptr);
    ON_CALL(*ublock.get(), Open()).WillByDefault(Return(true));
    ON_CALL(*ublock.get(), CompleteIOs()).WillByDefault(Return(0));
    EXPECT_CALL(*ublock.get(), Open()).Times(AtLeast(1));
    EXPECT_CALL(*ublock.get(), CompleteIOs()).Times(AtLeast(1));
    ioDispatcher.AddDeviceForReactor(ublock);

    // When: Call CompleteForThreadLocalDeviceList
    IODispatcher::CompleteForThreadLocalDeviceList();

    // Then: Do nothing

    // Call RemoveDivceForReactor for restoring original state of static threadLocalDeviceList value
    ON_CALL(*ublock.get(), Close()).WillByDefault(Return(0));
    EXPECT_CALL(*ublock.get(), Close()).Times(AtLeast(1));
    ioDispatcher.RemoveDeviceForReactor(ublock);
}

TEST(IODispatcher, Submit_Reactor_Sync)
{
    // Given: IODispatcher, MockEventFrameworkApi, MockUbio
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, IsReactorNow()).WillByDefault(Return(true));
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);
    ON_CALL(*ubio.get(), IsSyncMode()).WillByDefault(Return(true));
    EXPECT_CALL(*ubio.get(), IsSyncMode()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), Complete(_)).Times(AtLeast(1));
    int actual, expected = -2;

    // When: Call Submit with MockUbio, true(sync) and false(isRecoveryNeeded)
    actual = ioDispatcher.Submit(ubio, true, false);

    // Then: Expect actual value and expected value should be same
    EXPECT_EQ(actual, expected);
}

TEST(IODispatcher, Submit_Reactor_Async_Recovery)
{
    // Given: IODispatcher, MockEventFrameworkApi, MockEventScheduler, MockEventFactory, MockUbio
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, IsReactorNow()).WillByDefault(Return(true));
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    IODispatcher ioDispatcher{&mockEventFrameworkApi, &mockEventScheduler};
    NiceMock<MockEventFactory> mockEventFactory;
    ON_CALL(mockEventFactory, Create(_)).WillByDefault(Return(nullptr));
    ioDispatcher.RegisterRecoveryEventFactory(&mockEventFactory);
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);
    int actual, expected = -1;

    // When 1: Call Submit with MockUbio, false(async) and true(isRecoveryNeeded)
    // And ubio need recovery
    ON_CALL(*ubio.get(), NeedRecovery()).WillByDefault(Return(true));
    ON_CALL(*ubio.get(), SetError(_)).WillByDefault(Return());
    EXPECT_CALL(*ubio.get(), NeedRecovery()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), SetError(_)).Times(AtLeast(1));
    actual = ioDispatcher.Submit(ubio, false, true);

    // Then 1: Expect actual value and expected value should be same
    EXPECT_EQ(actual, expected);

    // When 2: Call Submit with MockUbio, false(async) and true(isRecoveryNeeded)
    // But ubio doesn't need recovery
    NiceMock<MockUBlockDevice> mockUBlockDevice{"", 0, nullptr};
    ON_CALL(mockUBlockDevice, SubmitAsyncIO(_)).WillByDefault(Return(3));
    ON_CALL(*ubio.get(), NeedRecovery()).WillByDefault(Return(false));
    ON_CALL(*ubio.get(), GetUBlock()).WillByDefault(Return(&mockUBlockDevice));
    EXPECT_CALL(*ubio.get(), GetUBlock()).Times(AtLeast(1));
    expected = 3;
    actual = ioDispatcher.Submit(ubio, false, true);

    // Then 2: Expect actual value and expected value should be same
    EXPECT_EQ(actual, expected);
}

TEST(IODispatcher, Submit_Reactor_Async_NotRecovery)
{
    // Given: MockEventFrameworkApi, IODispatcher, MockUbio, MockUBlockDevice
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, IsReactorNow()).WillByDefault(Return(true));
    IODispatcher ioDispatcher{&mockEventFrameworkApi};
    NiceMock<MockUBlockDevice> mockUBlockDevice{"", 0, nullptr};
    ON_CALL(mockUBlockDevice, SubmitAsyncIO(_)).WillByDefault(Return(4));
    NiceMock<MockArrayDevice> mockArrayDevice{nullptr};
    ON_CALL(mockArrayDevice, GetUblockPtr()).WillByDefault(Return(&mockUBlockDevice));
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);
    ON_CALL(*ubio.get(), GetArrayDev()).WillByDefault(Return(&mockArrayDevice));
    EXPECT_CALL(*ubio.get(), GetArrayDev()).Times(AtLeast(1));
    int actual, expected = 4;

    // When: Call Submit with MockUbio, false(async) and false(isRecoveryNeeded)
    actual = ioDispatcher.Submit(ubio, false, false);

    // Then: Expect actual value and expected value should be same
    EXPECT_EQ(actual, expected);
}

TEST(IODispatcher, Submit_NotReactor_Sync_Recovery)
{
    // Given: IODispatcher, MockEventFrameworkApi, MockUBlockDevice, MockUbio
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, IsReactorNow()).WillByDefault(Return(false));
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, GetAllowedIoCount(_)).WillByDefault(Return(2));
    IODispatcher ioDispatcher{&mockEventFrameworkApi, &mockEventScheduler};

    NiceMock<MockUBlockDevice> mockUBlockDevice{"", 0, nullptr};
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);
    ON_CALL(*ubio.get(), SetSyncMode()).WillByDefault(Return());
    ON_CALL(*ubio.get(), GetUBlock()).WillByDefault(Return(&mockUBlockDevice));
    ON_CALL(*ubio.get(), WaitDone()).WillByDefault(Return());
    EXPECT_CALL(*ubio.get(), SetSyncMode()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), GetUBlock()).Times(AtLeast(1));
    EXPECT_CALL(*ubio.get(), WaitDone()).Times(AtLeast(1));
    int actual, expected = 0;

    // When 1: Call Submit with MockUbio, true(sync) and true(isRecoveryNeeded)
    // And ublock->GetDedicatedIOWorker return nullptr
    ON_CALL(mockUBlockDevice, GetDedicatedIOWorker()).WillByDefault(Return(nullptr));
    actual = ioDispatcher.Submit(ubio, true, true);

    // Then 1: Expect actual value and expected value should be same
    EXPECT_EQ(actual, expected);

    // When 2: Call Submit with MockUbio, true(sync) and true(isRecoveryNeeded)
    // And ublock->GetDedicatedIOWorker return MockIOWorker
    cpu_set_t cpuSet;
    NiceMock<MockIOWorker> mockIOWorker{cpuSet, 0};
    ON_CALL(mockIOWorker, EnqueueUbio(_)).WillByDefault(Return());
    ON_CALL(mockUBlockDevice, GetDedicatedIOWorker()).WillByDefault(Return(&mockIOWorker));
    actual = ioDispatcher.Submit(ubio, true, true);
    // Then 2: Expect actual value and expected value should be same
    EXPECT_EQ(actual, expected);
}

} // namespace pos
