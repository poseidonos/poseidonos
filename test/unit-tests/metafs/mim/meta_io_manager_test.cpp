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

#include "src/metafs/mim/meta_io_manager.h"

#include <gtest/gtest.h>
#include <sched.h>

#include <string>

#include "test/unit-tests/metafs/mim/metafs_io_request_mock.h"
#include "test/unit-tests/metafs/mim/metafs_io_scheduler_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"

using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(MetaIoManager, CheckSuccess)
{
    MetaIoManager* mgr = new MetaIoManager();
    EXPECT_TRUE(mgr->IsSuccess(POS_EVENT_ID::SUCCESS));
    EXPECT_FALSE(mgr->IsSuccess(POS_EVENT_ID::MFS_ERROR_MESSAGE));
    delete mgr;
}

TEST(MetaIoManager, CheckInit)
{
    MetaIoManager* mgr = new MetaIoManager();
    mgr->Init();
    delete mgr;
}

TEST(MetaIoManager, CheckSanity)
{
    MockMetaFsIoRequest* req = new MockMetaFsIoRequest();
    MetaIoManager* mgr = new MetaIoManager();

    EXPECT_EQ(mgr->CheckReqSanity(*req), POS_EVENT_ID::SUCCESS);

    delete mgr;
    delete req;
}

TEST(MetaIoManager, ProcessNewReq_testIfReturnsSuccessWhenSyncIoIsEnabledAndIoHasBeenCompletedWithoutError)
{
    const int arrayId = 0;
    cpu_set_t cpuSet;
    const std::string threadName = "testThread";
    NiceMock<MockMetaFsIoScheduler>* scheduler =
        new NiceMock<MockMetaFsIoScheduler>(0, 0, 0, threadName, cpuSet, nullptr, nullptr);
    EXPECT_CALL(*scheduler, EnqueueNewReq).Times(AtLeast(1));

    MockMetaFsIoRequest* req = new MockMetaFsIoRequest();
    req->ioMode = MetaIoMode::Async;
    req->reqType = MetaIoRequestType::Read;
    EXPECT_CALL(*req, IsSyncIO).WillRepeatedly(Return(true));
    EXPECT_CALL(*req, IsIoCompleted).WillRepeatedly(Return(true));
    EXPECT_CALL(*req, GetError).WillRepeatedly(Return(false));

    NiceMock<MockMetaStorageSubsystem>* storage = new NiceMock<MockMetaStorageSubsystem>(arrayId);

    MetaIoManager* mgr = new MetaIoManager(scheduler, storage);
    mgr->Init();

    EXPECT_EQ(mgr->ProcessNewReq(*req), POS_EVENT_ID::SUCCESS);

    req->reqType = MetaIoRequestType::Write;
    EXPECT_EQ(mgr->ProcessNewReq(*req), POS_EVENT_ID::SUCCESS);

    delete mgr;
    delete req;
    delete scheduler;
}

TEST(MetaIoManager, ProcessNewReq_testIfReturnsSuccessWhenReqTypeIsReadAndIoModeIsSync)
{
    const int arrayId = 0;
    cpu_set_t cpuSet;
    const std::string threadName = "testThread";
    NiceMock<MockMetaFsIoScheduler>* scheduler =
        new NiceMock<MockMetaFsIoScheduler>(0, 0, 0, threadName, cpuSet, nullptr, nullptr);
    EXPECT_CALL(*scheduler, EnqueueNewReq).Times(AtLeast(1));

    MockMetaFsIoRequest* req = new MockMetaFsIoRequest();
    req->ioMode = MetaIoMode::Sync;
    req->reqType = MetaIoRequestType::Read;
    EXPECT_CALL(*req, IsSyncIO).WillRepeatedly(Return(true));
    EXPECT_CALL(*req, IsIoCompleted).WillRepeatedly(Return(true));
    EXPECT_CALL(*req, GetError).WillRepeatedly(Return(false));

    NiceMock<MockMetaStorageSubsystem>* storage = new NiceMock<MockMetaStorageSubsystem>(arrayId);

    MetaIoManager* mgr = new MetaIoManager(scheduler, storage);
    mgr->Init();

    EXPECT_EQ(mgr->ProcessNewReq(*req), POS_EVENT_ID::SUCCESS);

    req->reqType = MetaIoRequestType::Write;
    EXPECT_EQ(mgr->ProcessNewReq(*req), POS_EVENT_ID::SUCCESS);

    delete mgr;
    delete req;
    delete scheduler;
}

TEST(MetaIoManager, CheckArray)
{
    const int arrayId = 0;
    cpu_set_t cpuSet;
    const std::string threadName = "testThread";
    NiceMock<MockMetaFsIoScheduler>* scheduler =
        new NiceMock<MockMetaFsIoScheduler>(0, 0, 0, threadName, cpuSet, nullptr, nullptr);
    EXPECT_CALL(*scheduler, AddArrayInfo).WillRepeatedly(Return(true));
    EXPECT_CALL(*scheduler, RemoveArrayInfo).WillRepeatedly(Return(true));

    NiceMock<MockMetaStorageSubsystem>* storage = new NiceMock<MockMetaStorageSubsystem>(arrayId);

    MetaIoManager* mgr = new MetaIoManager(scheduler, storage);
    mgr->Init();

    MaxMetaLpnMapPerMetaStorage map;

    EXPECT_TRUE(mgr->AddArrayInfo(arrayId, map));
    EXPECT_TRUE(mgr->RemoveArrayInfo(arrayId));

    EXPECT_CALL(*scheduler, AddArrayInfo).WillRepeatedly(Return(false));
    EXPECT_CALL(*scheduler, RemoveArrayInfo).WillRepeatedly(Return(false));

    EXPECT_FALSE(mgr->AddArrayInfo(arrayId, map));
    EXPECT_FALSE(mgr->RemoveArrayInfo(arrayId));

    delete mgr;
    delete scheduler;
}

} // namespace pos
