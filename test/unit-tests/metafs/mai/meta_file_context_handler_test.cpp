/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "src/metafs/mai/meta_file_context_handler.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "src/metafs/config/metafs_config.h"
#include "test/unit-tests/lib/bitmap_mock.h"
#include "test/unit-tests/metafs/mvm/meta_volume_manager_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class MetaFileContextHandlerFixture : public ::testing::Test
{
public:
    MetaFileContextHandlerFixture(void)
    {
    }

    virtual ~MetaFileContextHandlerFixture(void)
    {
    }

    virtual void SetUp(void)
    {
        fileName = "testFile";

        volMgr = new NiceMock<MockMetaVolumeManager>(ARRAY_ID, nullptr);
        handler = new MetaFileContextHandler(ARRAY_ID, nullptr, volMgr);
        handler->Initialize(SIGNATURE);
    }

    virtual void TearDown(void)
    {
        delete volMgr;
        delete handler;
    }

    MetaFileInodeInfo* GetInode(void)
    {
        MetaFileInodeInfo* inode = new MetaFileInodeInfo;
        memcpy(inode->data.field.fileName, fileName.c_str(), fileName.length());
        inode->data.field.extentCnt = 1;
        inode->data.field.fileByteSize = EXPECT_FILE_BYTE_SIZE;
        return inode;
    }

    void SetExpectedCallToVolumeManager(MetaFileInodeInfo* inode)
    {
        EXPECT_CALL(*volMgr, HandleNewRequest).WillOnce([=](MetaFsRequestBase& reqMsg) -> POS_EVENT_ID {
            static_cast<MetaFsFileControlRequest&>(reqMsg).completionData.inodeInfoPointer = inode;
            return EID(SUCCESS);
        });
    }

protected:
    MetaFileContextHandler* handler;
    NiceMock<MockMetaVolumeManager>* volMgr;

    std::string fileName;
    const int ARRAY_ID = 0;
    const uint64_t SIGNATURE = 0x12345678;
    const FileDescriptorType FD = 0;
    const MetaVolumeType VOLUME_TYPE = MetaVolumeType::SsdVolume;

    const MetaFileType EXPECT_FILE_TYPE = MetaFileType::SpecialPurposeMap;
    const FileSizeType EXPECT_FILE_BYTE_SIZE = 4096;
};

TEST_F(MetaFileContextHandlerFixture, AddFileContext_testIfThereIsNoError)
{
    MetaFileInodeInfo* inode = GetInode();

    SetExpectedCallToVolumeManager(inode);

    handler->AddFileContext(fileName, FD, VOLUME_TYPE);

    EXPECT_EQ(handler->GetBitMap()->FindFirstSet(0), FD);
    EXPECT_EQ(handler->GetNameMap()->find(make_pair(VOLUME_TYPE, FD))->second, fileName);
    EXPECT_EQ(handler->GetIndexMap()->find(make_pair(VOLUME_TYPE, fileName))->second, 0);
}

TEST_F(MetaFileContextHandlerFixture, GetFileContext_testIfThereIsNoContextToMatch)
{
    EXPECT_EQ(handler->GetFileContext(FD, VOLUME_TYPE), nullptr);
}

TEST_F(MetaFileContextHandlerFixture, GetFileContext_testIfExpectedContextWillBeReturned)
{
    MetaFileInodeInfo* inode = GetInode();

    SetExpectedCallToVolumeManager(inode);

    handler->AddFileContext(fileName, FD, VOLUME_TYPE);

    MetaFileContext* ctx = handler->GetFileContext(FD, VOLUME_TYPE);
    ASSERT_NE(ctx, nullptr);
    EXPECT_EQ(ctx->fileType, MetaFileType::SpecialPurposeMap);
    EXPECT_EQ(ctx->sizeInByte, EXPECT_FILE_BYTE_SIZE);
    EXPECT_EQ(ctx->signature, SIGNATURE);
}

TEST_F(MetaFileContextHandlerFixture, GetFileContext_testIfThereIsContextButNotMatchedToTheVolumeType)
{
    MetaFileInodeInfo* inode = GetInode();

    SetExpectedCallToVolumeManager(inode);

    handler->AddFileContext(fileName, FD, VOLUME_TYPE);

    EXPECT_NE(handler->GetFileContext(FD, VOLUME_TYPE), nullptr);
    EXPECT_EQ(handler->GetFileContext(FD, MetaVolumeType::JournalVolume), nullptr);
}

TEST_F(MetaFileContextHandlerFixture, GetFileContext_testIfThereIsContextButNotMatchedToTheFileDescriptor)
{
    MetaFileInodeInfo* inode = GetInode();

    SetExpectedCallToVolumeManager(inode);

    handler->AddFileContext(fileName, FD, VOLUME_TYPE);

    EXPECT_NE(handler->GetFileContext(FD, VOLUME_TYPE), nullptr);
    EXPECT_EQ(handler->GetFileContext(1, VOLUME_TYPE), nullptr);
}

TEST_F(MetaFileContextHandlerFixture, TryRemoveFileContext_testIfThereIsNoError)
{
    MetaFileInodeInfo* inode = GetInode();

    SetExpectedCallToVolumeManager(inode);

    handler->AddFileContext(fileName, FD, VOLUME_TYPE);

    handler->RemoveFileContext(FD, VOLUME_TYPE);

    uint64_t expectedValue = MetaFsConfig::MAX_VOLUME_CNT;
    EXPECT_EQ(handler->GetBitMap()->FindFirstSet(0), expectedValue);
    std::unique_ptr<MetaVolTypeAndFdToFileName> nameMap = handler->GetNameMap();
    EXPECT_EQ(nameMap->find(make_pair(VOLUME_TYPE, FD)), nameMap->end());
    std::unique_ptr<MetaVolTypeAndFileNameToIndex> indexMap = handler->GetIndexMap();
    EXPECT_EQ(indexMap->find(make_pair(VOLUME_TYPE, fileName)), indexMap->end());
}
} // namespace pos
