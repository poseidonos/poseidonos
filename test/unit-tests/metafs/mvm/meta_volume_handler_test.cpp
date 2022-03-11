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

#include "src/metafs/mvm/meta_volume_handler.h"
#include "test/unit-tests/metafs/mvm/meta_volume_container_mock.h"
#include <string>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
class MetaVolumeHandlerFixture : public ::testing::Test
{
public:
    MetaVolumeHandlerFixture(void)
    {
    }

    virtual ~MetaVolumeHandlerFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        handler = new MetaVolumeHandler();
        container = new NiceMock<MockMetaVolumeContainer>(arrayId);

        handler->InitHandler(container);

        msg.fileName = &fileName;
        msg.fileByteSize = size;
        msg.volType = MetaVolumeType::SsdVolume;
    }

    virtual void
    TearDown(void)
    {
        delete handler;
        delete container;
    }

protected:
    MetaVolumeHandler* handler;
    NiceMock<MockMetaVolumeContainer>* container;

    MetaVolumeType volType = MetaVolumeType::SsdVolume;
    MetaFsFileControlRequest msg;

    std::string fileName = "TESTFILE";
    FileSizeType size = 100;
    int arrayId = 0;
};

TEST_F(MetaVolumeHandlerFixture, CheckFileExist_Positive)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(true));

    EXPECT_EQ(handler->HandleCheckFileExist(volType, msg),
        POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaVolumeHandlerFixture, CheckFileExist_Negative)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(false));

    EXPECT_EQ(handler->HandleCheckFileExist(volType, msg),
        POS_EVENT_ID::MFS_FILE_NOT_FOUND);
}

TEST_F(MetaVolumeHandlerFixture, HandleOpen_Positive)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(true));
    EXPECT_CALL(*container, LookupFileDescByName);
    EXPECT_CALL(*container, AddFileInActiveList)
            .WillOnce(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(handler->HandleOpenFileReq(volType, msg),
        POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaVolumeHandlerFixture, HandleOpen_Negative0)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(false));

    EXPECT_EQ(handler->HandleOpenFileReq(volType, msg),
        POS_EVENT_ID::MFS_FILE_NOT_FOUND);
}

TEST_F(MetaVolumeHandlerFixture, HandleOpen_Negative1)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(true));
    EXPECT_CALL(*container, LookupFileDescByName);
    EXPECT_CALL(*container, AddFileInActiveList)
            .WillOnce(Return(POS_EVENT_ID::MFS_FILE_OPEN_REPETITIONARY));

    EXPECT_EQ(handler->HandleOpenFileReq(volType, msg),
        POS_EVENT_ID::MFS_FILE_OPEN_REPETITIONARY);
}

TEST_F(MetaVolumeHandlerFixture, HandleOpen_Negative2)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(true));
    EXPECT_CALL(*container, LookupFileDescByName)
            .WillOnce(Return(MetaFsCommonConst::INVALID_FD));
    EXPECT_CALL(*container, AddFileInActiveList)
            .WillOnce(Return(POS_EVENT_ID::SUCCESS));

    EXPECT_EQ(handler->HandleOpenFileReq(volType, msg),
        POS_EVENT_ID::MFS_FILE_OPEN_FAILED);
}

TEST_F(MetaVolumeHandlerFixture, HandleClose_Positive)
{
    EXPECT_CALL(*container, CheckFileInActive)
            .WillOnce(Return(true));
    EXPECT_CALL(*container, RemoveFileFromActiveList);

    EXPECT_EQ(handler->HandleCloseFileReq(volType, msg),
        POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaVolumeHandlerFixture, HandleClose_Negative)
{
    EXPECT_CALL(*container, CheckFileInActive)
            .WillOnce(Return(false));

    EXPECT_EQ(handler->HandleCloseFileReq(volType, msg),
        POS_EVENT_ID::MFS_FILE_NOT_OPENED);
}

TEST_F(MetaVolumeHandlerFixture, HandleCreate_Positive)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(false));
    EXPECT_CALL(*container, GetAvailableSpace)
            .WillOnce(Return(size + 1));
    EXPECT_CALL(*container, CreateFile)
            .WillOnce(Return(true));

    EXPECT_EQ(handler->HandleCreateFileReq(volType, msg),
        POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaVolumeHandlerFixture, HandleCreate_Negative0)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(true));

    EXPECT_EQ(handler->HandleCreateFileReq(volType, msg),
        POS_EVENT_ID::MFS_FILE_CREATE_FAILED);
}

TEST_F(MetaVolumeHandlerFixture, HandleCreate_Negative1)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(false));
    EXPECT_CALL(*container, GetAvailableSpace)
            .WillOnce(Return(size));

    EXPECT_EQ(handler->HandleCreateFileReq(volType, msg),
        POS_EVENT_ID::MFS_FILE_CREATE_FAILED);
}

TEST_F(MetaVolumeHandlerFixture, HandleCreate_Negative2)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(false));
    EXPECT_CALL(*container, GetAvailableSpace)
            .WillOnce(Return(size + 1));
    EXPECT_CALL(*container, CreateFile)
            .WillOnce(Return(false));

    EXPECT_EQ(handler->HandleCreateFileReq(volType, msg),
        POS_EVENT_ID::MFS_FILE_CREATE_FAILED);
}

TEST_F(MetaVolumeHandlerFixture, HandleDelete_Positive)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(true));
    EXPECT_CALL(*container, TrimData)
            .WillOnce(Return(true));
    EXPECT_CALL(*container, DeleteFile)
            .WillOnce(Return(true));

    EXPECT_EQ(handler->HandleDeleteFileReq(volType, msg),
        POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaVolumeHandlerFixture, HandleDelete_Negative0)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(false));

    EXPECT_EQ(handler->HandleDeleteFileReq(volType, msg),
        POS_EVENT_ID::MFS_FILE_NOT_FOUND);
}

TEST_F(MetaVolumeHandlerFixture, HandleDelete_Negative1)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(true));
    EXPECT_CALL(*container, TrimData)
            .WillOnce(Return(false));

    EXPECT_EQ(handler->HandleDeleteFileReq(volType, msg),
        POS_EVENT_ID::MFS_FILE_TRIM_FAILED);
}

TEST_F(MetaVolumeHandlerFixture, HandleDelete_Negative2)
{
    EXPECT_CALL(*container, IsGivenFileCreated)
            .WillOnce(Return(true));
    EXPECT_CALL(*container, TrimData)
            .WillOnce(Return(true));
    EXPECT_CALL(*container, DeleteFile)
            .WillOnce(Return(false));

    EXPECT_EQ(handler->HandleDeleteFileReq(volType, msg),
        POS_EVENT_ID::MFS_FILE_DELETE_FAILED);
}

TEST_F(MetaVolumeHandlerFixture, CheckAccessible_Positive)
{
    EXPECT_CALL(*container, CheckFileInActive)
            .WillOnce(Return(true));

    EXPECT_EQ(handler->HandleCheckFileAccessibleReq(volType, msg),
        POS_EVENT_ID::SUCCESS);

    EXPECT_EQ(msg.completionData.fileAccessible, true);
}

TEST_F(MetaVolumeHandlerFixture, CheckAccessible_Negative)
{
    EXPECT_CALL(*container, CheckFileInActive)
            .WillOnce(Return(false));

    EXPECT_EQ(handler->HandleCheckFileAccessibleReq(volType, msg),
        POS_EVENT_ID::SUCCESS);

    EXPECT_EQ(msg.completionData.fileAccessible, false);
}

TEST_F(MetaVolumeHandlerFixture, CheckChunkSize)
{
    EXPECT_CALL(*container, GetDataChunkSize)
            .WillOnce(Return(4032));

    EXPECT_EQ(handler->HandleGetDataChunkSizeReq(volType, msg),
        POS_EVENT_ID::SUCCESS);

    EXPECT_EQ(msg.completionData.dataChunkSize, 4032);
}

TEST_F(MetaVolumeHandlerFixture, CheckFileSize)
{
    EXPECT_CALL(*container, GetFileSize)
            .WillOnce(Return(4032));

    EXPECT_EQ(handler->HandleGetFileSizeReq(volType, msg),
        POS_EVENT_ID::SUCCESS);

    EXPECT_EQ(msg.completionData.fileSize, 4032);
}

TEST_F(MetaVolumeHandlerFixture, CheckMediaType)
{
    EXPECT_EQ(handler->HandleGetTargetMediaTypeReq(volType, msg),
        POS_EVENT_ID::SUCCESS);

    EXPECT_EQ(msg.completionData.targetMediaType, MetaStorageType::SSD);
}

TEST_F(MetaVolumeHandlerFixture, CheckFileBaseLpn)
{
    EXPECT_CALL(*container, GetFileBaseLpn)
            .WillOnce(Return(1234));

    EXPECT_EQ(handler->HandleGetFileBaseLpnReq(volType, msg),
        POS_EVENT_ID::SUCCESS);

    EXPECT_EQ(msg.completionData.fileBaseLpn, 1234);
}

TEST_F(MetaVolumeHandlerFixture, CheckExtentSize)
{
    EXPECT_CALL(*container, GetAvailableSpace)
            .WillOnce(Return(9999));

    EXPECT_EQ(handler->HandleGetFreeFileRegionSizeReq(volType, msg),
        POS_EVENT_ID::SUCCESS);

    EXPECT_EQ(msg.completionData.fileSize, 9999);
}

TEST_F(MetaVolumeHandlerFixture, CheckCreateArray)
{
    EXPECT_EQ(handler->HandleCreateArrayReq(volType, msg),
        POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaVolumeHandlerFixture, CheckDeleteArray)
{
    EXPECT_EQ(handler->HandleDeleteArrayReq(volType, msg),
        POS_EVENT_ID::SUCCESS);
}

TEST_F(MetaVolumeHandlerFixture, CheckMaxLpn)
{
    EXPECT_CALL(*container, GetMaxLpn)
            .WillOnce(Return(9999));

    EXPECT_EQ(handler->HandleGetMaxMetaLpnReq(volType, msg),
        POS_EVENT_ID::SUCCESS);

    EXPECT_EQ(msg.completionData.maxLpn, 9999);
}

TEST_F(MetaVolumeHandlerFixture, CheckInodeList_Positive)
{
    std::vector<MetaFileInfoDumpCxt>* fileInfoList
                = new std::vector<MetaFileInfoDumpCxt>;
    msg.completionData.fileInfoListPointer = fileInfoList;

    EXPECT_CALL(*container, GetInodeList);

    EXPECT_EQ(handler->HandleGetMetaFileInodeListReq(msg),
        POS_EVENT_ID::SUCCESS);

    delete fileInfoList;
    msg.completionData.fileInfoListPointer = nullptr;
}

TEST_F(MetaVolumeHandlerFixture, CheckInodeList_Negative)
{
    msg.completionData.fileInfoListPointer = nullptr;

    EXPECT_EQ(handler->HandleGetMetaFileInodeListReq(msg),
        POS_EVENT_ID::MFS_INVALID_PARAMETER);

    msg.completionData.fileInfoListPointer = nullptr;
}

TEST_F(MetaVolumeHandlerFixture, GetInode_Positive)
{
    MetaFileInode inode;

    EXPECT_CALL(*container, LookupMetaVolumeType(Matcher<std::string&>(_), _))
            .WillRepeatedly(Return(POS_EVENT_ID::SUCCESS));
    EXPECT_CALL(*container, LookupFileDescByName)
            .WillRepeatedly(Return(0));
    EXPECT_CALL(*container, GetInode)
            .WillRepeatedly(ReturnRef(inode));
    EXPECT_CALL(*container, CopyInodeToInodeInfo)
            .WillOnce(Return(true));

    EXPECT_EQ(handler->HandleGetFileInodeReq(msg),
        POS_EVENT_ID::SUCCESS);

    EXPECT_EQ(msg.completionData.inodeInfoPointer->data.field.dataLocation,
        MetaStorageType::SSD);
}

TEST_F(MetaVolumeHandlerFixture, GetInode_Negative)
{
    EXPECT_CALL(*container, LookupMetaVolumeType(Matcher<std::string&>(_), _))
            .WillRepeatedly(Return(POS_EVENT_ID::MFS_INVALID_PARAMETER));

    EXPECT_EQ(handler->HandleGetFileInodeReq(msg),
        POS_EVENT_ID::MFS_FILE_NOT_FOUND);
}

TEST_F(MetaVolumeHandlerFixture, CheckChunkSize2)
{
    EXPECT_EQ(handler->HandleEstimateDataChunkSizeReq(msg),
        POS_EVENT_ID::SUCCESS);

    EXPECT_EQ(msg.completionData.dataChunkSize, 4032);
}
} // namespace pos
