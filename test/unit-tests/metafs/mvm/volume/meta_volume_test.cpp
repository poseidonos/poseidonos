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

#include "src/metafs/mvm/volume/meta_volume.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <map>
#include <string>

#include "src/metafs/mvm/volume/meta_volume_state.h"
#include "test/unit-tests/metafs/mvm/volume/catalog_manager_mock.h"
#include "test/unit-tests/metafs/mvm/volume/catalog_mock.h"
#include "test/unit-tests/metafs/mvm/volume/extent_allocator_mock.h"
#include "test/unit-tests/metafs/mvm/volume/file_descriptor_allocator_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_creator_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_deleter_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_manager_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"

using ::testing::_;
using ::testing::Matcher;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace std;

namespace pos
{
class MetaVolumeTester : public MetaVolume
{
public:
    MetaVolumeTester(const int arrayId, const MetaVolumeType volumeType,
        const MetaLpnType maxVolumePageNum = 0, InodeManager* inodeMgr = nullptr,
        CatalogManager* catalogMgr = nullptr, InodeCreator* inodeCreator = nullptr,
        InodeDeleter* inodeDeleter = nullptr)
    : MetaVolume(arrayId, volumeType, maxVolumePageNum, inodeMgr, catalogMgr, inodeCreator, inodeDeleter)
    {
    }
    virtual ~MetaVolumeTester(void)
    {
    }
    void InitVolumeBaseLpn(void)
    {
        volumeBaseLpn_ = 2000;
    }
    bool IsOkayToStore(FileSizeType fileByteSize, MetaFilePropertySet& prop)
    {
        return true;
    }
    void SetVolumeType(MetaVolumeType volumeType)
    {
        volumeType_ = volumeType;
    }
    void SetVolumeState(MetaVolumeState state)
    {
        volumeState_ = state;
    }
    MetaLpnType
    GetBaseLpn(void)
    {
        return MetaVolume::GetBaseLpn();
    }

    MetaLpnType
    GetMaxLpn(void)
    {
        return MetaVolume::GetMaxLpn();
    }
};

class MetaVolumeFixture : public ::testing::Test
{
public:
    MetaVolumeFixture(void)
    {
    }
    virtual ~MetaVolumeFixture(void)
    {
    }
    virtual void
    SetUp(void)
    {
        inodeMgr = new NiceMock<MockInodeManager>(arrayId);
        catalogMgr = new NiceMock<MockCatalogManager>(arrayId);
        metaStorage = new NiceMock<MockMetaStorageSubsystem>(arrayId);

        inodeCreator = new NiceMock<MockInodeCreator>(inodeMgr);
        inodeDeleter = new NiceMock<MockInodeDeleter>(inodeMgr);

        metaVolume = new MetaVolumeTester(arrayId, volumeType, maxVolumePageNum,
            inodeMgr, catalogMgr, inodeCreator, inodeDeleter);

        EXPECT_CALL(*inodeMgr, Init);
        EXPECT_CALL(*inodeMgr, SetMss);
        EXPECT_CALL(*catalogMgr, Init);
        EXPECT_CALL(*catalogMgr, SetMss);

        EXPECT_CALL(*inodeMgr, GetMetaFileBaseLpn()).WillRepeatedly(Return(DEFAULT_SIZE));
        EXPECT_CALL(*inodeMgr, GetRegionSizeInLpn()).WillRepeatedly(Return(DEFAULT_SIZE));
        EXPECT_CALL(*catalogMgr, GetRegionSizeInLpn).WillRepeatedly(Return(DEFAULT_SIZE));

        metaVolume->Init(metaStorage);
    }
    virtual void
    TearDown(void)
    {
        delete metaVolume;
        delete metaStorage;
    }

protected:
    const int DEFAULT_SIZE = 5;

    MetaVolumeTester* metaVolume = nullptr;

    NiceMock<MockInodeManager>* inodeMgr = nullptr;
    NiceMock<MockCatalogManager>* catalogMgr = nullptr;
    NiceMock<MockMetaStorageSubsystem>* metaStorage = nullptr;
    NiceMock<MockInodeCreator>* inodeCreator = nullptr;
    NiceMock<MockInodeDeleter>* inodeDeleter = nullptr;

    int arrayId = 0;
    MetaVolumeType volumeType = MetaVolumeType::SsdVolume;
    MetaLpnType maxVolumePageNum = 1024;
};

TEST_F(MetaVolumeFixture, CheckStoreability)
{
    FileSizeType size = 0;
    MetaFilePropertySet set;

    EXPECT_EQ(metaVolume->IsOkayToStore(size, set), true);
}

TEST_F(MetaVolumeFixture, CreateVolumePositive)
{
    EXPECT_CALL(*catalogMgr, CreateCatalog).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, SaveContent).WillRepeatedly(Return(true));

    EXPECT_TRUE(metaVolume->CreateVolume());
}

TEST_F(MetaVolumeFixture, CreateVolumeNegative_DueTo_CreateCatalog)
{
    EXPECT_CALL(*catalogMgr, CreateCatalog).WillRepeatedly(Return(false));

    EXPECT_FALSE(metaVolume->CreateVolume());
}

TEST_F(MetaVolumeFixture, CreateVolumeNegative_DueTo_SaveContent_CatalogMgr)
{
    EXPECT_CALL(*catalogMgr, CreateCatalog).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, SaveContent).WillRepeatedly(Return(false));

    EXPECT_FALSE(metaVolume->CreateVolume());
}

TEST_F(MetaVolumeFixture, CreateVolumeNegative_DueTo_SaveContent_InodeMgr)
{
    EXPECT_CALL(*catalogMgr, CreateCatalog).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, SaveContent).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent).WillRepeatedly(Return(false));

    EXPECT_FALSE(metaVolume->CreateVolume());
}

TEST_F(MetaVolumeFixture, OpenVolumePositive)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, RestoreContent(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RestoreContent(_, _, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;

    EXPECT_TRUE(metaVolume->OpenVolume(info, false));
}

TEST_F(MetaVolumeFixture, OpenVolumeNegative_DueTo_LoadVolCatalog)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(false));
    EXPECT_CALL(*catalogMgr, RestoreContent(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RestoreContent(_, _, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;

    EXPECT_FALSE(metaVolume->OpenVolume(info, false));
}

TEST_F(MetaVolumeFixture, OpenVolumeNegative_DueTo_LoadContent)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, RestoreContent(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadContent()).WillRepeatedly(Return(false));
    EXPECT_CALL(*inodeMgr, RestoreContent(_, _, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;

    EXPECT_FALSE(metaVolume->OpenVolume(info, false));
}

TEST_F(MetaVolumeFixture, OpenVolumePositive_Even_If_RestoreContent_CatalogMgr)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, RestoreContent(_, _, _)).WillRepeatedly(Return(false));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RestoreContent(_, _, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;

    EXPECT_TRUE(metaVolume->OpenVolume(info, false));
}

TEST_F(MetaVolumeFixture, OpenVolumePositive_Even_If_RestoreContent_InodeMgr)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, RestoreContent(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RestoreContent(_, _, _, _)).WillRepeatedly(Return(false));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;

    EXPECT_TRUE(metaVolume->OpenVolume(info, false));
}

TEST_F(MetaVolumeFixture, OpenVolumeNegative_DueTo_RestoreContent_CatalogMgr)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, RestoreContent(_, _, _)).WillRepeatedly(Return(false));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RestoreContent(_, _, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;
    metaVolume->SetVolumeType(MetaVolumeType::NvRamVolume);

    EXPECT_FALSE(metaVolume->OpenVolume(info, true));
}

TEST_F(MetaVolumeFixture, OpenVolumeNegative_DueTo_RestoreContent_InodeMgr)
{
    EXPECT_CALL(*catalogMgr, LoadVolCatalog()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, RestoreContent(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, Bringup()).WillRepeatedly(Return());
    EXPECT_CALL(*inodeMgr, LoadContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RestoreContent(_, _, _, _)).WillRepeatedly(Return(false));
    EXPECT_CALL(*inodeMgr, Bringup()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;
    metaVolume->SetVolumeType(MetaVolumeType::NvRamVolume);

    EXPECT_FALSE(metaVolume->OpenVolume(info, true));
}

TEST_F(MetaVolumeFixture, CloseVolumePositive)
{
    EXPECT_CALL(*inodeMgr, GetFileCountInActive()).WillRepeatedly(Return(0));
    EXPECT_CALL(*catalogMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, BackupContent(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, BackupContent(_, _, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Finalize()).WillRepeatedly(Return());
    EXPECT_CALL(*catalogMgr, Finalize()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;
    bool resetContext = false;
    metaVolume->SetVolumeState(MetaVolumeState::Open);

    EXPECT_TRUE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CloseVolumePositive_VolumeIsNotClosed)
{
    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;
    bool resetContext = false;

    EXPECT_TRUE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CloseVolumeNegative_GetFileCountInActive)
{
    EXPECT_CALL(*inodeMgr, GetFileCountInActive()).WillRepeatedly(Return(1));
    EXPECT_CALL(*catalogMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, BackupContent(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, BackupContent(_, _, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Finalize()).WillRepeatedly(Return());
    EXPECT_CALL(*catalogMgr, Finalize()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;
    bool resetContext = false;
    metaVolume->SetVolumeState(MetaVolumeState::Open);

    EXPECT_FALSE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CloseVolumeNegative_SaveContent_CatalogMgr)
{
    EXPECT_CALL(*inodeMgr, GetFileCountInActive()).WillRepeatedly(Return(0));
    EXPECT_CALL(*catalogMgr, SaveContent()).WillRepeatedly(Return(false));
    EXPECT_CALL(*inodeMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, BackupContent(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, BackupContent(_, _, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Finalize()).WillRepeatedly(Return());
    EXPECT_CALL(*catalogMgr, Finalize()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;
    bool resetContext = false;
    metaVolume->SetVolumeState(MetaVolumeState::Open);

    EXPECT_FALSE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CloseVolumeNegative_SaveContent_InodeMgr)
{
    EXPECT_CALL(*inodeMgr, GetFileCountInActive()).WillRepeatedly(Return(0));
    EXPECT_CALL(*catalogMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent()).WillRepeatedly(Return(false));
    EXPECT_CALL(*catalogMgr, BackupContent(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, BackupContent(_, _, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Finalize()).WillRepeatedly(Return());
    EXPECT_CALL(*catalogMgr, Finalize()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;
    bool resetContext = false;
    metaVolume->SetVolumeState(MetaVolumeState::Open);

    EXPECT_FALSE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CloseVolumeNegative_BackupContent_CatalogMgr)
{
    EXPECT_CALL(*inodeMgr, GetFileCountInActive()).WillRepeatedly(Return(0));
    EXPECT_CALL(*catalogMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, BackupContent(_, _, _)).WillRepeatedly(Return(false));
    EXPECT_CALL(*inodeMgr, BackupContent(_, _, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, Finalize()).WillRepeatedly(Return());
    EXPECT_CALL(*catalogMgr, Finalize()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;
    bool resetContext = false;
    metaVolume->SetVolumeState(MetaVolumeState::Open);
    metaVolume->SetVolumeType(MetaVolumeType::NvRamVolume);

    EXPECT_FALSE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CloseVolumeNegative_BackupContent_InodeMgr)
{
    EXPECT_CALL(*inodeMgr, GetFileCountInActive()).WillRepeatedly(Return(0));
    EXPECT_CALL(*catalogMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, SaveContent()).WillRepeatedly(Return(true));
    EXPECT_CALL(*catalogMgr, BackupContent(_, _, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, BackupContent(_, _, _, _)).WillRepeatedly(Return(false));
    EXPECT_CALL(*inodeMgr, Finalize()).WillRepeatedly(Return());
    EXPECT_CALL(*catalogMgr, Finalize()).WillRepeatedly(Return());

    MetaLpnType* info = new MetaLpnType[2]();
    info[0] = 0;
    info[1] = 0;
    bool resetContext = false;
    metaVolume->SetVolumeState(MetaVolumeState::Open);
    metaVolume->SetVolumeType(MetaVolumeType::NvRamVolume);

    EXPECT_FALSE(metaVolume->CloseVolume(info, resetContext));
}

TEST_F(MetaVolumeFixture, CheckActiveFds)
{
    EXPECT_CALL(*inodeMgr, AddFileInActiveList).WillRepeatedly(Return(EID(SUCCESS)));
    EXPECT_CALL(*inodeMgr, CheckFileInActive).WillRepeatedly(Return(true));
    EXPECT_CALL(*inodeMgr, RemoveFileFromActiveList).WillRepeatedly(Return());

    for (FileDescriptorType fd = 0; fd < 5; ++fd)
    {
        EXPECT_EQ(metaVolume->AddFileInActiveList(fd), EID(SUCCESS));
    }

    for (FileDescriptorType fd = 0; fd < 5; ++fd)
    {
        EXPECT_TRUE(metaVolume->CheckFileInActive(fd));
    }

    for (FileDescriptorType fd = 0; fd < 5; ++fd)
    {
        metaVolume->RemoveFileFromActiveList(fd);
    }
}

TEST_F(MetaVolumeFixture, CheckBaseLpn)
{
    EXPECT_EQ(metaVolume->GetBaseLpn(), 5);
}

TEST_F(MetaVolumeFixture, CheckMaxLpn)
{
    EXPECT_EQ(metaVolume->GetMaxLpn(), 1024);
}

TEST_F(MetaVolumeFixture, CheckRegionSizeInLpn)
{
    EXPECT_CALL(*catalogMgr, GetRegionSizeInLpn).WillOnce(Return(20));
    EXPECT_CALL(*inodeMgr, GetRegionSizeInLpn(_))
        .WillOnce(Return(20))
        .WillOnce(Return(20));

    EXPECT_EQ(metaVolume->GetRegionSizeInLpn(MetaRegionType::VolCatalog), 20);
    EXPECT_EQ(metaVolume->GetRegionSizeInLpn(MetaRegionType::FileInodeHdr), 20);
    EXPECT_EQ(metaVolume->GetRegionSizeInLpn(MetaRegionType::FileInodeTable), 20);
}

TEST_F(MetaVolumeFixture, CheckInodeList_Negative)
{
    std::vector<MetaFileInfoDumpCxt>* fileInfoList = nullptr;

    EXPECT_CALL(*inodeMgr, IsFileInodeInUse).WillRepeatedly(Return(false));

    metaVolume->GetInodeList(fileInfoList);
}

TEST_F(MetaVolumeFixture, CheckFileCreation_Positive)
{
    std::string fileName = "TESTFILE";
    MetaFsFileControlRequest reqMsg;
    reqMsg.fileName = &fileName;
    FileControlResult result = {0, EID(SUCCESS)};

    EXPECT_CALL(*inodeCreator, Create).WillOnce(Return(result));

    result = metaVolume->CreateFile(reqMsg);

    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, EID(SUCCESS));
}

TEST_F(MetaVolumeFixture, CheckFileCreation_Negative)
{
    std::string fileName = "TESTFILE";
    MetaFsFileControlRequest reqMsg;
    reqMsg.fileName = &fileName;
    FileControlResult result = {0, EID(MFS_META_SAVE_FAILED)};

    EXPECT_CALL(*inodeCreator, Create).WillOnce(Return(result));

    result = metaVolume->CreateFile(reqMsg);

    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, EID(MFS_META_SAVE_FAILED));
}

TEST_F(MetaVolumeFixture, CheckFileDeletion_Positive)
{
    std::string fileName = "TESTFILE";
    MetaFsFileControlRequest reqMsg;
    reqMsg.fileName = &fileName;
    FileControlResult result = {0, EID(SUCCESS)};

    EXPECT_CALL(*inodeDeleter, Delete).WillOnce(Return(result));

    result = metaVolume->DeleteFile(reqMsg);

    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, EID(SUCCESS));
}

TEST_F(MetaVolumeFixture, CheckFileDeletion_Negative)
{
    std::string fileName = "TESTFILE";
    MetaFsFileControlRequest reqMsg;
    reqMsg.fileName = &fileName;
    FileControlResult result = {0, EID(MFS_META_SAVE_FAILED)};

    EXPECT_CALL(*inodeDeleter, Delete).WillOnce(Return(result));

    result = metaVolume->DeleteFile(reqMsg);

    EXPECT_EQ(result.first, 0);
    EXPECT_EQ(result.second, EID(MFS_META_SAVE_FAILED));
}

TEST_F(MetaVolumeFixture, CheckCreatedFile)
{
    StringHashType hash = 1234;

    EXPECT_CALL(*inodeMgr, IsGivenFileCreated).WillOnce(Return(true));

    EXPECT_EQ(metaVolume->IsGivenFileCreated(hash), true);
}

TEST_F(MetaVolumeFixture, CheckFileSize)
{
    EXPECT_CALL(*inodeMgr, GetFileSize).WillOnce(Return(1234));

    EXPECT_EQ(metaVolume->GetFileSize(0), 1234);
}

TEST_F(MetaVolumeFixture, CheckChunkSize)
{
    EXPECT_CALL(*inodeMgr, GetDataChunkSize).WillOnce(Return(1234));

    EXPECT_EQ(metaVolume->GetDataChunkSize(0), 1234);
}

TEST_F(MetaVolumeFixture, CheckFileBaseLpn)
{
    EXPECT_CALL(*inodeMgr, GetFileBaseLpn).WillOnce(Return(1234));

    EXPECT_EQ(metaVolume->GetFileBaseLpn(0), 1234);
}

TEST_F(MetaVolumeFixture, CheckTrim_Positive)
{
    std::string fileName = "TESTFILE";

    MetaFsFileControlRequest reqMsg;
    reqMsg.fileName = &fileName;

    MetaFileInode inode;
    inode.data.basic.field.ioAttribute.media = MetaStorageType::SSD;
    inode.data.basic.field.pagemapCnt = 1;
    inode.data.basic.field.pagemap[0].SetStartLpn(0);
    inode.data.basic.field.pagemap[0].SetCount(10);

    // Trim
    EXPECT_CALL(*inodeMgr, LookupDescriptorByName).WillOnce(Return(0));
    EXPECT_CALL(*inodeMgr, GetFileInode).WillRepeatedly(ReturnRef(inode));

    // _Trim
    EXPECT_CALL(*metaStorage, TrimFileData)
        .WillRepeatedly(Return(EID(SUCCESS)));
    EXPECT_CALL(*metaStorage, WritePage)
        .WillRepeatedly(Return(EID(SUCCESS)));

    EXPECT_EQ(metaVolume->TrimData(reqMsg), true);
}

TEST_F(MetaVolumeFixture, LookupDescriptorByName)
{
    std::string fileName = "TESTFILE";

    EXPECT_CALL(*inodeMgr, LookupDescriptorByName).WillOnce(Return(1));

    EXPECT_EQ(metaVolume->LookupDescriptorByName(fileName), 1);
}

TEST_F(MetaVolumeFixture, LookupNameByDescriptor)
{
    std::string fileName = "TESTFILE";

    EXPECT_CALL(*inodeMgr, LookupNameByDescriptor).WillOnce(Return(fileName));

    EXPECT_EQ(metaVolume->LookupNameByDescriptor(0), fileName);
}

TEST_F(MetaVolumeFixture, CheckInode)
{
    MetaFileInode inode;
    inode.data.basic.field.ioAttribute.media = MetaStorageType::SSD;
    inode.data.basic.field.pagemapCnt = 1;
    inode.data.basic.field.pagemap[0].SetStartLpn(0);
    inode.data.basic.field.pagemap[0].SetCount(10);

    EXPECT_CALL(*inodeMgr, GetFileInode).WillRepeatedly(ReturnRef(inode));

    EXPECT_EQ(metaVolume->GetInode(0).data.basic.field.ioAttribute.media,
        inode.data.basic.field.ioAttribute.media);
    EXPECT_EQ(metaVolume->GetInode(0).data.basic.field.pagemapCnt,
        inode.data.basic.field.pagemapCnt);
    EXPECT_EQ(metaVolume->GetInode(0).data.basic.field.pagemap[0].GetStartLpn(),
        inode.data.basic.field.pagemap[0].GetStartLpn());
    EXPECT_EQ(metaVolume->GetInode(0).data.basic.field.pagemap[0].GetCount(),
        inode.data.basic.field.pagemap[0].GetCount());
}

TEST_F(MetaVolumeFixture, CopyInode_testIfTheInodeInfoIsNullptr)
{
    std::string fileName = "TESTFILE";
    MetaFileInodeInfo* inodeInfo = nullptr;

    bool result = metaVolume->CopyInodeToInodeInfo(0, inodeInfo);
    EXPECT_EQ(result, false);
}

TEST_F(MetaVolumeFixture, CopyInode_testIfCopyMethodWillBeSuccess)
{
    std::string fileName = "TESTFILE";
    MetaFileInodeInfo inodeInfo;
    MetaFileInode inode;
    inode.data.basic.field.fileName = fileName;
    inode.data.basic.field.fd = 0;

    EXPECT_CALL(*inodeMgr, GetFileInode).WillOnce(ReturnRef(inode));

    bool result = metaVolume->CopyInodeToInodeInfo(0, &inodeInfo);

    EXPECT_EQ(result, true);
    EXPECT_EQ((fileName == inodeInfo.data.field.fileName) ? true : false, true);
    EXPECT_EQ(inodeInfo.data.field.fd, 0);
}

TEST_F(MetaVolumeFixture, Get_TheLastValidLpn)
{
    EXPECT_CALL(*inodeMgr, GetTheLastValidLpn).WillOnce(Return(100));

    MetaLpnType result = metaVolume->GetTheLastValidLpn();

    EXPECT_EQ(result, 100);
}

TEST(MetaVolume, Create)
{
    MetaVolumeTester vol(0, MetaVolumeType::SsdVolume);
}
} // namespace pos
