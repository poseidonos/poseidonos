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
class InodeManagerFixture : public ::testing::Test
{
public:
    InodeManagerFixture(void)
    {
        inodeHdr = nullptr;
        inodeTable = nullptr;
        fdAllocator = nullptr;
        extentAllocator = nullptr;
        inodes = nullptr;
    }

    virtual ~InodeManagerFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        inodeHdr = new NiceMock<MockInodeTableHeader>(type, 0);
        inodeTable = new NiceMock<MockInodeTable>(type, 10);
        fdAllocator = new NiceMock<MockFileDescriptorAllocator>;
        extentAllocator = new NiceMock<MockExtentAllocator>;

        inodes = new MetaFileInodeArray;

        EXPECT_CALL(*inodeHdr, GetLpnCntOfRegion).WillOnce(Return(10));
        EXPECT_CALL(*inodeTable, GetLpnCntOfRegion).WillOnce(Return(20));

        EXPECT_CALL(*extentAllocator, Init);

        inodeMgr = new InodeManager(arrayId, inodeHdr, inodeTable, fdAllocator,
            extentAllocator);

        inodeMgr->Init(type, baseLpn, maxLpn);
    }

    virtual void
    TearDown(void)
    {
        delete inodeMgr;
    }

protected:
    InodeManager* inodeMgr;

    NiceMock<MockInodeTableHeader>* inodeHdr;
    NiceMock<MockInodeTable>* inodeTable;
    NiceMock<MockFileDescriptorAllocator>* fdAllocator;
    NiceMock<MockExtentAllocator>* extentAllocator;

    MetaFileInodeArray* inodes;

    int arrayId = 0;
    MetaVolumeType type = MetaVolumeType::SsdVolume;
    MetaLpnType baseLpn = 0;
    MetaLpnType maxLpn = 200;
};

TEST_F(InodeManagerFixture, CheckRegionSizeInLpn)
{
    EXPECT_CALL(*inodeHdr, GetLpnCntOfRegion).WillOnce(Return(10));
    EXPECT_CALL(*inodeTable, GetLpnCntOfRegion).WillOnce(Return(20));

    EXPECT_EQ(inodeMgr->GetRegionSizeInLpn(), 30);
}

TEST_F(InodeManagerFixture, CheckInodeManagerBringup)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;
    MetaFileInode inode;

    EXPECT_CALL(*inodeHdr, BuildFreeInodeEntryMap);
    EXPECT_CALL(*inodeHdr, GetInodeInUseBitmap).WillRepeatedly(ReturnRef(bitmap));
    EXPECT_CALL(*inodeTable, GetInode).WillRepeatedly(ReturnRef(inode));
    EXPECT_CALL(*inodeTable, GetInodeArray).WillOnce(ReturnRef(*inodes));

    inodeMgr->Bringup();
}

TEST_F(InodeManagerFixture, CheckSaveContent_Positive)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;
    MetaFileInode inode;

    EXPECT_CALL(*inodeHdr, Store()).WillOnce(Return(true));
    EXPECT_CALL(*inodeHdr, GetInodeInUseBitmap).WillOnce(ReturnRef(bitmap));
    EXPECT_CALL(*inodeTable, GetInode).WillRepeatedly(ReturnRef(inode));
    EXPECT_CALL(*inodeTable, GetBaseLpn).WillOnce(Return(0));

    EXPECT_EQ(inodeMgr->SaveContent(), true);
}

TEST_F(InodeManagerFixture, CheckSaveContent_Negative0)
{
    EXPECT_CALL(*inodeHdr, Store()).WillOnce(Return(false));

    EXPECT_EQ(inodeMgr->SaveContent(), false);
}

TEST_F(InodeManagerFixture, CheckSaveContent_Negative1)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;
    MetaFileInode inode;

    bitmap.set(0, true);

    EXPECT_CALL(*inodeHdr, Store()).WillOnce(Return(true));
    EXPECT_CALL(*inodeHdr, GetInodeInUseBitmap).WillOnce(ReturnRef(bitmap));
    EXPECT_CALL(*inodeTable, GetInode).WillRepeatedly(ReturnRef(inode));
    EXPECT_CALL(*inodeTable, GetBaseLpn).WillOnce(Return(0));
    EXPECT_CALL(*inodeTable, Store(_, _, _, _)).WillOnce(Return(false));

    EXPECT_EQ(inodeMgr->SaveContent(), false);
}

TEST_F(InodeManagerFixture, CheckFinalize)
{
    EXPECT_CALL(*fdAllocator, Reset);

    inodeMgr->Finalize();
}

TEST_F(InodeManagerFixture, CheckMss)
{
    EXPECT_CALL(*inodeHdr, SetMss);
    EXPECT_CALL(*inodeTable, SetMss);

    inodeMgr->SetMss(nullptr);
}

TEST_F(InodeManagerFixture, CheckInodeInformation)
{
    FileDescriptorType fd = 0;
    MetaFileInode inode;
    inode.data.basic.field.fd = 0;
    inode.data.basic.field.fileByteSize = 100;
    inode.data.basic.field.dataChunkSize = 4032;
    inode.data.basic.field.pagemapCnt = 2;
    inode.data.basic.field.pagemap[0].SetStartLpn(10);
    inode.data.basic.field.pagemap[0].SetCount(20);
    inode.data.basic.field.pagemap[1].SetStartLpn(110);
    inode.data.basic.field.pagemap[1].SetCount(210);

    std::unordered_map<FileDescriptorType, MetaFileInode*>& fd2InodeMap =
        inodeMgr->GetInodeMap();

    fd2InodeMap.insert({ 0, &inode });

    EXPECT_EQ(inodeMgr->GetFileSize(fd), 100);
    EXPECT_EQ(inodeMgr->GetDataChunkSize(fd), 4032);
    EXPECT_EQ(inodeMgr->GetFileBaseLpn(fd), 10);

    std::vector<MetaFileExtent> extents;
    uint32_t count = inodeMgr->GetExtent(0, extents);

    EXPECT_EQ(count, 2);
    if (count == 2)
    {
        EXPECT_EQ(extents[0].GetStartLpn(), 10);
        EXPECT_EQ(extents[0].GetCount(), 20);
        EXPECT_EQ(extents[1].GetStartLpn(), 110);
        EXPECT_EQ(extents[1].GetCount(), 210);
    }
}

TEST_F(InodeManagerFixture, CreateInodeContents)
{
    std::vector<pos::MetaFileExtent> extentList;

    EXPECT_CALL(*inodeHdr, Create);
    EXPECT_CALL(*inodeTable, Create);
    EXPECT_CALL(*extentAllocator, GetAllocatedExtentList)
        .WillOnce(Return(extentList));
    EXPECT_CALL(*inodeHdr, SetFileExtentContent);

    inodeMgr->CreateInitialInodeContent(10);
}

TEST_F(InodeManagerFixture, LoadContents_Positive)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;

    EXPECT_CALL(*inodeHdr, Load()).WillOnce(Return(true));
    EXPECT_CALL(*inodeTable, GetBaseLpn()).WillOnce(Return(0));
    EXPECT_CALL(*inodeHdr, GetInodeInUseBitmap).WillRepeatedly(ReturnRef(bitmap));

    EXPECT_EQ(inodeMgr->LoadContent(), true);
}

TEST_F(InodeManagerFixture, LoadContents_Negative0)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;

    EXPECT_CALL(*inodeHdr, Load()).WillOnce(Return(false));

    EXPECT_EQ(inodeMgr->LoadContent(), false);
}

TEST_F(InodeManagerFixture, LoadContents_Negative1)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;
    bitmap.set(0, true);

    EXPECT_CALL(*inodeHdr, Load()).WillOnce(Return(true));
    EXPECT_CALL(*inodeTable, GetBaseLpn()).WillOnce(Return(0));
    EXPECT_CALL(*inodeHdr, GetInodeInUseBitmap).WillRepeatedly(ReturnRef(bitmap));
    EXPECT_CALL(*inodeTable, Load(_, _, _, _)).WillOnce(Return(false));

    EXPECT_EQ(inodeMgr->LoadContent(), false);
}

TEST_F(InodeManagerFixture, CheckRegionBaseLpn0)
{
    EXPECT_CALL(*inodeHdr, GetBaseLpn()).WillOnce(Return(10));

    EXPECT_EQ(inodeMgr->GetRegionBaseLpn(MetaRegionType::FileInodeHdr), 10);
}

TEST_F(InodeManagerFixture, CheckRegionBaseLpn1)
{
    EXPECT_CALL(*inodeTable, GetBaseLpn()).WillOnce(Return(10));

    EXPECT_EQ(inodeMgr->GetRegionBaseLpn(MetaRegionType::FileInodeTable), 10);
}

TEST_F(InodeManagerFixture, CheckRegionSize0)
{
    EXPECT_CALL(*inodeHdr, GetLpnCntOfRegion()).WillOnce(Return(10));

    EXPECT_EQ(inodeMgr->GetRegionSizeInLpn(MetaRegionType::FileInodeHdr), 10);
}

TEST_F(InodeManagerFixture, CheckRegionSize1)
{
    EXPECT_CALL(*inodeTable, GetLpnCntOfRegion()).WillOnce(Return(10));

    EXPECT_EQ(inodeMgr->GetRegionSizeInLpn(MetaRegionType::FileInodeTable), 10);
}

TEST_F(InodeManagerFixture, CheckFdMap)
{
    std::unordered_map<FileDescriptorType, MetaVolumeType> dest;
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;
    bitmap.set(0, true);

    EXPECT_CALL(*inodeHdr, GetInodeInUseBitmap).WillRepeatedly(ReturnRef(bitmap));
    EXPECT_CALL(*inodeTable, GetFileDescriptor).WillOnce(Return(0));

    inodeMgr->PopulateFDMapWithVolumeType(dest);

    EXPECT_EQ(dest.size(), 1);
    if (dest.size() == 1)
    {
        EXPECT_EQ(dest[0], MetaVolumeType::SsdVolume);
    }
}

TEST_F(InodeManagerFixture, CheckHashMap)
{
    std::unordered_map<StringHashType, MetaVolumeType> dest;

    inodes->at(0).SetInUse(true);

    EXPECT_CALL(*inodeTable, GetInodeArray).WillOnce(ReturnRef(*inodes));

    inodeMgr->PopulateFileNameWithVolumeType(dest);

    EXPECT_EQ(dest.size(), 1);
    if (dest.size() == 1)
    {
        EXPECT_EQ(dest[0], MetaVolumeType::SsdVolume);
    }
}

TEST_F(InodeManagerFixture, CheckUtilization)
{
    EXPECT_CALL(*extentAllocator, GetAvailableLpnCount).WillOnce(Return(100));

    EXPECT_EQ(inodeMgr->GetAvailableLpnCount(), 100);
}

TEST_F(InodeManagerFixture, CheckAvailableSpaceInByteSize0)
{
    EXPECT_CALL(*extentAllocator, GetAvailableLpnCount).WillOnce(Return(7));

    EXPECT_EQ(inodeMgr->GetAvailableSpace(), 0);
}

TEST_F(InodeManagerFixture, CheckAvailableSpaceInByteSize1)
{
    EXPECT_CALL(*extentAllocator, GetAvailableLpnCount).WillOnce(Return(8));

    EXPECT_EQ(inodeMgr->GetAvailableSpace(), 8 * 4032);
}

TEST_F(InodeManagerFixture, CheckAvailableSpaceInByteSize2)
{
    EXPECT_CALL(*extentAllocator, GetAvailableLpnCount).WillOnce(Return(9));

    EXPECT_EQ(inodeMgr->GetAvailableSpace(), 8 * 4032);
}

TEST_F(InodeManagerFixture, CheckFileInActive)
{
    std::unordered_set<FileDescriptorType>& set = inodeMgr->GetActiveFiles();
    set.insert(0);

    EXPECT_EQ(inodeMgr->CheckFileInActive(0), true);
    EXPECT_EQ(inodeMgr->CheckFileInActive(1), false);
}

TEST_F(InodeManagerFixture, CheckAddedFileInActive)
{
    EXPECT_EQ(inodeMgr->AddFileInActiveList(0),
                    EID(SUCCESS));
    EXPECT_EQ(inodeMgr->AddFileInActiveList(0),
                    EID(MFS_FILE_OPEN_REPETITIONARY));
}

TEST_F(InodeManagerFixture, RemoveFileInActive)
{
    std::unordered_set<FileDescriptorType>& set = inodeMgr->GetActiveFiles();
    set.insert(0);

    EXPECT_EQ(set.size(), 1);

    inodeMgr->RemoveFileFromActiveList(0);
}

TEST_F(InodeManagerFixture, GetFileCountInActive)
{
    std::unordered_set<FileDescriptorType>& set = inodeMgr->GetActiveFiles();
    set.insert(0);

    EXPECT_EQ(inodeMgr->GetFileCountInActive(), 1);

    set.erase(0);

    EXPECT_EQ(inodeMgr->GetFileCountInActive(), 0);
}

TEST_F(InodeManagerFixture, SetMetaFileBase)
{
    EXPECT_CALL(*extentAllocator, SetFileBaseLpn);

    inodeMgr->SetMetaFileBaseLpn(10);
}

TEST_F(InodeManagerFixture, GetMetaFileBase)
{
    EXPECT_CALL(*extentAllocator, GetFileBaseLpn)
            .WillOnce(Return(123));

    EXPECT_EQ(inodeMgr->GetMetaFileBaseLpn(), 123);
}

TEST_F(InodeManagerFixture, CheckFileCreated)
{
    EXPECT_CALL(*fdAllocator, IsGivenFileCreated(Matcher<StringHashType>(_)))
            .WillOnce(Return(true));

    EXPECT_EQ(inodeMgr->IsGivenFileCreated(123), true);
}

TEST_F(InodeManagerFixture, CheckFileCreated0)
{
    std::string fileName = "TESTFILE";

    EXPECT_CALL(*fdAllocator, FindFdByName)
            .WillOnce(Return(10));

    EXPECT_EQ(inodeMgr->LookupDescriptorByName(fileName), 10);
}

TEST_F(InodeManagerFixture, CheckFileCreated1)
{
    std::string fileName = "TESTFILE";
    MetaFileInode inode;
    inode.data.basic.field.fd = 0;
    inode.data.basic.field.fileName = fileName;
    inode.data.basic.field.fileByteSize = 100;
    inode.data.basic.field.dataChunkSize = 4032;
    inode.data.basic.field.pagemapCnt = 1;
    inode.data.basic.field.pagemap[0].SetStartLpn(10);
    inode.data.basic.field.pagemap[0].SetCount(20);

    std::unordered_map<FileDescriptorType, MetaFileInode*>& fd2InodeMap =
        inodeMgr->GetInodeMap();
    fd2InodeMap.insert({ 0, &inode });

    EXPECT_EQ(inodeMgr->LookupNameByDescriptor(1), "");
    EXPECT_EQ(inodeMgr->LookupNameByDescriptor(0), fileName);
}

TEST_F(InodeManagerFixture, CheckInode0)
{
    std::string fileName = "TESTFILE";
    MetaFileInode inode;
    inode.data.basic.field.fd = 0;
    inode.data.basic.field.fileName = fileName;
    inode.data.basic.field.fileByteSize = 100;
    inode.data.basic.field.dataChunkSize = 4032;
    inode.data.basic.field.pagemapCnt = 1;
    inode.data.basic.field.pagemap[0].SetStartLpn(10);
    inode.data.basic.field.pagemap[0].SetCount(20);

    std::unordered_map<FileDescriptorType, MetaFileInode*>& fd2InodeMap =
        inodeMgr->GetInodeMap();
    fd2InodeMap.insert({ 0, &inode });

    MetaFileInode& obj = inodeMgr->GetFileInode(0);

    EXPECT_EQ(&obj, &inode);
}

TEST_F(InodeManagerFixture, CheckInode1)
{
    std::string fileName = "TESTFILE";
    MetaFileInode inode;
    inode.data.basic.field.fd = 0;
    inode.data.basic.field.fileName = fileName;
    inode.data.basic.field.fileByteSize = 100;
    inode.data.basic.field.dataChunkSize = 4032;
    inode.data.basic.field.pagemapCnt = 1;
    inode.data.basic.field.pagemap[0].SetStartLpn(10);
    inode.data.basic.field.pagemap[0].SetCount(20);

    EXPECT_CALL(*inodeTable, GetInode)
            .WillOnce(ReturnRef(inode));

    MetaFileInode& obj = inodeMgr->GetInodeEntry(0);

    EXPECT_EQ(&obj, &inode);
}

TEST_F(InodeManagerFixture, CheckInUseFlag)
{
    EXPECT_CALL(*inodeHdr, IsFileInodeInUse)
            .WillOnce(Return(true));

    EXPECT_EQ(inodeMgr->IsFileInodeInUse(0), true);
}

TEST_F(InodeManagerFixture, BackupContent_Positive)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;
    MetaFileInode inode;
    MetaVolumeType type = MetaVolumeType::NvRamVolume;

    EXPECT_CALL(*inodeHdr, Store(_, _, _, _))
            .WillOnce(Return(true));

    // _StoreInodeToMedia()
    EXPECT_CALL(*inodeHdr, GetInodeInUseBitmap)
            .WillOnce(ReturnRef(bitmap));

    EXPECT_EQ(inodeMgr->BackupContent(type, 0, 0, 0), true);
}

TEST_F(InodeManagerFixture, BackupContent_Negative0)
{
    MetaVolumeType type = MetaVolumeType::NvRamVolume;

    EXPECT_CALL(*inodeHdr, Store(_, _, _, _))
            .WillOnce(Return(false));

    EXPECT_EQ(inodeMgr->BackupContent(type, 0, 0, 0), false);
}

TEST_F(InodeManagerFixture, BackupContent_Negative1)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;
    bitmap.set(0, true);
    MetaVolumeType type = MetaVolumeType::NvRamVolume;

    EXPECT_CALL(*inodeHdr, Store(_, _, _, _))
            .WillOnce(Return(true));

    // _StoreInodeToMedia()
    EXPECT_CALL(*inodeHdr, GetInodeInUseBitmap)
            .WillOnce(ReturnRef(bitmap));
    EXPECT_CALL(*inodeTable, Store(_, _, _, _))
            .WillOnce(Return(false));

    EXPECT_EQ(inodeMgr->BackupContent(type, 0, 0, 0), false);
}

TEST_F(InodeManagerFixture, RestoreContent_Positive)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;
    MetaVolumeType type = MetaVolumeType::NvRamVolume;

    EXPECT_CALL(*inodeHdr, Load(_, _, _, _))
            .WillOnce(Return(true));

    // _LoadInodeFromMedia()
    EXPECT_CALL(*inodeHdr, GetInodeInUseBitmap)
            .WillOnce(ReturnRef(bitmap));

    EXPECT_EQ(inodeMgr->RestoreContent(type, 0, 0, 0), true);
}

TEST_F(InodeManagerFixture, RestoreContent_Negative0)
{
    MetaVolumeType type = MetaVolumeType::NvRamVolume;

    EXPECT_CALL(*inodeHdr, Load(_, _, _, _))
            .WillOnce(Return(false));

    EXPECT_EQ(inodeMgr->RestoreContent(type, 0, 0, 0), false);
}

TEST_F(InodeManagerFixture, RestoreContent_Negative1)
{
    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bitmap;
    bitmap.set(0, true);
    MetaVolumeType type = MetaVolumeType::NvRamVolume;

    EXPECT_CALL(*inodeHdr, Load(_, _, _, _))
            .WillOnce(Return(true));

    // _LoadInodeFromMedia()
    EXPECT_CALL(*inodeHdr, GetInodeInUseBitmap)
            .WillOnce(ReturnRef(bitmap));
    EXPECT_CALL(*inodeTable, Load(_, _, _, _))
            .WillOnce(Return(false));

    EXPECT_EQ(inodeMgr->RestoreContent(type, 0, 0, 0), false);
}

TEST_F(InodeManagerFixture, RestoreContent_CheckTheLastLpn)
{
    MetaVolumeType type = MetaVolumeType::SsdVolume;
    std::vector<pos::MetaFileExtent> extentList;

    EXPECT_CALL(*inodeHdr, GetFileExtentContent)
            .WillOnce(Return(extentList));

    EXPECT_EQ(inodeMgr->GetTheLastValidLpn(), 0);

    extentList.push_back({0, 100});     // 0 to 99

    EXPECT_CALL(*inodeHdr, GetFileExtentContent)
            .WillOnce(Return(extentList));

    EXPECT_EQ(inodeMgr->GetTheLastValidLpn(), 99);

    extentList.push_back({200, 100});   // 200 to 299

    EXPECT_CALL(*inodeHdr, GetFileExtentContent)
            .WillOnce(Return(extentList));

    EXPECT_EQ(inodeMgr->GetTheLastValidLpn(), 299);
}
} // namespace pos
