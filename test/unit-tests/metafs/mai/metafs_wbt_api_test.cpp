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

#include "src/metafs/mai/metafs_wbt_api.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "test/unit-tests/metafs/mai/metafs_file_control_api_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(MetaFsWBTApi, FileList_testIfMetaFileListWillNotBeReturnedWhenTheModuleIsAbnormal)
{
    int arrayId = 0;
    MetaVolumeType volumeType = MetaVolumeType::SsdVolume;

    NiceMock<MockMetaStorageSubsystem>* storage = new NiceMock<MockMetaStorageSubsystem>(arrayId);
    NiceMock<MockMetaFsFileControlApi>* ctrl = new NiceMock<MockMetaFsFileControlApi>(arrayId, storage, nullptr);
    MetaFsWBTApi api(arrayId, ctrl);

    std::vector<MetaFileInfoDumpCxt> result;

    api.SetStatus(false);

    EXPECT_EQ(api.GetMetaFileList(result, volumeType), false);

    delete ctrl;
}

TEST(MetaFsWBTApi, FileList_testIfMetaFileListWillNotBeReturnedWhenThereIsNoMetaFile)
{
    int arrayId = 0;
    MetaVolumeType volumeType = MetaVolumeType::SsdVolume;

    NiceMock<MockMetaStorageSubsystem>* storage = new NiceMock<MockMetaStorageSubsystem>(arrayId);
    NiceMock<MockMetaFsFileControlApi>* ctrl = new NiceMock<MockMetaFsFileControlApi>(arrayId, storage, nullptr);
    MetaFsWBTApi api(arrayId, ctrl);

    std::vector<MetaFileInfoDumpCxt> original;
    std::vector<MetaFileInfoDumpCxt> result;

    api.SetStatus(true);
    EXPECT_CALL(*ctrl, Wbt_GetMetaFileList).WillOnce(Return(original));

    EXPECT_EQ(api.GetMetaFileList(result, volumeType), false);
    EXPECT_EQ(original.size(), 0);
    EXPECT_EQ(result.size(), 0);

    delete ctrl;
}

TEST(MetaFsWBTApi, FileList_testIfMetaFileListWillBeReturned)
{
    int arrayId = 0;
    MetaVolumeType volumeType = MetaVolumeType::SsdVolume;

    NiceMock<MockMetaStorageSubsystem>* storage = new NiceMock<MockMetaStorageSubsystem>(arrayId);
    NiceMock<MockMetaFsFileControlApi>* ctrl = new NiceMock<MockMetaFsFileControlApi>(arrayId, storage, nullptr);
    MetaFsWBTApi api(arrayId, ctrl);

    std::vector<MetaFileInfoDumpCxt> original;
    std::vector<MetaFileInfoDumpCxt> result;

    MetaFileInfoDumpCxt ctx1;
    MetaFileInfoDumpCxt ctx2;
    MetaFileInfoDumpCxt ctx3;

    original.push_back(ctx1);
    original.push_back(ctx2);
    original.push_back(ctx3);

    api.SetStatus(true);
    EXPECT_CALL(*ctrl, Wbt_GetMetaFileList).WillOnce(Return(original));

    EXPECT_EQ(api.GetMetaFileList(result, volumeType), true);
    EXPECT_EQ(original.size(), 3);
    EXPECT_EQ(result.size(), 3);

    delete ctrl;
}

TEST(MetaFsWBTApi, FileList_testIfMetaFileInodeWillNotBeReturnedWhenTheModuleIsAbnormal)
{
    int arrayId = 0;
    MetaVolumeType volumeType = MetaVolumeType::SsdVolume;
    std::string fileName = "TEST_FILE";

    NiceMock<MockMetaStorageSubsystem>* storage = new NiceMock<MockMetaStorageSubsystem>(arrayId);
    NiceMock<MockMetaFsFileControlApi>* ctrl = new NiceMock<MockMetaFsFileControlApi>(arrayId, storage, nullptr);
    MetaFsWBTApi api(arrayId, ctrl);

    MetaFileInodeDumpCxt result;

    api.SetStatus(false);

    EXPECT_EQ(api.GetMetaFileInode(fileName, result, volumeType), false);

    delete ctrl;
}

TEST(MetaFsWBTApi, FileList_testIfMetaFileInodeWillNotBeReturnedWhenThereIsNoMetaFile)
{
    int arrayId = 0;
    MetaVolumeType volumeType = MetaVolumeType::SsdVolume;
    std::string fileName = "TEST_FILE";

    NiceMock<MockMetaStorageSubsystem>* storage = new NiceMock<MockMetaStorageSubsystem>(arrayId);
    NiceMock<MockMetaFsFileControlApi>* ctrl = new NiceMock<MockMetaFsFileControlApi>(arrayId, storage, nullptr);
    MetaFsWBTApi api(arrayId, ctrl);

    MetaFileInodeDumpCxt result;

    api.SetStatus(true);
    EXPECT_CALL(*ctrl, Wbt_GetMetaFileInode).WillOnce(Return(nullptr));

    EXPECT_EQ(api.GetMetaFileInode(fileName, result, volumeType), false);

    delete ctrl;
}

TEST(MetaFsWBTApi, FileList_testIfMetaFileInodeWillBeReturned)
{
    int arrayId = 0;
    MetaVolumeType volumeType = MetaVolumeType::SsdVolume;
    std::string fileName = "TEST_FILE";

    NiceMock<MockMetaStorageSubsystem>* storage = new NiceMock<MockMetaStorageSubsystem>(arrayId);
    NiceMock<MockMetaFsFileControlApi>* ctrl = new NiceMock<MockMetaFsFileControlApi>(arrayId, storage, nullptr);
    MetaFsWBTApi api(arrayId, ctrl);

    FileDescriptorType expectedFd = 1;
    std::string expectedFileName = fileName;

    MetaFileInodeInfo* original = new MetaFileInodeInfo();
    original->data.field.fd = expectedFd;
    memcpy(original->data.field.fileName, fileName.c_str(), fileName.length());
    MetaFileInodeDumpCxt result;

    api.SetStatus(true);
    EXPECT_CALL(*ctrl, Wbt_GetMetaFileInode).WillOnce(Return(original));

    EXPECT_EQ(api.GetMetaFileInode(fileName, result, volumeType), true);
    EXPECT_EQ(result.inodeInfo.data.field.fd, expectedFd);
    EXPECT_EQ(std::string(result.inodeInfo.data.field.fileName), expectedFileName);

    delete ctrl;
}

} // namespace pos
