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

#include "src/metafs/mim/mio_handler.h"
#include "src/metafs/mim/mpio_handler.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"
#include "test/unit-tests/metafs/lib/metafs_pool_mock.h"
#include "test/unit-tests/metafs/mim/metafs_io_multilevel_q_mock.h"
#include "test/unit-tests/metafs/mim/mpio_allocator_mock.h"
#include "test/unit-tests/metafs/mim/mpio_handler_mock.h"
#include "test/unit-tests/metafs/mim/metafs_io_request_mock.h"
#include "test/unit-tests/metafs/mim/mfs_io_range_overlap_chker_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"
#include "test/unit-tests/metafs/include/metafs_mock.h"
#include "test/unit-tests/metafs/mai/metafs_file_control_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_io_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_management_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_wbt_api_mock.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::AtLeast;

namespace pos
{
class MioHandlerTestFixture : public ::testing::Test
{
public:
    MioHandlerTestFixture(void)
    : ioSQ(nullptr),
      ioCQ(nullptr),
      doneQ(nullptr),
      bottomhalfHandler(nullptr),
      mioPool(nullptr),
      mpioAllocator(nullptr),
      arrayInfo(nullptr),
      mgmt(nullptr),
      ctrl(nullptr),
      wbt(nullptr),
      io(nullptr),
      metaFs(nullptr)
    {
    }

    virtual ~MioHandlerTestFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        const uint32_t POOL_SIZE = 1024;

        tp = new NiceMock<MockTelemetryPublisher>;

        ioSQ = new NiceMock<MockMetaFsIoMultilevelQ<MetaFsIoRequest*, RequestPriority>>;
        ioCQ = new NiceMock<MockMetaFsIoMultilevelQ<Mio*, RequestPriority>>;
        doneQ = new MockMetaFsIoMultilevelQ<Mpio*, RequestPriority>();
        bottomhalfHandler = new NiceMock<MockMpioHandler>(0, 0, nullptr, doneQ);
        mpioAllocator = new NiceMock<MockMpioAllocator>(POOL_SIZE);
        mioPool = new NiceMock<MockMetaFsPool<Mio*>>(POOL_SIZE);
        arrayInfo = new MockIArrayInfo();
        EXPECT_CALL(*arrayInfo, GetName).WillRepeatedly(Return("TESTARRAY"));
        EXPECT_CALL(*arrayInfo, GetIndex).WillRepeatedly(Return(0));

        mss = new NiceMock<MockMetaStorageSubsystem>(arrayInfo->GetIndex());

        mgmt = new MockMetaFsManagementApi(arrayInfo->GetIndex(), mss);
        ctrl = new MockMetaFsFileControlApi(arrayInfo->GetIndex(), mss);
        wbt = new MockMetaFsWBTApi(arrayInfo->GetIndex(), ctrl);
        io = new MockMetaFsIoApi(arrayInfo->GetIndex(), ctrl, mss, tp);

        metaFs = new MockMetaFs(arrayInfo, false, mgmt, ctrl, io, wbt, mss, nullptr);

        handler = new MioHandler(0, 0, ioSQ, ioCQ, mpioAllocator, mioPool, tp);
    }

    virtual void
    TearDown(void)
    {
        delete handler;
        delete metaFs;
        delete arrayInfo;
        delete bottomhalfHandler;
        delete tp;
    }

protected:
    MioHandler* handler;

    NiceMock<MockMetaFsIoMultilevelQ<MetaFsIoRequest*, RequestPriority>>* ioSQ;
    NiceMock<MockMetaFsIoMultilevelQ<Mio*, RequestPriority>>* ioCQ;
    MockMetaFsIoMultilevelQ<Mpio*, RequestPriority>* doneQ;
    NiceMock<MockMpioHandler>* bottomhalfHandler;
    NiceMock<MockMetaFsPool<Mio*>>* mioPool;
    NiceMock<MockMpioAllocator>* mpioAllocator;
    NiceMock<MockMetaStorageSubsystem>* mss;
    NiceMock<MockTelemetryPublisher>* tp;

    MockIArrayInfo* arrayInfo;
    MockMetaFsManagementApi* mgmt;
    MockMetaFsFileControlApi* ctrl;
    MockMetaFsWBTApi* wbt;
    MockMetaFsIoApi* io;

    MockMetaFs* metaFs;
};

TEST_F(MioHandlerTestFixture, Normal)
{
    const int MAX_COUNT = 200;

    bool result = false;

    MockMetaFsIoRequest* msg = new MockMetaFsIoRequest();
    msg->reqType = MetaIoRequestType::Read;

    EXPECT_CALL(*ioSQ, Enqueue).Times(AtLeast(1));
    EXPECT_CALL(*ioSQ, Dequeue).WillRepeatedly(Return(msg));
    EXPECT_CALL(*ctrl, GetMaxMetaLpn).WillRepeatedly(Return(100));

    handler->BindPartialMpioHandler(bottomhalfHandler);
    result = handler->AddArrayInfo(arrayInfo->GetIndex());
    EXPECT_TRUE(result);

    for (int i = 0; i < MAX_COUNT; i++)
    {
        handler->EnqueueNewReq(msg);
    }

    for (int i = 0; i < MAX_COUNT; i++)
    {
        handler->TophalfMioProcessing();
    }

    result = handler->RemoveArrayInfo(arrayInfo->GetIndex());
    EXPECT_TRUE(result);

    delete msg;
}

TEST_F(MioHandlerTestFixture, Normal_PushToRetryQueue)
{
    const int MAX_COUNT = 5;

    bool result = false;
    MockMetaFsIoRangeOverlapChker* checker = new MockMetaFsIoRangeOverlapChker();

    MockMetaFsIoRequest* msg = new MockMetaFsIoRequest();
    msg->reqType = MetaIoRequestType::Write;
    msg->arrayId = 0;
    msg->targetMediaType = MetaStorageType::SSD;

    EXPECT_CALL(*ioSQ, Enqueue).Times(AtLeast(1));
    EXPECT_CALL(*checker, IsRangeOverlapConflicted).WillRepeatedly(Return(true));

    handler->BindPartialMpioHandler(bottomhalfHandler);
    result = handler->AddArrayInfo(arrayInfo->GetIndex(), MetaStorageType::SSD, checker);
    EXPECT_TRUE(result);

    for (int i = 0; i < MAX_COUNT; i++)
    {
        handler->EnqueueNewReq(msg);
    }

    for (int i = 0; i < MAX_COUNT; i++)
    {
        handler->TophalfMioProcessing();
    }

    delete msg;
}

TEST_F(MioHandlerTestFixture, Repeat_AddAndRemoveArray)
{
    const int MAX_COUNT = 200;
    bool result = false;

    EXPECT_CALL(*arrayInfo, GetName).WillRepeatedly(Return("TESTARRAY"));
    EXPECT_CALL(*ctrl, GetMaxMetaLpn).WillRepeatedly(Return(100));
    EXPECT_CALL(*mgmt, IsValidVolume).WillRepeatedly(Return(true));

    result = handler->AddArrayInfo(arrayInfo->GetIndex());
    EXPECT_TRUE(result);

    result = handler->RemoveArrayInfo(arrayInfo->GetIndex());
    EXPECT_TRUE(result);

    result = handler->RemoveArrayInfo(arrayInfo->GetIndex());
    EXPECT_FALSE(result);
}

} // namespace pos
