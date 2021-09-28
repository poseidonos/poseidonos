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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "src/metafs/mai/metafs_io_api.h"
#include "test/unit-tests/metafs/mai/metafs_file_control_api_mock.h"
#include "test/unit-tests/metafs/mim/meta_io_manager_mock.h"

#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(MetaFsIoApi, Read_testIfDataWillNotBeReturnedWhenTheModuleIsAbnormal)
{
    int arrayId = 0;
    FileDescriptorType fd = 0;
    MetaStorageType type = MetaStorageType::SSD;
    NiceMock<MockMetaFsFileControlApi>* ctrl = new NiceMock<MockMetaFsFileControlApi>();

    MetaFsIoApi api(arrayId, ctrl);

    api.SetStatus(false);

    EXPECT_EQ(api.Read(fd, nullptr, type), POS_EVENT_ID::MFS_MODULE_NOT_READY);

    delete ctrl;
}

TEST(MetaFsIoApi, Read_testIfDataWillNotBeReturnedWhenThereIsNoFileInfo)
{
    int arrayId = 0;
    FileDescriptorType fd = 0;
    MetaStorageType type = MetaStorageType::SSD;
    NiceMock<MockMetaFsFileControlApi>* ctrl = new NiceMock<MockMetaFsFileControlApi>();

    MetaFsIoApi api(arrayId, ctrl);

    api.SetStatus(true);

    EXPECT_CALL(*ctrl, GetFileInfo).WillOnce(Return(nullptr));

    EXPECT_EQ(api.Read(fd, nullptr, type), POS_EVENT_ID::MFS_FILE_NOT_FOUND);

    delete ctrl;
}

TEST(MetaFsIoApi, Read_testIfDataWillBeReturned)
{
    int arrayId = 0;
    FileDescriptorType fd = 0;
    MetaStorageType type = MetaStorageType::SSD;
    MetaLpnType lpnSize = 5;
    NiceMock<MockMetaIoManager>* io = new NiceMock<MockMetaIoManager>();
    NiceMock<MockMetaFsFileControlApi>* ctrl = new NiceMock<MockMetaFsFileControlApi>(arrayId);

    MetaFsIoApi api(arrayId, ctrl, io);

    api.SetStatus(true);

    MetaFileExtent extent;
    extent.SetStartLpn(0);
    extent.SetCount(lpnSize);

    MetaFileContext fileCtx;
    fileCtx.chunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    fileCtx.extents = &extent;
    fileCtx.extentsCount = 1;
    fileCtx.fileBaseLpn = 0;
    fileCtx.isActivated = true;
    fileCtx.sizeInByte = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE * lpnSize;
    fileCtx.storageType = MetaStorageType::SSD;

    EXPECT_CALL(*ctrl, GetFileInfo).WillOnce(Return(&fileCtx));
    EXPECT_CALL(*io, CheckReqSanity).WillOnce(Return(POS_EVENT_ID::SUCCESS));
    EXPECT_CALL(*io, ProcessNewReq).WillOnce(Return(POS_EVENT_ID::SUCCESS));

    char* rBuf = (char*)malloc(fileCtx.sizeInByte);
    memset(rBuf, 0, fileCtx.sizeInByte);

    EXPECT_EQ(api.Read(fd, rBuf, type), POS_EVENT_ID::SUCCESS);

    free(rBuf);
    delete ctrl;
}

TEST(MetaFsIoApi, Write_testIfDataWillNotBeStoredWhenTheModuleIsAbnormal)
{
    int arrayId = 0;
    FileDescriptorType fd = 0;
    MetaStorageType type = MetaStorageType::SSD;
    NiceMock<MockMetaFsFileControlApi>* ctrl = new NiceMock<MockMetaFsFileControlApi>();

    MetaFsIoApi api(arrayId, ctrl);

    api.SetStatus(false);

    EXPECT_EQ(api.Write(fd, nullptr, type), POS_EVENT_ID::MFS_MODULE_NOT_READY);

    delete ctrl;
}

TEST(MetaFsIoApi, Write_testIfDataWillNotBeStoredWhenThereIsNoFileInfo)
{
    int arrayId = 0;
    FileDescriptorType fd = 0;
    MetaStorageType type = MetaStorageType::SSD;
    NiceMock<MockMetaFsFileControlApi>* ctrl = new NiceMock<MockMetaFsFileControlApi>();

    MetaFsIoApi api(arrayId, ctrl);

    api.SetStatus(true);

    EXPECT_CALL(*ctrl, GetFileInfo).WillOnce(Return(nullptr));

    EXPECT_EQ(api.Write(fd, nullptr, type), POS_EVENT_ID::MFS_FILE_NOT_FOUND);

    delete ctrl;
}

TEST(MetaFsIoApi, Write_testIfDataWillBeStored)
{
    int arrayId = 0;
    FileDescriptorType fd = 0;
    MetaStorageType type = MetaStorageType::SSD;
    MetaLpnType lpnSize = 5;
    NiceMock<MockMetaIoManager>* io = new NiceMock<MockMetaIoManager>();
    NiceMock<MockMetaFsFileControlApi>* ctrl = new NiceMock<MockMetaFsFileControlApi>(arrayId);

    MetaFsIoApi api(arrayId, ctrl, io);

    api.SetStatus(true);

    MetaFileExtent extent;
    extent.SetStartLpn(0);
    extent.SetCount(lpnSize);

    MetaFileContext fileCtx;
    fileCtx.chunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    fileCtx.extents = &extent;
    fileCtx.extentsCount = 1;
    fileCtx.fileBaseLpn = 0;
    fileCtx.isActivated = true;
    fileCtx.sizeInByte = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE * lpnSize;
    fileCtx.storageType = MetaStorageType::SSD;

    EXPECT_CALL(*ctrl, GetFileInfo).WillOnce(Return(&fileCtx));
    EXPECT_CALL(*io, CheckReqSanity).WillOnce(Return(POS_EVENT_ID::SUCCESS));
    EXPECT_CALL(*io, ProcessNewReq).WillOnce(Return(POS_EVENT_ID::SUCCESS));

    char* rBuf = (char*)malloc(fileCtx.sizeInByte);
    memset(rBuf, 0, fileCtx.sizeInByte);

    EXPECT_EQ(api.Write(fd, rBuf, type), POS_EVENT_ID::SUCCESS);

    free(rBuf);
    delete ctrl;
}
} // namespace pos
