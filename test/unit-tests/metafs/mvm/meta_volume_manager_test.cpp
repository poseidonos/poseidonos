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

#include "src/metafs/mvm/meta_volume_manager.h"
#include "test/unit-tests/metafs/mvm/meta_volume_handler_mock.h"
#include "test/unit-tests/metafs/mvm/meta_volume_container_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"
#include "test/unit-tests/metafs/include/metafs_file_control_request_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
class MetaVolumeManagerFixture : public ::testing::Test
{
public:
    MetaVolumeManagerFixture(void)
    {
    }

    virtual ~MetaVolumeManagerFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        volContainer = new NiceMock<MockMetaVolumeContainer>(arrayId);
        volHandler = new NiceMock<MockMetaVolumeHandler>(volContainer);
        mss = new NiceMock<MockMetaStorageSubsystem>(arrayId);

        volumeMgr = new MetaVolumeManager(arrayId, mss, nullptr, volHandler, volContainer);
    }

    virtual void
    TearDown(void)
    {
        delete volumeMgr;
    }

protected:
    MetaVolumeManager* volumeMgr;

    NiceMock<MockMetaVolumeHandler>* volHandler;
    NiceMock<MockMetaVolumeContainer>* volContainer;
    NiceMock<MockMetaStorageSubsystem>* mss;

    int arrayId = 0;
    MetaVolumeType volumeType = MetaVolumeType::SsdVolume;
    MetaLpnType baseLpn = 0;
};

TEST(MetaVolumeManager, CreateObject)
{
    NiceMock<MockMetaStorageSubsystem>* mss;

    MetaVolumeManager mgr(0, mss);
}

TEST_F(MetaVolumeManagerFixture, CheckReqSanity_Positive)
{
    MockMetaFsFileControlRequest req;

    EXPECT_CALL(req, IsValid).WillOnce(Return(true));

    EXPECT_EQ(volumeMgr->CheckReqSanity(req), EID(SUCCESS));
}

TEST_F(MetaVolumeManagerFixture, CheckReqSanity_Negative)
{
    MockMetaFsFileControlRequest req;

    EXPECT_CALL(req, IsValid).WillOnce(Return(false));

    EXPECT_EQ(volumeMgr->CheckReqSanity(req), EID(MFS_INVALID_PARAMETER));
}

TEST_F(MetaVolumeManagerFixture, InitModule0)
{
    EXPECT_CALL(*volContainer, IsGivenVolumeExist).WillOnce(Return(true));

    volumeMgr->InitVolume(MetaVolumeType::SsdVolume, arrayId, 0);
}

TEST_F(MetaVolumeManagerFixture, InitModule1)
{
    EXPECT_CALL(*volContainer, IsGivenVolumeExist).WillOnce(Return(false));
    EXPECT_CALL(*volContainer, InitContext);

    volumeMgr->InitVolume(MetaVolumeType::SsdVolume, arrayId, 0);
}

TEST_F(MetaVolumeManagerFixture, CreateVolume_Positive)
{
    MetaVolumeType volType = MetaVolumeType::SsdVolume;

    EXPECT_CALL(*volContainer, CreateVolume).WillOnce(Return(true));

    EXPECT_EQ(volumeMgr->CreateVolume(volType), true);
}

TEST_F(MetaVolumeManagerFixture, CreateVolume_Negative)
{
    MetaVolumeType volType = MetaVolumeType::SsdVolume;

    EXPECT_CALL(*volContainer, CreateVolume).WillOnce(Return(false));

    EXPECT_EQ(volumeMgr->CreateVolume(volType), false);
}

TEST_F(MetaVolumeManagerFixture, OpenVolume_Positive)
{
    bool npor = true;

    EXPECT_CALL(*volContainer, OpenAllVolumes).WillOnce(Return(true));

    EXPECT_EQ(volumeMgr->OpenVolume(npor), true);
}

TEST_F(MetaVolumeManagerFixture, OpenVolume_Negative)
{
    bool npor = true;

    EXPECT_CALL(*volContainer, OpenAllVolumes).WillOnce(Return(false));

    EXPECT_EQ(volumeMgr->OpenVolume(npor), false);
}

TEST_F(MetaVolumeManagerFixture, CloseVolume_Positive)
{
    bool npor = true;

    EXPECT_CALL(*volContainer, CloseAllVolumes).WillOnce(Return(true));

    EXPECT_EQ(volumeMgr->CloseVolume(npor), true);
}

TEST_F(MetaVolumeManagerFixture, CloseVolume_Negative)
{
    bool npor = true;

    EXPECT_CALL(*volContainer, CloseAllVolumes).WillOnce(Return(false));

    EXPECT_EQ(volumeMgr->CloseVolume(npor), false);
}

TEST_F(MetaVolumeManagerFixture, CheckFileAccessible_Negative_DueTo_VolumeType)
{
    FileDescriptorType fd = 0;
    MetaVolumeType volType = MetaVolumeType::Max;

    POS_EVENT_ID rc = volumeMgr->CheckFileAccessible(fd, volType);

    EXPECT_EQ(rc, EID(MFS_INVALID_PARAMETER));
}

TEST_F(MetaVolumeManagerFixture, CheckFileAccessible_Negative_DueTo_LookupFail)
{
    FileDescriptorType fd = 0;
    MetaVolumeType volType = MetaVolumeType::SsdVolume;

    EXPECT_CALL(*volContainer, LookupMetaVolumeType(Matcher<FileDescriptorType>(_), _))
        .WillOnce(Return(EID(MFS_INVALID_PARAMETER)));

    POS_EVENT_ID rc = volumeMgr->CheckFileAccessible(fd, volType);

    EXPECT_EQ(rc, EID(MFS_INVALID_PARAMETER));
}

TEST_F(MetaVolumeManagerFixture, CheckFileAccessible_Positive)
{
    FileDescriptorType fd = 0;
    MetaVolumeType volType = MetaVolumeType::SsdVolume;

    EXPECT_CALL(*volContainer, LookupMetaVolumeType(Matcher<FileDescriptorType>(_), _))
        .WillOnce(Return(EID(SUCCESS)));

    POS_EVENT_ID rc = volumeMgr->CheckFileAccessible(fd, volType);

    // can't control req.completionData.fileAccessible
    EXPECT_EQ(rc, EID(MFS_FILE_INACTIVATED));
}

TEST_F(MetaVolumeManagerFixture, GetFileSize_Positive)
{
    FileDescriptorType fd = 0;
    MetaVolumeType volType = MetaVolumeType::SsdVolume;
    FileSizeType outFileByteSize;

    EXPECT_CALL(*volContainer, LookupMetaVolumeType(Matcher<FileDescriptorType>(_), _))
        .WillOnce(Return(EID(SUCCESS)));

    POS_EVENT_ID rc = volumeMgr->GetFileSize(fd, volType, outFileByteSize);

    EXPECT_EQ(rc, EID(SUCCESS));
    EXPECT_EQ(outFileByteSize, 0);
}

TEST_F(MetaVolumeManagerFixture, GetDataChunkSize_Positive)
{
    FileDescriptorType fd = 0;
    MetaVolumeType volType = MetaVolumeType::SsdVolume;
    FileSizeType outFileByteSize;

    EXPECT_CALL(*volContainer, LookupMetaVolumeType(Matcher<FileDescriptorType>(_), _))
        .WillOnce(Return(EID(SUCCESS)));

    POS_EVENT_ID rc = volumeMgr->GetDataChunkSize(fd, volType, outFileByteSize);

    EXPECT_EQ(rc, EID(SUCCESS));
    EXPECT_EQ(outFileByteSize, 0);
}

TEST_F(MetaVolumeManagerFixture, GetDataChunkSize_Negative)
{
    FileDescriptorType fd = 0;
    MetaVolumeType volType = MetaVolumeType::SsdVolume;
    FileSizeType outFileByteSize;

    EXPECT_CALL(*volContainer, LookupMetaVolumeType(Matcher<FileDescriptorType>(_), _))
        .WillOnce(Return(EID(MFS_INVALID_PARAMETER)));

    POS_EVENT_ID rc = volumeMgr->GetDataChunkSize(fd, volType, outFileByteSize);

    EXPECT_EQ(rc, EID(MFS_INVALID_PARAMETER));
}

TEST_F(MetaVolumeManagerFixture, GetTargetMediaType_Positive)
{
    FileDescriptorType fd = 0;
    MetaVolumeType volType = MetaVolumeType::SsdVolume;
    MetaStorageType type;

    EXPECT_CALL(*volContainer, LookupMetaVolumeType(Matcher<FileDescriptorType>(_), _))
        .WillOnce(Return(EID(SUCCESS)));

    POS_EVENT_ID rc = volumeMgr->GetTargetMediaType(fd, volType, type);

    EXPECT_EQ(rc, EID(SUCCESS));
    EXPECT_EQ(type, MetaStorageType::SSD);
}

TEST_F(MetaVolumeManagerFixture, GetFileBaseLpn_Positive)
{
    FileDescriptorType fd = 0;
    MetaVolumeType volType = MetaVolumeType::SsdVolume;
    MetaLpnType outFileBaseLpn;

    EXPECT_CALL(*volContainer, LookupMetaVolumeType(Matcher<FileDescriptorType>(_), _))
        .WillOnce(Return(EID(SUCCESS)));

    POS_EVENT_ID rc = volumeMgr->GetFileBaseLpn(fd, volType, outFileBaseLpn);

    EXPECT_EQ(rc, EID(SUCCESS));
    EXPECT_EQ(outFileBaseLpn, 0);
}

TEST_F(MetaVolumeManagerFixture, CheckTheLastLpn)
{
    MetaVolumeType volType = MetaVolumeType::SsdVolume;

    EXPECT_CALL(*volContainer, GetTheLastValidLpn(_)).WillOnce(Return(100));

    EXPECT_EQ(volumeMgr->GetTheLastValidLpn(volType), 100);
}
} // namespace pos
