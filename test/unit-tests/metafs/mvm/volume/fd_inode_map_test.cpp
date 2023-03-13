/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
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

#include "src/metafs/mvm/volume/fd_inode_map.h"

#include <gtest/gtest.h>

#include <string>

using namespace std;

namespace pos
{
class FdInodeMapFixture : public ::testing::Test
{
public:
    FdInodeMapFixture(void)
    : inodeMap(nullptr),
      FILE_NAME("testfile")
    {
    }

    virtual ~FdInodeMapFixture(void)
    {
    }

    virtual void SetUp(void)
    {
        inodeMap = new FdInodeMap();
    }

    virtual void TearDown(void)
    {
        delete inodeMap;
    }

    std::unordered_map<FileDescriptorType, MetaFileInode*> GetInodeMap(void)
    {
        return inodeMap->GetInodeMap();
    }

    MetaFileInode* AllocateInode(void)
    {
        MetaFileInode* inode = new MetaFileInode;
        inode->data.basic.field.fd = FD;
        inode->data.basic.field.fileName = FILE_NAME;
        inode->data.basic.field.fileByteSize = BYTE_SIZE;
        inode->data.basic.field.dataChunkSize = CHUNK_SIZE;
        inode->data.basic.field.pagemapCnt = 1;
        inode->data.basic.field.pagemap[0].SetStartLpn(10);
        inode->data.basic.field.pagemap[0].SetCount(20);
        return inode;
    }

protected:
    const FileDescriptorType FD = 0;
    const FileSizeType BYTE_SIZE = 100;
    const FileSizeType CHUNK_SIZE = 4032;
    const std::string FILE_NAME;

    FdInodeMap* inodeMap;
};

TEST_F(FdInodeMapFixture, Add_testIfEntryIsCreatedCorrectly)
{
    // check init state
    auto fd2InodeMap = GetInodeMap();
    EXPECT_EQ(fd2InodeMap.size(), 0);

    // add entry
    auto inode = AllocateInode();
    inodeMap->Add(FD, inode);

    // map's changed
    fd2InodeMap = GetInodeMap();
    EXPECT_EQ(fd2InodeMap.size(), 1);

    // check values in the inode
    auto result = fd2InodeMap[FD];
    EXPECT_EQ(result->data.basic.field.fd, FD);
    EXPECT_EQ(result->data.basic.field.fileByteSize, BYTE_SIZE);
    EXPECT_EQ(result->data.basic.field.dataChunkSize, CHUNK_SIZE);

    delete inode;
}

TEST_F(FdInodeMapFixture, Remove_testIfEntryIsRemovedCorrectly)
{
    // make init state
    auto inode = AllocateInode();
    inodeMap->Add(FD, inode);

    // check init state
    auto fd2InodeMap = GetInodeMap();
    EXPECT_EQ(fd2InodeMap.size(), 1);

    // remove entry
    auto size = inodeMap->Remove(FD);

    // entry will be removed
    EXPECT_EQ(size, 1);
    fd2InodeMap = GetInodeMap();
    EXPECT_EQ(fd2InodeMap.size(), 0);

    delete inode;
}

TEST_F(FdInodeMapFixture, Remove_testIfEntryIsNotRemoved)
{
    // make init state
    auto inode = AllocateInode();
    inodeMap->Add(FD, inode);

    // check init state
    auto fd2InodeMap = GetInodeMap();
    EXPECT_EQ(fd2InodeMap.size(), 1);

    // remove entry
    auto size = inodeMap->Remove(FD + 1);

    // entry won't be removed
    EXPECT_EQ(size, 0);
    fd2InodeMap = GetInodeMap();
    EXPECT_EQ(fd2InodeMap.size(), 1);

    delete inode;
}

TEST_F(FdInodeMapFixture, GetInode_testIfInodeIsStoredWell)
{
    // make init state
    auto fd2InodeMap = GetInodeMap();
    EXPECT_EQ(fd2InodeMap.size(), 0);

    auto inode = AllocateInode();
    inodeMap->Add(FD, inode);

    // get inode with fd
    auto result = inodeMap->GetInode(FD);

    // check values in the inode
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->data.basic.field.fd, FD);
    EXPECT_EQ(result->data.basic.field.fileByteSize, BYTE_SIZE);
    EXPECT_EQ(result->data.basic.field.dataChunkSize, CHUNK_SIZE);

    delete inode;
}

TEST_F(FdInodeMapFixture, GetInode_testIfReturnsNullptrWithInvalidFileDescriptor)
{
    // make init state
    auto fd2InodeMap = GetInodeMap();
    EXPECT_EQ(fd2InodeMap.size(), 0);

    auto inode = AllocateInode();
    inodeMap->Add(FD, inode);

    // get inode with fd
    // assertion is in the method currently
    EXPECT_DEATH(inodeMap->GetInode(FD + 1), "");

    delete inode;
}

TEST_F(FdInodeMapFixture, GetFileName_testIfReturnsExpectedName)
{
    // make init state
    auto fd2InodeMap = GetInodeMap();
    EXPECT_EQ(fd2InodeMap.size(), 0);

    auto inode = AllocateInode();
    inodeMap->Add(FD, inode);

    // get filename with fd
    auto fileName = inodeMap->GetFileName(FD);
    EXPECT_EQ(fileName, FILE_NAME);

    delete inode;
}

TEST_F(FdInodeMapFixture, GetFileName_testIfReturnsNothing)
{
    // make init state
    auto fd2InodeMap = GetInodeMap();
    EXPECT_EQ(fd2InodeMap.size(), 0);

    auto inode = AllocateInode();
    inodeMap->Add(FD, inode);

    // get filename with fd
    auto fileName = inodeMap->GetFileName(FD + 1);
    EXPECT_EQ(fileName, "");

    delete inode;
}

TEST_F(FdInodeMapFixture, Reset_testIfResetAllEntry)
{
    // add entry
    auto inode = AllocateInode();
    inodeMap->Add(FD, inode);

    // check the count
    auto fd2InodeMap = GetInodeMap();
    EXPECT_EQ(fd2InodeMap.size(), 1);

    // reset
    inodeMap->Reset();

    // check the count again
    fd2InodeMap = GetInodeMap();
    EXPECT_EQ(fd2InodeMap.size(), 0);

    delete inode;
}
} // namespace pos
