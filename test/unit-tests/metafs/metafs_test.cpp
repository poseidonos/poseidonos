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

#include "src/metafs/metafs.h"
#include "test/unit-tests/metafs/include/metafs_mock.h"
#include "test/unit-tests/metafs/mai/metafs_file_control_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_io_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_management_api_mock.h"
#include "test/unit-tests/metafs/mai/metafs_wbt_api_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"
#include <gtest/gtest.h>
#include <string>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace std;

namespace pos
{
class MetaFsFixture : public ::testing::Test
{
public:
    MetaFsFixture(void)
    {
    }

    virtual ~MetaFsFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        ptnSize.totalStripes = 100;
        ptnSize.blksPerStripe = 10;

        arrayInfo = new NiceMock<MockIArrayInfo>;
        mgmt = new NiceMock<MockMetaFsManagementApi>;
        ctrl = new NiceMock<MockMetaFsFileControlApi>;
        io = new NiceMock<MockMetaFsIoApi>;
        wbt = new NiceMock<MockMetaFsWBTApi>;
        mss = new NiceMock<MockMetaStorageSubsystem>(arrayId);

        tp = new NiceMock<MockTelemetryPublisher>;

        for (int i = 0; i < MetaFsGeometryInfo::MAX_INFO_COUNT; ++i)
        {
            mediaInfoList[i].valid = false;
        }

        metaFs = new MetaFs(arrayInfo, false, mgmt, ctrl, io, wbt, mss, tp);
    }

    virtual void
    TearDown(void)
    {
        delete metaFs;
        delete arrayInfo;
    }

protected:
    MetaFs* metaFs;
    NiceMock<MockIArrayInfo>* arrayInfo;
    NiceMock<MockMetaFsManagementApi>* mgmt;
    NiceMock<MockMetaFsFileControlApi>* ctrl;
    NiceMock<MockMetaFsIoApi>* io;
    NiceMock<MockMetaFsWBTApi>* wbt;
    NiceMock<MockMetaStorageSubsystem>* mss;
    NiceMock<MockTelemetryPublisher>* tp;

    PartitionLogicalSize ptnSize;
    MetaFsStorageIoInfoList mediaInfoList;
    string fileName = "TestFile";
    uint64_t fileSize = 100;
    int arrayId = 0;
};

TEST_F(MetaFsFixture, InitMetaFs)
{
    EXPECT_CALL(*mgmt, InitializeSystem(_, _))
        .WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*arrayInfo, GetSizeInfo).WillRepeatedly(Return(&ptnSize));
    EXPECT_CALL(*mgmt, GetAllStoragePartitionInfo)
        .WillRepeatedly(ReturnRef(mediaInfoList));
    EXPECT_CALL(*ctrl, CreateVolume).WillRepeatedly(Return(true));
    EXPECT_CALL(*mgmt, CreateMbr).WillRepeatedly(Return(true));
    EXPECT_CALL(*ctrl, OpenVolume).WillOnce(Return(true));
    EXPECT_CALL(*io, AddArray).WillOnce(Return(true));
    EXPECT_CALL(*mgmt, SetStatus);
    EXPECT_CALL(*io, SetStatus);
    EXPECT_CALL(*ctrl, SetStatus);
    EXPECT_CALL(*wbt, SetStatus);

    EXPECT_EQ(metaFs->Init(), 0);
}

TEST_F(MetaFsFixture, CheckFlush)
{
    metaFs->Flush();
}

TEST_F(MetaFsFixture, CheckDispose)
{
    EXPECT_CALL(*ctrl, CloseVolume(_)).WillOnce(Return(true));

    metaFs->Dispose();
}

TEST_F(MetaFsFixture, CheckShutdown_Without_Storage)
{
    EXPECT_CALL(*io, RemoveArray(_));

    metaFs->Shutdown();
}

TEST_F(MetaFsFixture, CheckShutdown_With_Storage)
{
    EXPECT_CALL(*mgmt, InitializeSystem(_, _))
        .WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*arrayInfo, GetSizeInfo).WillRepeatedly(Return(&ptnSize));
    EXPECT_CALL(*mgmt, GetAllStoragePartitionInfo)
        .WillRepeatedly(ReturnRef(mediaInfoList));
    EXPECT_CALL(*ctrl, CreateVolume).WillRepeatedly(Return(true));
    EXPECT_CALL(*mgmt, CreateMbr).WillRepeatedly(Return(true));
    EXPECT_CALL(*ctrl, OpenVolume).WillOnce(Return(true));
    EXPECT_CALL(*io, AddArray).WillOnce(Return(true));
    EXPECT_CALL(*mgmt, SetStatus);
    EXPECT_CALL(*io, SetStatus);
    EXPECT_CALL(*ctrl, SetStatus);
    EXPECT_CALL(*wbt, SetStatus);

    EXPECT_EQ(metaFs->Init(), 0);

    EXPECT_CALL(*io, RemoveArray(_));
    EXPECT_CALL(*mss, Close);

    metaFs->Shutdown();
}

TEST_F(MetaFsFixture, CheckEpochSignature)
{
    EXPECT_CALL(*mgmt, GetEpochSignature).WillOnce(Return(123456));

    EXPECT_EQ(metaFs->GetEpochSignature(), 123456);
}

TEST_F(MetaFsFixture, CheckTheLastStripeId)
{
    LogicalBlkAddr addr = {1, 10};

    EXPECT_CALL(*ctrl, GetTheLastValidLpn(_)).WillOnce(Return(100));
    EXPECT_CALL(*mss, TranslateAddress(_, _)).WillOnce(Return(addr));

    EXPECT_EQ(metaFs->GetTheLastValidStripeId(), 1);
}
} // namespace pos
