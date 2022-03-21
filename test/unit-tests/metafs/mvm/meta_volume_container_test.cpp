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

#include "src/metafs/mvm/meta_volume_container.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "test/unit-tests/metafs/mvm/volume/meta_volume_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
class MetaVolumeContainerTexture : public ::testing::Test
{
public:
    MetaVolumeContainerTexture(void)
    {
    }

    virtual ~MetaVolumeContainerTexture(void)
    {
    }

    virtual void SetUp(void) override
    {
        ssdVolume = std::make_shared<NiceMock<MockMetaVolume>>(arrayId, MetaVolumeType::SsdVolume, 0);
        nvramVolume = std::make_shared<NiceMock<MockMetaVolume>>(arrayId, MetaVolumeType::NvRamVolume, 0);
        journalVolume = std::make_shared<NiceMock<MockMetaVolume>>(arrayId, MetaVolumeType::JournalVolume, 0);

        EXPECT_CALL(*ssdVolume, Init);
        EXPECT_CALL(*nvramVolume, Init);
        EXPECT_CALL(*journalVolume, Init);

        EXPECT_CALL(*ssdVolume, GetVolumeType).WillRepeatedly(Return(MetaVolumeType::SsdVolume));
        EXPECT_CALL(*nvramVolume, GetVolumeType).WillRepeatedly(Return(MetaVolumeType::NvRamVolume));
        EXPECT_CALL(*journalVolume, GetVolumeType).WillRepeatedly(Return(MetaVolumeType::JournalVolume));

        container = new MetaVolumeContainer(arrayId);
        volumeList[0] = ssdVolume;
        volumeList[1] = nvramVolume;
        volumeList[2] = journalVolume;

        for (int i = 0; i < (int)MetaVolumeType::Max; ++i)
        {
            _InitializeVolume((MetaVolumeType)i);
        }
    }

    virtual void TearDown(void) override
    {
        delete container;
    }

protected:
    void _InitializeVolume(MetaVolumeType volumeType)
    {
        int index = (int)volumeType;
        mss[index] = new NiceMock<MockMetaStorageSubsystem>(arrayId);
        container->InitContext(volumeType, arrayId,
            maxVolPageNum[index], mss[index], volumeList[index]);
    }

    MetaVolumeContainer* container;

    std::shared_ptr<NiceMock<MockMetaVolume>> ssdVolume;
    std::shared_ptr<NiceMock<MockMetaVolume>> nvramVolume;
    std::shared_ptr<NiceMock<MockMetaVolume>> journalVolume;

    NiceMock<MockMetaStorageSubsystem>* mss[(int)MetaVolumeType::Max];
    std::shared_ptr<MetaVolume> volumeList[(int)MetaVolumeType::Max];

    int arrayId = 0;
    MetaLpnType maxVolPageNum[(int)MetaVolumeType::Max] = {100, 1000, 150};
};

TEST_F(MetaVolumeContainerTexture, CreateVolumes)
{
    for (int i = 0; i < (int)MetaVolumeType::Max; ++i)
    {
        EXPECT_CALL(*(NiceMock<MockMetaVolume>*)volumeList[i].get(), CreateVolume).WillOnce(Return(true));
        EXPECT_EQ(container->CreateVolume((MetaVolumeType)i), true);
    }
}

TEST_F(MetaVolumeContainerTexture, OpenAllVolumes)
{
    EXPECT_CALL(*ssdVolume, CreateVolume).WillOnce(Return(true));
    EXPECT_CALL(*ssdVolume, OpenVolume).WillOnce(Return(true));
    EXPECT_CALL(*ssdVolume, GetRegionSizeInLpn).WillRepeatedly(Return(0));
    EXPECT_CALL(*nvramVolume, CreateVolume).WillOnce(Return(true));
    EXPECT_CALL(*nvramVolume, OpenVolume).WillOnce(Return(true));
    EXPECT_CALL(*journalVolume, CreateVolume).WillOnce(Return(true));
    EXPECT_CALL(*journalVolume, OpenVolume).WillOnce(Return(true));

    for (int i = 0; i < (int)MetaVolumeType::Max; ++i)
    {
        container->CreateVolume((MetaVolumeType)i);
    }

    EXPECT_EQ(container->OpenAllVolumes(true), true);
}

TEST_F(MetaVolumeContainerTexture, CloseAllVolumes)
{
    EXPECT_CALL(*ssdVolume, CreateVolume).WillOnce(Return(true));
    EXPECT_CALL(*ssdVolume, OpenVolume).WillOnce(Return(true));
    EXPECT_CALL(*ssdVolume, CloseVolume).WillOnce([this](MetaLpnType* info, bool& reset)
    {
        reset = true;
        return true;
    });
    EXPECT_CALL(*ssdVolume, GetBaseLpn).WillRepeatedly(Return(0));
    EXPECT_CALL(*ssdVolume, GetRegionSizeInLpn).WillRepeatedly(Return(0));
    EXPECT_CALL(*nvramVolume, CreateVolume).WillOnce(Return(true));
    EXPECT_CALL(*nvramVolume, OpenVolume).WillOnce(Return(true));
    EXPECT_CALL(*nvramVolume, CloseVolume).WillOnce([this](MetaLpnType* info, bool& reset)
    {
        reset = true;
        return true;
    });
    EXPECT_CALL(*journalVolume, CreateVolume).WillOnce(Return(true));
    EXPECT_CALL(*journalVolume, OpenVolume).WillOnce(Return(true));
    EXPECT_CALL(*journalVolume, CloseVolume).WillOnce([this](MetaLpnType* info, bool& reset)
    {
        reset = true;
        return true;
    });

    for (int i = 0; i < (int)MetaVolumeType::Max; ++i)
    {
        container->CreateVolume((MetaVolumeType)i);
    }

    container->OpenAllVolumes(true);

    bool npor = false;
    EXPECT_EQ(container->CloseAllVolumes(npor), true);
    EXPECT_EQ(npor, true);
}

TEST_F(MetaVolumeContainerTexture, CloseVolumesWhichAreSsdAndNvramVolumes)
{
    EXPECT_CALL(*ssdVolume, CreateVolume).WillOnce(Return(true));
    EXPECT_CALL(*ssdVolume, OpenVolume).WillOnce(Return(true));
    EXPECT_CALL(*ssdVolume, CloseVolume).WillOnce([this](MetaLpnType* info, bool& reset)
    {
        reset = true;
        return true;
    });
    EXPECT_CALL(*ssdVolume, GetBaseLpn).WillRepeatedly(Return(0));
    EXPECT_CALL(*ssdVolume, GetRegionSizeInLpn).WillRepeatedly(Return(0));
    EXPECT_CALL(*nvramVolume, CreateVolume).WillOnce(Return(true));
    EXPECT_CALL(*nvramVolume, OpenVolume).WillOnce(Return(true));
    EXPECT_CALL(*nvramVolume, CloseVolume).WillOnce([this](MetaLpnType* info, bool& reset)
    {
        reset = true;
        return true;
    });
    EXPECT_CALL(*journalVolume, CreateVolume).WillOnce(Return(true));
    EXPECT_CALL(*journalVolume, OpenVolume).WillOnce(Return(true));
    EXPECT_CALL(*journalVolume, CloseVolume).WillOnce([this](MetaLpnType* info, bool& reset)
    {
        reset = true;
        return true;
    });

    for (int i = 0; i < (int)MetaVolumeType::Max; ++i)
    {
        container->CreateVolume((MetaVolumeType)i);
    }

    container->OpenAllVolumes(true);

    bool npor = false;
    EXPECT_EQ(container->CloseAllVolumes(npor), true);
    EXPECT_EQ(npor, true);
}

TEST_F(MetaVolumeContainerTexture, CheckExistVolume)
{
    for (int i = 0; i < (int)MetaVolumeType::Max; ++i)
    {
        EXPECT_EQ(container->IsGivenVolumeExist((MetaVolumeType)i), true);
    }
}

TEST_F(MetaVolumeContainerTexture, Trim)
{
    MetaFsFileControlRequest reqMsg;

    EXPECT_CALL(*ssdVolume, TrimData).WillOnce(Return(true));

    EXPECT_EQ(container->TrimData(MetaVolumeType::SsdVolume, reqMsg), true);
}

TEST_F(MetaVolumeContainerTexture, FileCreation)
{
    MetaFsFileControlRequest reqMsg;

    EXPECT_CALL(*ssdVolume, CreateFile)
        .WillOnce(Return(std::make_pair(0, POS_EVENT_ID::SUCCESS)));

    EXPECT_EQ(container->CreateFile(MetaVolumeType::SsdVolume, reqMsg), true);
}

TEST_F(MetaVolumeContainerTexture, FileDeletion)
{
    MetaFsFileControlRequest reqMsg;

    EXPECT_CALL(*ssdVolume, DeleteFile)
        .WillOnce(Return(std::make_pair(0, POS_EVENT_ID::SUCCESS)));

    EXPECT_EQ(container->DeleteFile(MetaVolumeType::SsdVolume, reqMsg), true);
}

TEST_F(MetaVolumeContainerTexture, CheckExtent)
{
    EXPECT_CALL(*ssdVolume, GetAvailableSpace).WillOnce(Return(10));

    EXPECT_EQ(container->GetAvailableSpace(MetaVolumeType::SsdVolume), 10);
}

TEST_F(MetaVolumeContainerTexture, CheckActiveFile)
{
    EXPECT_CALL(*ssdVolume, CheckFileInActive).WillOnce(Return(true));

    EXPECT_EQ(container->CheckFileInActive(MetaVolumeType::SsdVolume, 10), true);
}

TEST_F(MetaVolumeContainerTexture, AddActiveFile)
{
    EXPECT_CALL(*ssdVolume, AddFileInActiveList).WillOnce(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(container->AddFileInActiveList(MetaVolumeType::SsdVolume, 10), POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaVolumeContainerTexture, RemoveFileFromActiveList)
{
    EXPECT_CALL(*ssdVolume, RemoveFileFromActiveList);

    container->RemoveFileFromActiveList(MetaVolumeType::SsdVolume, 10);
}

TEST_F(MetaVolumeContainerTexture, CheckFileCreated0)
{
    EXPECT_CALL(*ssdVolume, IsGivenFileCreated).WillOnce(Return(true));

    std::string fileName = "TESTFILE";
    EXPECT_EQ(container->IsGivenFileCreated(fileName), true);
}

TEST_F(MetaVolumeContainerTexture, CheckFileCreated1)
{
    EXPECT_CALL(*ssdVolume, IsGivenFileCreated).WillOnce(Return(false));
    EXPECT_CALL(*nvramVolume, IsGivenFileCreated).WillOnce(Return(true));

    std::string fileName = "TESTFILE";
    EXPECT_EQ(container->IsGivenFileCreated(fileName), true);
}

TEST_F(MetaVolumeContainerTexture, CheckFileSize)
{
    EXPECT_CALL(*ssdVolume, GetFileSize).WillOnce(Return(0));

    EXPECT_EQ(container->GetFileSize(MetaVolumeType::SsdVolume, 10), 0);
}

TEST_F(MetaVolumeContainerTexture, CheckChunkSize)
{
    EXPECT_CALL(*ssdVolume, GetDataChunkSize).WillOnce(Return(0));

    EXPECT_EQ(container->GetDataChunkSize(MetaVolumeType::SsdVolume, 10), 0);
}

TEST_F(MetaVolumeContainerTexture, CheckBaseLpn)
{
    EXPECT_CALL(*ssdVolume, GetFileBaseLpn).WillOnce(Return(0));

    EXPECT_EQ(container->GetFileBaseLpn(MetaVolumeType::SsdVolume, 10), 0);
}

TEST_F(MetaVolumeContainerTexture, CheckMaxLpn)
{
    EXPECT_CALL(*ssdVolume, GetMaxLpn).WillOnce(Return(0));

    EXPECT_EQ(container->GetMaxLpn(MetaVolumeType::SsdVolume), 0);
}

TEST_F(MetaVolumeContainerTexture, LookupFile0)
{
    EXPECT_CALL(*ssdVolume, LookupDescriptorByName).WillOnce(Return(1));

    std::string fileName = "TESTFILE";
    EXPECT_EQ(container->LookupFileDescByName(fileName), 1);
}

TEST_F(MetaVolumeContainerTexture, LookupFile1)
{
    FileDescriptorType invalid = MetaFsCommonConst::INVALID_FD;

    EXPECT_CALL(*ssdVolume, LookupDescriptorByName)
        .WillOnce(Return(invalid));
    EXPECT_CALL(*nvramVolume, LookupDescriptorByName)
        .WillOnce(Return(1));

    std::string fileName = "TESTFILE";
    EXPECT_EQ(container->LookupFileDescByName(fileName), 1);
}

TEST_F(MetaVolumeContainerTexture, LookupFile_Negative)
{
    FileDescriptorType invalid = MetaFsCommonConst::INVALID_FD;

    EXPECT_CALL(*ssdVolume, LookupDescriptorByName)
        .WillOnce(Return(invalid));
    EXPECT_CALL(*nvramVolume, LookupDescriptorByName)
        .WillOnce(Return(invalid));
    EXPECT_CALL(*journalVolume, LookupDescriptorByName)
        .WillOnce(Return(invalid));

    std::string fileName = "TESTFILE";
    EXPECT_EQ(container->LookupFileDescByName(fileName), invalid);
}

TEST_F(MetaVolumeContainerTexture, CheckInodeList)
{
    std::vector<MetaFileInfoDumpCxt>* fileInfoList =
        new std::vector<MetaFileInfoDumpCxt>;

    EXPECT_CALL(*ssdVolume, GetInodeList)
        .WillOnce([this](std::vector<MetaFileInfoDumpCxt>*& fileInfoList)
        {
            MetaFileInfoDumpCxt ctx;
            ctx.fileName = "111";
            ctx.fd = 0;
            fileInfoList->push_back(ctx);
        });
    EXPECT_CALL(*nvramVolume, GetInodeList)
        .WillOnce([this](std::vector<MetaFileInfoDumpCxt>*& fileInfoList)
        {
            MetaFileInfoDumpCxt ctx;
            ctx.fileName = "222";
            ctx.fd = 1;
            fileInfoList->push_back(ctx);
        });

    fileInfoList->clear();
    container->GetInodeList(fileInfoList, MetaVolumeType::SsdVolume);

    EXPECT_EQ(fileInfoList->size(), 1);
    EXPECT_EQ(fileInfoList->at(0).fileName, "111");
    EXPECT_EQ(fileInfoList->at(0).fd, 0);

    fileInfoList->clear();
    container->GetInodeList(fileInfoList, MetaVolumeType::NvRamVolume);

    EXPECT_EQ(fileInfoList->size(), 1);
    EXPECT_EQ(fileInfoList->at(0).fileName, "222");
    EXPECT_EQ(fileInfoList->at(0).fd, 1);
}

TEST_F(MetaVolumeContainerTexture, CheckInode)
{
    std::string fileName = "TESTFILE";
    MetaFileInode inode;
    inode.data.basic.field.fileName = fileName;
    inode.data.basic.field.fd = 0;

    EXPECT_CALL(*ssdVolume, GetInode)
        .WillOnce(ReturnRef(inode));

    MetaFileInode& inode0 = container->GetInode(0, MetaVolumeType::SsdVolume);
    EXPECT_TRUE(inode0.data.basic.field.fileName == fileName);
}

TEST_F(MetaVolumeContainerTexture, CopyInode_testIfTheInodeInfoIsNullptr)
{
    std::string fileName = "TESTFILE";
    MetaFileInodeInfo* inodeInfo = nullptr;

    EXPECT_CALL(*ssdVolume, CopyInodeToInodeInfo).WillOnce(Return(false));

    bool result = container->CopyInodeToInodeInfo(0, MetaVolumeType::SsdVolume, inodeInfo);
    EXPECT_EQ(result, false);
}

TEST_F(MetaVolumeContainerTexture, CopyInode_testIfCopyMethodWillBeSuccess)
{
    std::string fileName = "TESTFILE";
    MetaFileInodeInfo inodeInfo;
    MetaFileInode inode;
    inode.data.basic.field.fileName = fileName;
    inode.data.basic.field.fd = 0;

    EXPECT_CALL(*ssdVolume, CopyInodeToInodeInfo).WillOnce(Return(true));

    bool result = container->CopyInodeToInodeInfo(0, MetaVolumeType::SsdVolume, &inodeInfo);

    EXPECT_EQ(result, true);
}

TEST_F(MetaVolumeContainerTexture, CheckLookupFile0_Positive)
{
    EXPECT_CALL(*ssdVolume, LookupNameByDescriptor).WillOnce(Return("1"));

    POS_EVENT_ID rc = container->LookupMetaVolumeType(0, MetaVolumeType::SsdVolume);
    EXPECT_EQ(rc, POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaVolumeContainerTexture, CheckLookupFile0_Negative)
{
    EXPECT_CALL(*ssdVolume, LookupNameByDescriptor).WillOnce(Return(""));

    POS_EVENT_ID rc = container->LookupMetaVolumeType(0, MetaVolumeType::SsdVolume);
    EXPECT_EQ(rc, POS_EVENT_ID::MFS_INVALID_PARAMETER);
}

TEST_F(MetaVolumeContainerTexture, CheckLookupFile1_Positive)
{
    std::string fileName = "TESTFILE";

    EXPECT_CALL(*ssdVolume, LookupDescriptorByName).WillOnce(Return(1));

    POS_EVENT_ID rc = container->LookupMetaVolumeType(fileName, MetaVolumeType::SsdVolume);
    EXPECT_EQ(rc, POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaVolumeContainerTexture, CheckLookupFile2_Negative)
{
    std::string fileName = "TESTFILE";
    FileDescriptorType invalid = MetaFsCommonConst::INVALID_FD;

    EXPECT_CALL(*ssdVolume, LookupDescriptorByName).WillOnce(Return(invalid));

    POS_EVENT_ID rc = container->LookupMetaVolumeType(fileName, MetaVolumeType::SsdVolume);
    EXPECT_EQ(rc, POS_EVENT_ID::MFS_INVALID_PARAMETER);
}

TEST_F(MetaVolumeContainerTexture, DetermineVolume_Positive0)
{
    FileSizeType size = 10;
    MetaFilePropertySet prop;
    MetaVolumeType type = MetaVolumeType::NvRamVolume;

    EXPECT_CALL(*nvramVolume, IsOkayToStore).WillRepeatedly(Return(true));

    POS_EVENT_ID result = container->DetermineVolumeToCreateFile(size, prop, type);

    EXPECT_EQ(result, POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaVolumeContainerTexture, DetermineVolume_Positive1)
{
    FileSizeType size = 10;
    MetaFilePropertySet prop;
    MetaVolumeType type = MetaVolumeType::SsdVolume;

    EXPECT_CALL(*ssdVolume, IsOkayToStore).WillRepeatedly(Return(true));

    POS_EVENT_ID result = container->DetermineVolumeToCreateFile(size, prop, type);

    EXPECT_EQ(result, POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaVolumeContainerTexture, DetermineVolume_Negative)
{
    FileSizeType size = 10;
    MetaFilePropertySet prop;
    MetaVolumeType type = MetaVolumeType::NvRamVolume;

    EXPECT_CALL(*nvramVolume, IsOkayToStore).WillRepeatedly(Return(false));

    POS_EVENT_ID result = container->DetermineVolumeToCreateFile(size, prop, type);

    EXPECT_EQ(result, POS_EVENT_ID::MFS_META_VOLUME_NOT_ENOUGH_SPACE);
}

TEST_F(MetaVolumeContainerTexture, CheckTheLastLpn)
{
    MetaVolumeType type = MetaVolumeType::SsdVolume;

    EXPECT_CALL(*ssdVolume, GetTheLastValidLpn).WillOnce(Return(100));

    EXPECT_EQ(container->GetTheLastValidLpn(type), 100);
}
} // namespace pos
