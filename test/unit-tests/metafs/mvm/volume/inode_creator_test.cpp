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

#include "src/metafs/mvm/volume/inode_creator.h"
#include "test/unit-tests/metafs/mvm/volume/inode_manager_mock.h"
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
class InodeCreatorFixture : public ::testing::Test
{
public:
    InodeCreatorFixture(void)
    {
        inodeCreator = nullptr;
        inodeManager = nullptr;
        inodeHdr = nullptr;
        inodeTable = nullptr;
        fdAllocator = nullptr;
        extentAllocator = nullptr;
    }

    virtual ~InodeCreatorFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        inodeManager = new NiceMock<MockInodeManager>(arrayId);

        inodeHdr = new NiceMock<MockInodeTableHeader>(type, 0);
        inodeTable = new NiceMock<MockInodeTable>(type, 10);
        fdAllocator = new NiceMock<MockFileDescriptorAllocator>;
        extentAllocator = new NiceMock<MockExtentAllocator>;

        inodeManager = new NiceMock<MockInodeManager>(arrayId, inodeHdr, inodeTable,
            fdAllocator, extentAllocator);

        inodeCreator = new InodeCreator(inodeManager);
    }

    virtual void
    TearDown(void)
    {
        delete inodeCreator;
        delete inodeManager;
    }

protected:
    InodeCreator* inodeCreator;

    NiceMock<MockInodeManager>* inodeManager;
    NiceMock<MockInodeTableHeader>* inodeHdr;
    NiceMock<MockInodeTable>* inodeTable;
    NiceMock<MockFileDescriptorAllocator>* fdAllocator;
    NiceMock<MockExtentAllocator>* extentAllocator;

    int arrayId = 0;
    MetaVolumeType type = MetaVolumeType::SsdVolume;
};

TEST_F(InodeCreatorFixture, CheckFileCreation_Positive)
{
    std::string fileName = "TESTFILE";
    MetaFileInode inode;
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;

    std::vector<MetaFileExtent> extents;
    extents.push_back({0, 16});

    MetaFsFileControlRequest reqMsg;
    reqMsg.fileName = &fileName;
    reqMsg.fileByteSize = 4097;

    EXPECT_CALL(*fdAllocator, Alloc(fileName)).WillOnce(Return(0));

    // _AllocNewInodeEntry()
    EXPECT_CALL(*inodeHdr, GetFreeInodeEntryIdx);
    EXPECT_CALL(*inodeHdr, SetInodeInUse);
    EXPECT_CALL(*inodeTable, GetInode).WillOnce(ReturnRef(inode));

    EXPECT_CALL(*extentAllocator, AllocExtents).WillOnce(Return(extents));
    EXPECT_CALL(*extentAllocator, GetAllocatedExtentList).WillOnce(Return(extents));

    // SaveContent()
    EXPECT_CALL(*inodeManager, SaveContent()).WillOnce(Return(true));

    std::pair<FileDescriptorType, POS_EVENT_ID> result;
    result = inodeCreator->Create(reqMsg);

    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, EID(SUCCESS));
}

TEST_F(InodeCreatorFixture, CheckFileCreation_Negative)
{
    std::string fileName = "TESTFILE";
    MetaFileInode inode;

    std::vector<MetaFileExtent> extents;
    extents.push_back({0, 16});

    MetaFsFileControlRequest reqMsg;
    reqMsg.fileName = &fileName;
    reqMsg.fileByteSize = 4097;

    EXPECT_CALL(*fdAllocator, Alloc(fileName)).WillOnce(Return(0));

    // _AllocNewInodeEntry()
    EXPECT_CALL(*inodeHdr, GetFreeInodeEntryIdx);
    EXPECT_CALL(*inodeHdr, SetInodeInUse);
    EXPECT_CALL(*inodeTable, GetInode).WillOnce(ReturnRef(inode));

    EXPECT_CALL(*extentAllocator, AllocExtents).WillOnce(Return(extents));
    EXPECT_CALL(*extentAllocator, GetAllocatedExtentList).WillOnce(Return(extents));

    // SaveContent()
    EXPECT_CALL(*inodeManager, SaveContent()).WillOnce(Return(false));

    EXPECT_CALL(*extentAllocator, AddToFreeList);

    std::pair<FileDescriptorType, POS_EVENT_ID> result;
    result = inodeCreator->Create(reqMsg);

    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, EID(MFS_META_SAVE_FAILED));
}
} // namespace pos
