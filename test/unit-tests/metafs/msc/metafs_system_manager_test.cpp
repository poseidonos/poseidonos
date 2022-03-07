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

#include "src/metafs/msc/metafs_system_manager.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"
#include "test/unit-tests/metafs/msc/metafs_mbr_mgr_mock.h"
#include "test/unit-tests/metafs/msc/msc_req_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
class MetaFsSystemManagerFixture : public ::testing::Test
{
public:
    MetaFsSystemManagerFixture(void)
    : systemMgr(nullptr),
      mss(nullptr),
      mbr(nullptr)
    {
    }

    virtual ~MetaFsSystemManagerFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        mss = new NiceMock<MockMetaStorageSubsystem>(arrayId);
        mbr = new NiceMock<MockMetaFsMBRManager>(arrayId);

        systemMgr = new MetaFsSystemManager(arrayId, mss, mbr);
    }

    virtual void
    TearDown(void)
    {
        delete systemMgr;
        delete mss;
    }

protected:
    MetaFsSystemManager* systemMgr;

    NiceMock<MockMetaStorageSubsystem>* mss;
    NiceMock<MockMetaFsMBRManager>* mbr;

    int arrayId = 0;
};

TEST_F(MetaFsSystemManagerFixture, Initialize)
{
    std::shared_ptr<MetaStorageInfo> info = std::make_shared<MetaStorageInfo>();
    MetaStorageInfoList mediaInfoList;
    mediaInfoList.push_back(info);

    EXPECT_CALL(*mbr, Init);
    EXPECT_CALL(*mbr, SetMss);
    EXPECT_CALL(*mbr, RegisterVolumeGeometry);

    EXPECT_EQ(systemMgr->Init(mediaInfoList), true);
}

TEST_F(MetaFsSystemManagerFixture, CheckSignature)
{
    EXPECT_CALL(*mbr, GetEpochSignature).WillOnce(Return(1234));

    EXPECT_EQ(systemMgr->GetEpochSignature(), 1234);
}

TEST_F(MetaFsSystemManagerFixture, CheckReqSanity_Positive)
{
    MockMetaFsControlReqMsg req;

    EXPECT_CALL(req, IsValid).WillOnce(Return(true));

    EXPECT_EQ(systemMgr->CheckReqSanity(req), POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaFsSystemManagerFixture, CheckReqSanity_Negative)
{
    MockMetaFsControlReqMsg req;

    EXPECT_CALL(req, IsValid).WillOnce(Return(false));

    EXPECT_EQ(systemMgr->CheckReqSanity(req), POS_EVENT_ID::MFS_INVALID_PARAMETER);
}

TEST_F(MetaFsSystemManagerFixture, CheckPartitionInfo)
{
    MetaFsStorageIoInfoList info;

    EXPECT_CALL(*mbr, GetAllStoragePartitionInfo).WillOnce(ReturnRef(info));

    EXPECT_EQ(&(systemMgr->GetAllStoragePartitionInfo()), &info);
}

TEST_F(MetaFsSystemManagerFixture, CheckRegionSize)
{
    EXPECT_CALL(*mbr, GetRegionSizeInLpn).WillOnce(Return(1024));

    EXPECT_EQ(systemMgr->GetRegionSizeInLpn(), 1024);
}

TEST_F(MetaFsSystemManagerFixture, LoadMbr_Positive)
{
    bool isNpor = true;

    EXPECT_CALL(*mbr, LoadMBR).WillOnce(Return(true));
    EXPECT_CALL(*mbr, IsValidMBRExist).WillOnce(Return(true));
    EXPECT_CALL(*mbr, GetPowerStatus).WillOnce(Return(true));

    EXPECT_EQ(systemMgr->LoadMbr(isNpor), POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaFsSystemManagerFixture, LoadMbr_Negative0)
{
    bool isNpor = true;

    EXPECT_CALL(*mbr, LoadMBR).WillOnce(Return(true));
    EXPECT_CALL(*mbr, IsValidMBRExist).WillOnce(Return(false));

    EXPECT_EQ(systemMgr->LoadMbr(isNpor), POS_EVENT_ID::MFS_INVALID_MBR);
}

TEST_F(MetaFsSystemManagerFixture, LoadMbr_Negative1)
{
    bool isNpor = true;

    EXPECT_CALL(*mbr, LoadMBR).WillOnce(Return(false));

    EXPECT_EQ(systemMgr->LoadMbr(isNpor), POS_EVENT_ID::MFS_META_LOAD_FAILED);
}

TEST_F(MetaFsSystemManagerFixture, CheckClean_Positive)
{
    EXPECT_CALL(*mbr, GetEpochSignature).WillOnce(Return(0));

    EXPECT_EQ(systemMgr->IsMbrClean(), true);
}

TEST_F(MetaFsSystemManagerFixture, CheckClean_Negative)
{
    EXPECT_CALL(*mbr, GetEpochSignature).WillOnce(Return(1));

    EXPECT_EQ(systemMgr->IsMbrClean(), false);
}

TEST_F(MetaFsSystemManagerFixture, CreateMbr_Positive)
{
    EXPECT_CALL(*mbr, CreateMBR).WillOnce(Return(true));

    EXPECT_EQ(systemMgr->CreateMbr(), true);
}

TEST_F(MetaFsSystemManagerFixture, CreateMbr_Negative)
{
    EXPECT_CALL(*mbr, CreateMBR).WillOnce(Return(false));

    EXPECT_EQ(systemMgr->CreateMbr(), false);
}

TEST_F(MetaFsSystemManagerFixture, InitSystem_Positive)
{
    std::shared_ptr<MetaStorageInfo> info = std::make_shared<MetaStorageInfo>();
    MetaStorageInfoList mediaInfoList;
    mediaInfoList.push_back(info);

    MetaFsControlReqMsg req;
    req.reqType = MetaFsControlReqType::InitializeSystem;
    req.mediaList = &mediaInfoList;

    EXPECT_CALL(*mbr, Init);
    EXPECT_CALL(*mbr, SetMss);
    EXPECT_CALL(*mbr, RegisterVolumeGeometry);

    EXPECT_EQ(systemMgr->ProcessNewReq(req), POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaFsSystemManagerFixture, CloseSystem_Positive)
{
    MetaFsControlReqMsg req;
    req.reqType = MetaFsControlReqType::CloseSystem;

    EXPECT_CALL(*mbr, SetPowerStatus);
    EXPECT_CALL(*mbr, SaveContent).WillOnce(Return(true));
    EXPECT_CALL(*mbr, InvalidMBR);

    EXPECT_EQ(systemMgr->ProcessNewReq(req), POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaFsSystemManagerFixture, CloseSystem_Negative0)
{
    MetaFsControlReqMsg req;
    req.reqType = MetaFsControlReqType::CloseSystem;

    EXPECT_CALL(*mbr, SetPowerStatus);
    EXPECT_CALL(*mbr, SaveContent).WillOnce(Return(false));

    EXPECT_EQ(systemMgr->ProcessNewReq(req), POS_EVENT_ID::MFS_META_SAVE_FAILED);
}

TEST_F(MetaFsSystemManagerFixture, CloseSystem_Negative1)
{
    MetaFsControlReqMsg req;
    req.reqType = MetaFsControlReqType::CloseSystem;

    EXPECT_CALL(*mbr, SetPowerStatus);
    EXPECT_CALL(*mbr, SaveContent).WillOnce(Return(true));
    EXPECT_CALL(*mbr, InvalidMBR);
    EXPECT_CALL(*mss, Close).WillOnce(Return(POS_EVENT_ID::MFS_META_STORAGE_CLOSE_FAILED));

    EXPECT_EQ(systemMgr->ProcessNewReq(req), POS_EVENT_ID::MFS_META_STORAGE_CLOSE_FAILED);
}
} // namespace pos
