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

#include "src/metafs/mai/metafs_file_control_api.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "test/unit-tests/lib/bitmap_mock.h"
#include "test/unit-tests/metafs/mai/meta_file_context_handler_mock.h"
#include "test/unit-tests/metafs/mai/metafs_management_api_mock.h"
#include "test/unit-tests/metafs/mvm/meta_volume_manager_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class MetaFsFileControlApiFixture : public ::testing::Test
{
public:
    MetaFsFileControlApiFixture(void)
    {
    }

    virtual ~MetaFsFileControlApiFixture(void)
    {
    }

    virtual void SetUp(void)
    {
        storage = new NiceMock<MockMetaStorageSubsystem>(ARRAY_ID);
        mgmt = new NiceMock<MockMetaFsManagementApi>(ARRAY_ID, storage);
        volMgr = new NiceMock<MockMetaVolumeManager>(ARRAY_ID, storage);
        bitmap = new NiceMock<MockBitMap>(100);
        handler = std::make_unique<NiceMock<MockMetaFileContextHandler>>(0, nullptr, nullptr);
        ON_CALL(*handler, AddFileContext).WillByDefault(Return());

        api = new MetaFsFileControlApi(ARRAY_ID, true, storage, mgmt, volMgr, bitmap, std::move(handler), nullptr);
    }

    virtual void TearDown(void)
    {
        delete storage;
        delete mgmt;
        delete api;
    }

protected:
    MetaFsFileControlApi* api;
    NiceMock<MockMetaStorageSubsystem>* storage;
    NiceMock<MockMetaFsManagementApi>* mgmt;
    NiceMock<MockMetaVolumeManager>* volMgr;
    NiceMock<MockBitMap>* bitmap;
    std::unique_ptr<NiceMock<MockMetaFileContextHandler>> handler;

    const int ARRAY_ID = 0;
    const int FILE_DESCRIPTOR = 0;
    const MetaVolumeType VOLUME_TYPE = MetaVolumeType::SsdVolume;
};

TEST_F(MetaFsFileControlApiFixture, Wbt_GetMetaFileList_testIfMetaFileInfoListCanBeReturned)
{
    const size_t EXPECT_VECTOR_SIZE = 1;
    EXPECT_CALL(*volMgr, HandleNewRequest).WillOnce([](MetaFsRequestBase& reqMsg) -> POS_EVENT_ID {
        auto listPtr = static_cast<MetaFsFileControlRequest&>(reqMsg).completionData.fileInfoListPointer;
        listPtr->push_back(MetaFileInfoDumpCxt());
        return EID(SUCCESS);
    });

    auto result = api->Wbt_GetMetaFileList(VOLUME_TYPE);
    EXPECT_EQ(result.size(), EXPECT_VECTOR_SIZE);
}

TEST_F(MetaFsFileControlApiFixture, Wbt_GetMetaFileInode_testIfMetaFileInodeCanBeReturned)
{
    const FileDescriptorType EXPECT_FD = 10;
    std::string fileName = "testFile";
    EXPECT_CALL(*volMgr, HandleNewRequest).WillOnce([](MetaFsRequestBase& reqMsg) -> POS_EVENT_ID {
        auto data = &(static_cast<MetaFsFileControlRequest&>(reqMsg).completionData);
        data->inodeInfoPointer = new MetaFileInodeInfo;
        data->inodeInfoPointer->data.field.fd = EXPECT_FD;
        return EID(SUCCESS);
    });

    auto result = api->Wbt_GetMetaFileInode(fileName, VOLUME_TYPE);
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->data.field.fd, EXPECT_FD);
}

TEST_F(MetaFsFileControlApiFixture, GetAlignedFileIOSize_testIfFileIoSizeCanBeRetrieved)
{
    const size_t EXPECT_SIZE = 1;
    const FileDescriptorType FD = 10;
    std::string fileName = "testFile";
    EXPECT_CALL(*volMgr, HandleNewRequest).WillOnce([](MetaFsRequestBase& reqMsg) -> POS_EVENT_ID {
        auto data = &(static_cast<MetaFsFileControlRequest&>(reqMsg).completionData);
        data->dataChunkSize = EXPECT_SIZE;
        return EID(SUCCESS);
    });

    size_t size = api->GetAlignedFileIOSize(FD, VOLUME_TYPE);
    EXPECT_EQ(size, EXPECT_SIZE);
}

TEST_F(MetaFsFileControlApiFixture, GetTheLastValidLpn_TheLastValidLpn)
{
    const size_t EXPECT_LAST_LPN = 100;
    EXPECT_CALL(*volMgr, GetTheLastValidLpn).WillOnce(Return(EXPECT_LAST_LPN));

    EXPECT_EQ(api->GetTheLastValidLpn(VOLUME_TYPE), EXPECT_LAST_LPN);
}

TEST_F(MetaFsFileControlApiFixture, Create_testIfMetaFileCanBeCreated)
{
    EXPECT_CALL(*volMgr, HandleNewRequest).WillOnce(Return(EID(SUCCESS)));

    std::string fileName = "testFile";
    const uint64_t FILE_SIZE = 1024;
    const MetaFilePropertySet PROPERTY_SET;
    const MetaVolumeType VOLUME_TYPE = MetaVolumeType::SsdVolume;

    EXPECT_EQ(api->Create(fileName, FILE_SIZE, PROPERTY_SET, VOLUME_TYPE), EID(SUCCESS));
}

TEST_F(MetaFsFileControlApiFixture, Delete_testIfMetaFileCanBeDeleted)
{
    EXPECT_CALL(*volMgr, HandleNewRequest).WillOnce(Return(EID(SUCCESS)));

    std::string fileName = "testFile";
    const MetaVolumeType VOLUME_TYPE = MetaVolumeType::SsdVolume;

    EXPECT_EQ(api->Delete(fileName, VOLUME_TYPE), EID(SUCCESS));
}

TEST_F(MetaFsFileControlApiFixture, Open_testIfMetaFileCanBeCreated)
{
    EXPECT_CALL(*volMgr, HandleNewRequest).WillOnce(Return(EID(SUCCESS)));

    std::string fileName = "testFile";
    int fd = 0;

    EXPECT_EQ(api->Open(fileName, fd, VOLUME_TYPE), EID(SUCCESS));
    EXPECT_EQ(fd, 0);
}
} // namespace pos
