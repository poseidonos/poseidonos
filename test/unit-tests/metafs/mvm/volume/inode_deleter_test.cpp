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

#include "src/metafs/mvm/volume/inode_deleter.h"
#include "src/metafs/mvm/volume/inode_manager.h"
#include "test/unit-tests/metafs/mvm/volume/file_descriptor_allocator_mock.h"
#include "test/unit-tests/metafs/mvm/volume/extent_allocator_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_table_header_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_table_mock.h"

#include <vector>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Matcher;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
class InodeDeleterFixture : public ::testing::Test
{
public:
    InodeDeleterFixture(void)
    {
        inodeDeleter = nullptr;
        inodeManager = nullptr;
        inodeHdr = nullptr;
        inodeTable = nullptr;
        fdAllocator = nullptr;
        extentAllocator = nullptr;
    }

    virtual ~InodeDeleterFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        inodeHdr = new NiceMock<MockInodeTableHeader>(type, 0);
        inodeTable = new NiceMock<MockInodeTable>(type, 10);
        fdAllocator = new NiceMock<MockFileDescriptorAllocator>;
        extentAllocator = new NiceMock<MockExtentAllocator>;

        inodeManager = new InodeManager(arrayId, inodeHdr, inodeTable,
            fdAllocator, extentAllocator);

        inodeDeleter = new InodeDeleter(inodeManager);
    }

    virtual void
    TearDown(void)
    {
        delete inodeDeleter;
        delete inodeManager;
    }

protected:
    InodeDeleter* inodeDeleter;

    InodeManager* inodeManager;
    NiceMock<MockInodeTableHeader>* inodeHdr;
    NiceMock<MockInodeTable>* inodeTable;
    NiceMock<MockFileDescriptorAllocator>* fdAllocator;
    NiceMock<MockExtentAllocator>* extentAllocator;

    int arrayId = 0;
    MetaVolumeType type = MetaVolumeType::SsdVolume;
};

TEST_F(InodeDeleterFixture, CheckFileDeletion_Positive)
{
    std::string fileName = "TESTFILE";
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;

    std::vector<MetaFileExtent> extents;

    MetaFsFileControlRequest reqMsg;
    reqMsg.fileName = &fileName;
    reqMsg.fileByteSize = 4097;

    // GetExtent()
    MetaFileInode inode;
    inode.data.basic.field.fd = 0;
    inode.data.basic.field.fileByteSize = 100;
    inode.data.basic.field.dataChunkSize = 4032;
    inode.data.basic.field.pagemapCnt = 1;
    inode.data.basic.field.pagemap[0].SetStartLpn(10);
    inode.data.basic.field.pagemap[0].SetCount(20);
    std::unordered_map<FileDescriptorType, MetaFileInode*>& fd2InodeMap =
        inodeManager->GetInodeMap();
    fd2InodeMap.insert({ 0, &inode });

    // LookupDescriptorByName()
    EXPECT_CALL(*fdAllocator, FindFdByName).WillOnce(Return(0));

    EXPECT_EQ(inodeManager->GetExtent(0, extents), 1);

    EXPECT_CALL(*inodeHdr, ClearInodeInUse);

    EXPECT_CALL(*extentAllocator, AddToFreeList);
    EXPECT_CALL(*fdAllocator, Free(fileName, 0));
    EXPECT_CALL(*extentAllocator, PrintFreeExtentsList);
    EXPECT_CALL(*extentAllocator, GetAllocatedExtentList);

    // SaveContent()
    EXPECT_CALL(*inodeHdr, Store()).WillOnce(Return(true));
    EXPECT_CALL(*inodeHdr, GetInodeInUseBitmap).WillOnce(ReturnRef(bitmap));
    EXPECT_CALL(*inodeTable, GetInode).WillRepeatedly(ReturnRef(inode));
    EXPECT_CALL(*inodeTable, GetBaseLpn).WillOnce(Return(0));

    std::pair<FileDescriptorType, POS_EVENT_ID> result;
    result = inodeDeleter->Delete(reqMsg);

    EXPECT_EQ(fd2InodeMap.size(), 0);
    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, POS_EVENT_ID::SUCCESS);
}

TEST_F(InodeDeleterFixture, CheckFileDeletion_Negative)
{
    std::string fileName = "TESTFILE";

    MetaFsFileControlRequest reqMsg;
    reqMsg.fileName = &fileName;
    reqMsg.fileByteSize = 4097;

    // LookupDescriptorByName()
    EXPECT_CALL(*fdAllocator, FindFdByName).WillOnce(Return(0));

    // GetExtent()
    MetaFileInode inode;
    inode.data.basic.field.fd = 0;
    inode.data.basic.field.fileByteSize = 100;
    inode.data.basic.field.dataChunkSize = 4032;
    inode.data.basic.field.pagemapCnt = 1;
    inode.data.basic.field.pagemap[0].SetStartLpn(10);
    inode.data.basic.field.pagemap[0].SetCount(20);
    std::unordered_map<FileDescriptorType, MetaFileInode*>& fd2InodeMap =
        inodeManager->GetInodeMap();
    fd2InodeMap.insert({ 0, &inode });

    EXPECT_CALL(*inodeHdr, ClearInodeInUse);

    // SaveContent()
    EXPECT_CALL(*inodeHdr, Store()).WillOnce(Return(false));

    std::pair<FileDescriptorType, POS_EVENT_ID> result;
    result = inodeDeleter->Delete(reqMsg);

    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, POS_EVENT_ID::MFS_META_SAVE_FAILED);
}
} // namespace pos
