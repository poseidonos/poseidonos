/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
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

#include "src/volume/volume_service.h"
#include "src/metadata/meta_updater.h"
#include "src/gc/flow_control/flow_control.h"
#include "src/gc/flow_control/flow_control_service.h"
#include "src/meta_service/meta_service.h"

#include "test/integration-tests/framework/io_generator/io_generator.h"
#include "test/integration-tests/framework/write_tester/write_tester.h"
#include "test/integration-tests/framework/write_tester/io_dispatcher_stub.h"
#include "test/unit-tests/meta_service/i_meta_updater_mock.h"
#include "test/unit-tests/allocator/stripe_manager/stripe_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/journal_manager/i_journal_manager_mock.h"
#include "test/unit-tests/journal_manager/i_journal_writer_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/metadata/meta_event_factory_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"
#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"
#include "test/unit-tests/volume/i_volume_info_manager_mock.h"
#include "test/unit-tests/volume/i_volume_manager_mock.h"
#include "test/unit-tests/gc/flow_control/flow_control_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator/i_allocator_wbt_mock.h"
#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/allocator/i_context_replayer_mock.h"
#include "test/unit-tests/io_scheduler/io_dispatcher_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/journal_manager/i_journal_status_provider_mock.h"
#include "src/event_scheduler_service/event_scheduler_service.h"
#include "src/io_dispatcher_service/io_dispatcher_Service.h"

#include <chrono>
#include <thread>
#include <gtest/gtest.h>

namespace pos
{
using IoGeneratorSmartPtr = std::shared_ptr<IoGenerator>;

class FrameworkTest : public ::testing::Test
{
public:
    FrameworkTest(void);
    virtual ~FrameworkTest(void) = default;

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    IoGenerator* CreateIoGeneratorWithFullMocks(uint32_t numOfIoThread = 1);
    IoGenerator* CreateIoGeneratorSchedulerMock(uint32_t numOfIoThread = 1);

    NiceMock<MockIODispatcher>* mockIODispatcher;
    NiceMock<MockEventScheduler>* mockEventScheduler;
    NiceMock<MockIMetaUpdater>* mockMetaUpdater;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIJournalManager> journal;
    NiceMock<MockIJournalWriter> journalWriter;

    NiceMock<MockMetaEventFactory> metaEventFactory;
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIVolumeManager>* mockVolumeManager;
    NiceMock<MockFlowControl>* mockFlowControl;
    NiceMock<MockIBlockAllocator>* mockIBlockAllocator;
    NiceMock<MockIWBStripeAllocator>* mockIWBStripeAllocator;
    NiceMock<MockIAllocatorWbt>* mockIAllocatorWbt;
    NiceMock<MockIContextManager>* mockIContextManager;
    NiceMock<MockIContextReplayer>* mockIContextReplayer;
    NiceMock<MockIJournalStatusProvider> mockJournalStatus;

    MetaUpdater* metaUpdater;
    IODispatcherStub* ioDispatcherStub;

    WriteTester* writeTester;
};

FrameworkTest::FrameworkTest(void)
: metaUpdater(nullptr),
  ioDispatcherStub(nullptr),
  writeTester(nullptr)
{
}

void
FrameworkTest::SetUp(void)
{
    IoDispatcherServiceSingleton::Instance()->Unregister();
    EventSchedulerServiceSingleton::Instance()->Unregister();
    EventSchedulerSingleton::ResetInstance();
    IODispatcherSingleton::ResetInstance();

    mockEventScheduler = new NiceMock<MockEventScheduler>;
    EventSchedulerServiceSingleton::Instance()->Register(mockEventScheduler);

    mockIODispatcher = new NiceMock<MockIODispatcher>;

    mockMetaUpdater = new NiceMock<MockIMetaUpdater>;
    mockVolumeManager = new NiceMock<MockIVolumeManager>;
    mockIBlockAllocator = new NiceMock<MockIBlockAllocator>;
    mockIWBStripeAllocator = new NiceMock<MockIWBStripeAllocator>;
    mockIAllocatorWbt = new NiceMock<MockIAllocatorWbt>;
    mockIContextManager = new NiceMock<MockIContextManager>;
    mockIContextReplayer = new NiceMock<MockIContextReplayer>;
    mockFlowControl = new NiceMock<MockFlowControl>(nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr);

    // Test 시작 시에, Test 범위에 따라서 dependency injection 필요.
    metaUpdater = new MetaUpdater(&stripeMap, &journal, &journalWriter,
        mockEventScheduler, &metaEventFactory, &arrayInfo);

    ioDispatcherStub = new IODispatcherStub(mockEventScheduler);
    IoDispatcherServiceSingleton::Instance()->Register(ioDispatcherStub);

    IODispatcher* tempIoDispatcher = IoDispatcherServiceSingleton::Instance()->GetIODispatcher();
    assert(tempIoDispatcher == ioDispatcherStub);

    CallbackSmartPtr metaCallback(new NiceMock<MockCallback>(true));
    ON_CALL(metaEventFactory, CreateBlockMapUpdateEvent).WillByDefault(Return(metaCallback));
    ON_CALL(journal, IsEnabled).WillByDefault(Return(true));
    ON_CALL(*mockFlowControl, GetToken(_, _)).WillByDefault(Return((512 >> SECTOR_SIZE_SHIFT) * Ubio::BYTES_PER_UNIT));

    int arrayId = 0;
    std::string arrayName = "POSArray";
    VolumeServiceSingleton::Instance()->Register(arrayId, mockVolumeManager);
    FlowControlServiceSingleton::Instance()->Register(arrayName, mockFlowControl);
    AllocatorServiceSingleton::Instance()->RegisterAllocator(arrayName, arrayId, mockIBlockAllocator,
        mockIWBStripeAllocator, mockIAllocatorWbt, mockIContextManager, mockIContextReplayer);

    MetaServiceSingleton::Instance()->Register(arrayName, arrayId, metaUpdater, &mockJournalStatus);

    ON_CALL(*mockVolumeManager, GetArrayName()).WillByDefault(Return(arrayName));

    ON_CALL(*mockVolumeManager, IsWriteThroughEnabled()).WillByDefault(Return(false));
    ON_CALL(*mockIBlockAllocator, TryRdLock(_)).WillByDefault(Return(true));
}

void
FrameworkTest::TearDown(void)
{
    // clean up all allocated objectect in this class


    delete mockIODispatcher;

    delete mockMetaUpdater;
    delete mockVolumeManager;
    delete mockIWBStripeAllocator;
    delete mockIBlockAllocator;
    delete mockIAllocatorWbt;
    delete mockIContextManager;
    delete mockIContextReplayer;
    delete mockFlowControl;

    if (nullptr != writeTester)
    {
        delete writeTester;
        writeTester = nullptr;
    }

    delete metaUpdater;

    int arrayId = 0;
    std::string arrayName = "POSArray";
    VolumeServiceSingleton::Instance()->Unregister(arrayId);
    FlowControlServiceSingleton::Instance()->UnRegister(arrayName);
    AllocatorServiceSingleton::Instance()->UnregisterAllocator(arrayName);
    MetaServiceSingleton::Instance()->Unregister(arrayName);
    IoDispatcherServiceSingleton::Instance()->Unregister();
    EventSchedulerServiceSingleton::Instance()->Unregister();

    VolumeServiceSingleton::ResetInstance();
    FlowControlServiceSingleton::ResetInstance();
    AllocatorServiceSingleton::ResetInstance();
    MetaServiceSingleton::ResetInstance();
    EventSchedulerSingleton::ResetInstance();
    IODispatcherSingleton::ResetInstance();
}

IoGenerator*
FrameworkTest::CreateIoGeneratorWithFullMocks(uint32_t numOfIoThread)
{
    writeTester = new WriteTester();
    return (new IoGenerator(writeTester, numOfIoThread));
}

IoGenerator*
FrameworkTest::CreateIoGeneratorSchedulerMock(uint32_t numOfIoThread)
{
    writeTester = new WriteTester();
    return (new IoGenerator(writeTester, numOfIoThread));
}

TEST_F(FrameworkTest, Sequential_WriteIoGeneration_BlockMapUpdate)
{
    IoGeneratorSmartPtr ioGen(CreateIoGeneratorSchedulerMock());

    uint32_t volId = 0;
    uint32_t startOffset = 0;
    uint32_t numOfIo = 10;
    uint32_t blockSize = 8;

    EXPECT_CALL(metaEventFactory, CreateBlockMapUpdateEvent).Times(numOfIo);
    EXPECT_CALL(journalWriter, AddBlockMapUpdatedLog).Times(numOfIo);

    ioGen->SetConfiguration(volId, startOffset, numOfIo, IO_GEN_TYPE_SEQUENTIAL, blockSize);
    ioGen->GenerateWriteIo(true);
}

TEST_F(FrameworkTest, Sequential_WriteIoGeneration)
{
    IoGeneratorSmartPtr ioGen(CreateIoGeneratorSchedulerMock());

    uint32_t volId = 0;
    uint32_t startOffset = 0;
    uint32_t numOfIo = 10;
    uint32_t blockSize = 8;

    ioGen->SetConfiguration(volId, startOffset, numOfIo, IO_GEN_TYPE_SEQUENTIAL, blockSize);
    ioGen->GenerateWriteIo(true);
}

TEST_F(FrameworkTest, Random_WriteIoGeneration)
{
    IoGeneratorSmartPtr ioGen(CreateIoGeneratorSchedulerMock());

    uint32_t volId = 0;
    uint32_t startOffset = 0;
    uint32_t numOfIo = 512;
    uint32_t blockSize = 8;

    ioGen->SetConfiguration(volId, startOffset, numOfIo, IO_GEN_TYPE_RANDOM, blockSize);
    ioGen->GenerateWriteIo(true);
}
} // namespace pos