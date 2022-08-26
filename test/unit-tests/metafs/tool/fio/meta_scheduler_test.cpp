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

#include "src/metafs/tool/fio/meta_scheduler.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/metafs/include/metafs_mock.h"
#include "test/unit-tests/metafs/mai/metafs_io_api_mock.h"
#include <gtest/gtest.h>

using ::testing::NiceMock;
using ::testing::_;
using ::testing::Return;

namespace pos
{
// MetaIoHandler
TEST(MetaIoHandler, Create_testIfObjectWillBeCreatedCorrectly)
{
    MetaIoHandler handler(1);

    EXPECT_EQ(MetaIoHandler::index, 0);
    EXPECT_EQ(MetaIoHandler::fdList[MetaIoHandler::index], 1);
}

TEST(MetaIoHandler, Submit_testJustCallSubmitHandler)
{
    pos_io* io = new pos_io();
    iovec iov;
    io->iov = &iov;
    io->volume_id = 9;

    NiceMock<MockMetaFs> metaFs;
    NiceMock<MockMetaFsIoApi>* ioApi = new NiceMock<MockMetaFsIoApi>;
    metaFs.io = ioApi;
    EXPECT_CALL(*ioApi, SubmitIO).WillOnce(Return(EID(SUCCESS)));

    MetaIoHandler handler(1);
    handler.metaFs = &metaFs;

    EXPECT_EQ(handler.MetaFsIOSubmitHandler(io, 1), 0);
}

TEST(MetaIoHandler, Submit_testJustCallIOSubmitHandler0)
{
    pos_io* io = new pos_io();
    iovec iov;
    io->iov = &iov;
    io->volume_id = 9;

    NiceMock<MockMetaFs> metaFs;
    NiceMock<MockMetaFsIoApi>* ioApi = new NiceMock<MockMetaFsIoApi>;
    metaFs.io = ioApi;
    EXPECT_CALL(*ioApi, SubmitIO).WillOnce(Return(EID(SUCCESS)));

    MetaIoHandler handler(1);
    handler.metaFs = &metaFs;

    EXPECT_EQ(handler.IoSubmitHandler0(io), 0);
}

TEST(MetaIoHandler, Submit_testJustCallIOSubmitHandler1)
{
    pos_io* io = new pos_io();
    iovec iov;
    io->iov = &iov;
    io->volume_id = 9;

    NiceMock<MockMetaFs> metaFs;
    NiceMock<MockMetaFsIoApi>* ioApi = new NiceMock<MockMetaFsIoApi>;
    metaFs.io = ioApi;
    EXPECT_CALL(*ioApi, SubmitIO).WillOnce(Return(EID(SUCCESS)));

    MetaIoHandler handler(1);
    handler.metaFs = &metaFs;

    EXPECT_EQ(handler.IoSubmitHandler1(io), 0);
}

TEST(MetaIoHandler, Submit_testJustCallIOSubmitHandler2)
{
    pos_io* io = new pos_io();
    iovec iov;
    io->iov = &iov;
    io->volume_id = 9;

    NiceMock<MockMetaFs> metaFs;
    NiceMock<MockMetaFsIoApi>* ioApi = new NiceMock<MockMetaFsIoApi>;
    metaFs.io = ioApi;
    EXPECT_CALL(*ioApi, SubmitIO).WillOnce(Return(EID(SUCCESS)));

    MetaIoHandler handler(1);
    handler.metaFs = &metaFs;

    EXPECT_EQ(handler.IoSubmitHandler2(io), 0);
}

TEST(MetaIoHandler, Submit_testJustCallIOSubmitHandler3)
{
    pos_io* io = new pos_io();
    iovec iov;
    io->iov = &iov;
    io->volume_id = 9;

    NiceMock<MockMetaFs> metaFs;
    NiceMock<MockMetaFsIoApi>* ioApi = new NiceMock<MockMetaFsIoApi>;
    metaFs.io = ioApi;
    EXPECT_CALL(*ioApi, SubmitIO).WillOnce(Return(EID(SUCCESS)));

    MetaIoHandler handler(1);
    handler.metaFs = &metaFs;

    EXPECT_EQ(handler.IoSubmitHandler3(io), 0);
}

TEST(MetaIoHandler, Complete_testJustCallCompleteHandler)
{
    MetaIoHandler handler(1);

    handler.MetaFsIOCompleteHandler();
}

// MetaIOScheduler
TEST(MetaIOScheduler, testJustCallIoCompletion)
{
    pos_io* io = new pos_io();
    io->complete_cb = nullptr;

    MetaIOScheduler scheduler;

    scheduler.HandleMetaIoCompletion(io);
}

TEST(MetaIOScheduler, SetGetTargetFileDescriptor)
{
    MetaIOScheduler scheduler;

    scheduler.SetMetaFIOTargetFD(2);

    EXPECT_EQ(scheduler.GetMetaFIOTargetFD(), 2);
}

TEST(MetaIOScheduler, Callback_testJustCall)
{
    pos_io* io = new pos_io();
    io->volume_id = 9;

    MetaFioAIOCxt* cxt = new MetaFioAIOCxt(MetaFsIoOpcode::Write, 0, 0, 0, 4096, nullptr, nullptr, io, 10);

    MetaIOScheduler scheduler;

    scheduler.HandleIOCallback((void*)cxt);
}

// MetaFioAIOCxt
TEST(MetaFioAIOCxt, Create_testIfObjectWillBeCreatedCorrectly)
{
    pos_io* io = new pos_io();
    io->volume_id = 9;

    MetaFioAIOCxt cxt(MetaFsIoOpcode::Write, 0, 0, 0, 4096, nullptr, nullptr, io, 10);

    EXPECT_EQ(cxt.GetIBoFIOCxt(), io);
    EXPECT_EQ(cxt.GetReactor(), 10);
}
} // namespace pos
