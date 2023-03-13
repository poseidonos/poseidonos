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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "src/metafs/mvm/volume/inode_manager.h"
#include "test/unit-tests/metafs/mvm/volume/extent_allocator_mock.h"
#include "test/unit-tests/metafs/mvm/volume/fd_inode_map_mock.h"
#include "test/unit-tests/metafs/mvm/volume/file_descriptor_allocator_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_manager_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_table_header_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_table_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
class InodeDeleterFixture : public ::testing::Test
{
public:
    InodeDeleterFixture(void)
    : inodeDeleter(nullptr),
      inodeManager(nullptr),
      inodeHdr(nullptr),
      inodeTable(nullptr),
      fdAllocator(nullptr),
      extentAllocator(nullptr),
      inodeMap(nullptr),
      inode(nullptr)
    {
    }

    virtual ~InodeDeleterFixture(void)
    {
    }

    virtual void SetUp(void)
    {
        inodeHdr = new NiceMock<MockInodeTableHeader>(volumeType, 0);
        inodeTable = new NiceMock<MockInodeTable>(volumeType, 10);
        fdAllocator = new NiceMock<MockFileDescriptorAllocator>;
        extentAllocator = new NiceMock<MockExtentAllocator>;
        inodeMap = new NiceMock<MockFdInodeMap>;

        inodeManager = new NiceMock<MockInodeManager>(ARRAY_ID, inodeHdr, inodeTable,
            fdAllocator, extentAllocator, nullptr, inodeMap);

        inodeDeleter = new InodeDeleter(inodeManager);

        inode = AllocateInode();
    }

    virtual void TearDown(void)
    {
        delete inodeDeleter;
        delete inodeManager;
        delete inode;
    }

    MetaFileInode* AllocateInode(void)
    {
        MetaFileInode* inode = new MetaFileInode;
        inode->data.basic.field.fd = FD;
        inode->data.basic.field.fileName = fileName;
        inode->data.basic.field.fileByteSize = BYTE_SIZE;
        inode->data.basic.field.dataChunkSize = CHUNK_SIZE;
        inode->data.basic.field.pagemapCnt = EXTENT_COUNT;
        inode->data.basic.field.pagemap[0].SetStartLpn(START_LPN);
        inode->data.basic.field.pagemap[0].SetCount(LPN_COUNT);
        return inode;
    }

protected:
    InodeDeleter* inodeDeleter;

    NiceMock<MockInodeManager>* inodeManager;
    NiceMock<MockInodeTableHeader>* inodeHdr;
    NiceMock<MockInodeTable>* inodeTable;
    NiceMock<MockFileDescriptorAllocator>* fdAllocator;
    NiceMock<MockExtentAllocator>* extentAllocator;
    NiceMock<MockFdInodeMap>* inodeMap;

    const int ARRAY_ID = 0;
    const FileDescriptorType FD = 1;
    const uint16_t EXTENT_COUNT = 1;
    const FileSizeType BYTE_SIZE = 100;
    const FileSizeType CHUNK_SIZE = 4032;
    const MetaLpnType START_LPN = 10;
    const MetaLpnType LPN_COUNT = 20;
    const MetaVolumeType volumeType = MetaVolumeType::SsdVolume;
    std::string fileName = "TESTFILE";

    MetaFileInode* inode;
};

TEST_F(InodeDeleterFixture, CheckFileDeletion_testIfSuccessfullyExecuted)
{
    EXPECT_CALL(*inodeManager, LookupDescriptorByName).WillOnce(Return(FD));
    EXPECT_CALL(*inodeManager, GetExtent).WillOnce(Return(EXTENT_COUNT));
    EXPECT_CALL(*inodeManager, GetFileInode).WillOnce(ReturnRef(*inode));
    EXPECT_CALL(*inodeManager, SaveContent).WillOnce(Return(true));
    EXPECT_CALL(*inodeHdr, ClearInodeInUse);
    EXPECT_CALL(*inodeMap, Remove).WillOnce(Return(1));
    EXPECT_CALL(*fdAllocator, Free(fileName, FD));
    EXPECT_CALL(*extentAllocator, PrintFreeExtentsList);
    EXPECT_CALL(*extentAllocator, GetAllocatedExtentList);

    MetaFsFileControlRequest reqMsg;
    reqMsg.fileName = &fileName;
    reqMsg.fileByteSize = 4097;

    auto result = inodeDeleter->Delete(reqMsg);

    EXPECT_EQ(result.first, FD);
    EXPECT_EQ(result.second, EID(SUCCESS));
}
} // namespace pos
