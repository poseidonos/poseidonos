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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "test/unit-tests/metafs/mvm/meta_volume_container_mock.h"

using ::testing::_;
using ::testing::Matcher;
using ::testing::NiceMock;
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
        container = new NiceMock<MockMetaVolumeContainer>(arrayId);
        handler = new MetaVolumeHandler(container);

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

TEST_F(MetaVolumeHandlerFixture, HandleCheckFileExist_testIfTheMethodReturnsPositiveWhenTheVolumeAndFilenameAreValid)
{
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(true));
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(true));

    EXPECT_EQ(handler->HandleCheckFileExist(volType, msg),
        EID(SUCCESS));
}

TEST_F(MetaVolumeHandlerFixture, HandleCheckFileExist_testIfTheMethodReturnsNegativeWhenTheVolumeIsInvalid)
{
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(false));

    EXPECT_EQ(handler->HandleCheckFileExist(volType, msg),
        EID(MFS_META_VOLUME_NOT_FOUND));
}

TEST_F(MetaVolumeHandlerFixture, HandleCheckFileExist_testIfTheMethodReturnsNegativeWhenTheFilenameIsInvalid)
{
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(true));
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(false));

    EXPECT_EQ(handler->HandleCheckFileExist(volType, msg),
        EID(MFS_FILE_NOT_FOUND));
}

TEST_F(MetaVolumeHandlerFixture, HandleOpenFileReq_testIfTheFileCanBeOpenedWhenAllConditionsAreGood)
{
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(true));
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(true));
    EXPECT_CALL(*container, LookupFileDescByName);
    EXPECT_CALL(*container, AddFileInActiveList).WillOnce(Return(EID(SUCCESS)));

    EXPECT_EQ(handler->HandleOpenFileReq(volType, msg),
        EID(SUCCESS));
}

TEST_F(MetaVolumeHandlerFixture, HandleOpenFileReq_testIfTheFileIsNotValidWhenTheVolumeIsInvalid)
{
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(false));

    EXPECT_EQ(handler->HandleOpenFileReq(volType, msg),
        EID(MFS_META_VOLUME_NOT_FOUND));
}

TEST_F(MetaVolumeHandlerFixture, HandleOpenFileReq_testIfTheFileIsNotValidWhenTheFilenameIsInvalid)
{
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(true));
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(false));

    EXPECT_EQ(handler->HandleOpenFileReq(volType, msg),
        EID(MFS_FILE_NOT_FOUND));
}

TEST_F(MetaVolumeHandlerFixture, HandleOpenFileReq_testIfTheFileCannotBeOpenedAgainWhenTheFileIsNotFound)
{
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(true));
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(true));
    EXPECT_CALL(*container, LookupFileDescByName).WillOnce(Return(MetaFsCommonConst::INVALID_FD));

    EXPECT_EQ(handler->HandleOpenFileReq(volType, msg),
        EID(MFS_FILE_NOT_FOUND));
}

TEST_F(MetaVolumeHandlerFixture, HandleOpenFileReq_testIfTheFileCannotBeOpenedAgainWhenTheFileIsAlreadyOpened)
{
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(true));
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(true));
    EXPECT_CALL(*container, LookupFileDescByName).WillOnce(Return(0));
    EXPECT_CALL(*container, AddFileInActiveList).WillOnce(Return(EID(MFS_FILE_OPEN_REPETITIONARY)));

    EXPECT_EQ(handler->HandleOpenFileReq(volType, msg),
        EID(MFS_FILE_OPEN_REPETITIONARY));
}

TEST_F(MetaVolumeHandlerFixture, HandleCloseFileReq_testIfTheFileCanBeClosed)
{
    EXPECT_CALL(*container, CheckFileInActive).WillOnce(Return(true));
    EXPECT_CALL(*container, RemoveFileFromActiveList);

    EXPECT_EQ(handler->HandleCloseFileReq(volType, msg),
        EID(SUCCESS));
}

TEST_F(MetaVolumeHandlerFixture, HandleCloseFileReq_testIfTheFileCannotBeClosedWhenTheFileIsNotOpened)
{
    EXPECT_CALL(*container, CheckFileInActive).WillOnce(Return(false));

    EXPECT_EQ(handler->HandleCloseFileReq(volType, msg),
        EID(MFS_FILE_NOT_OPENED));
}

TEST_F(MetaVolumeHandlerFixture, HandleCreateFileReq_testIfTheFileCanBeCreated)
{
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(false));
    EXPECT_CALL(*container, GetAvailableSpace).WillOnce(Return(size + 1));
    EXPECT_CALL(*container, CreateFile).WillOnce(Return(true));

    EXPECT_EQ(handler->HandleCreateFileReq(volType, msg),
        EID(SUCCESS));
}

TEST_F(MetaVolumeHandlerFixture, HandleCreateFileReq_testIfTheFileCannotBeCreatedWhenTheFilenameIsAlreadyOccupied)
{
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(true));
    EXPECT_CALL(*container, CreateFile).Times(0);

    EXPECT_EQ(handler->HandleCreateFileReq(volType, msg),
        EID(MFS_FILE_NAME_EXISTED));
}

TEST_F(MetaVolumeHandlerFixture, HandleCreateFileReq_testIfTheFileCannotBeCreatedWhenTheVolumeDoesNotHaveEnoughSpaceToCreate)
{
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(false));
    EXPECT_CALL(*container, GetAvailableSpace).WillOnce(Return(0));
    EXPECT_CALL(*container, CreateFile).Times(0);

    EXPECT_EQ(handler->HandleCreateFileReq(volType, msg),
        EID(MFS_META_VOLUME_NOT_ENOUGH_SPACE));
}

TEST_F(MetaVolumeHandlerFixture, HandleCreateFileReq_testIfTheFileCannotBeCreatedWhenTheVolumeDoesNotHaveEnoughSpaceToCreate2)
{
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(false));
    EXPECT_CALL(*container, GetAvailableSpace).WillOnce(Return(size + 1));
    EXPECT_CALL(*container, CreateFile).WillOnce(Return(false));

    EXPECT_EQ(handler->HandleCreateFileReq(volType, msg),
        EID(MFS_FILE_CREATE_FAILED));
}

TEST_F(MetaVolumeHandlerFixture, HandleDeleteFileReq_testIfTheFileCanBeCreated)
{
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(true));
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(true));
    EXPECT_CALL(*container, TrimData).WillOnce(Return(true));
    EXPECT_CALL(*container, DeleteFile).WillOnce(Return(true));

    EXPECT_EQ(handler->HandleDeleteFileReq(volType, msg),
        EID(SUCCESS));
}

TEST_F(MetaVolumeHandlerFixture, HandleDeleteFileReq_testIfTheFileCannotBeDeletedWhenTheVolumeIsInvalid)
{
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(false));

    EXPECT_EQ(handler->HandleDeleteFileReq(volType, msg),
        EID(MFS_META_VOLUME_NOT_FOUND));
}

TEST_F(MetaVolumeHandlerFixture, HandleDeleteFileReq_testIfTheFileCannotBeDeletedWhenTheFileIsNotExisted)
{
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(true));
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(false));

    EXPECT_EQ(handler->HandleDeleteFileReq(volType, msg),
        EID(MFS_FILE_NOT_FOUND));
}

TEST_F(MetaVolumeHandlerFixture, HandleDeleteFileReq_testIfTheFileCannotBeDeletedWhenTrimOperationIsFailed)
{
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(true));
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(true));
    EXPECT_CALL(*container, TrimData).WillOnce(Return(false));

    EXPECT_EQ(handler->HandleDeleteFileReq(volType, msg),
        EID(MFS_FILE_TRIM_FAILED));
}

TEST_F(MetaVolumeHandlerFixture, HandleDeleteFileReq_testIfTheFileCannotBeDeletedWhenDeleteOperationIsFailed)
{
    EXPECT_CALL(*container, IsGivenVolumeExist).WillOnce(Return(true));
    EXPECT_CALL(*container, IsGivenFileCreated).WillOnce(Return(true));
    EXPECT_CALL(*container, TrimData).WillOnce(Return(true));
    EXPECT_CALL(*container, DeleteFile).WillOnce(Return(false));

    EXPECT_EQ(handler->HandleDeleteFileReq(volType, msg),
        EID(MFS_FILE_DELETE_FAILED));
}

TEST_F(MetaVolumeHandlerFixture, HandleCheckFileAccessibleReq_testIfTheRequestSucceeds)
{
    EXPECT_CALL(*container, CheckFileInActive).WillOnce(Return(false));

    EXPECT_EQ(handler->HandleCheckFileAccessibleReq(volType, msg),
        EID(SUCCESS));

    EXPECT_EQ(msg.completionData.fileAccessible, false);
}

TEST_F(MetaVolumeHandlerFixture, HandleGetDataChunkSizeReq_testIfTheRequestSucceeds)
{
    EXPECT_CALL(*container, GetDataChunkSize).WillOnce(Return(4032));

    EXPECT_EQ(handler->HandleGetDataChunkSizeReq(volType, msg),
        EID(SUCCESS));

    EXPECT_EQ(msg.completionData.dataChunkSize, 4032);
}

TEST_F(MetaVolumeHandlerFixture, HandleGetFileSizeReq_testIfTheRequestSucceeds)
{
    EXPECT_CALL(*container, GetFileSize).WillOnce(Return(4032));

    EXPECT_EQ(handler->HandleGetFileSizeReq(volType, msg),
        EID(SUCCESS));

    EXPECT_EQ(msg.completionData.fileSize, 4032);
}

TEST_F(MetaVolumeHandlerFixture, HandleGetTargetMediaTypeReq_testIfTheRequestSucceeds)
{
    EXPECT_EQ(handler->HandleGetTargetMediaTypeReq(volType, msg), EID(SUCCESS));
    EXPECT_EQ(msg.completionData.targetMediaType, MetaStorageType::SSD);
}

TEST_F(MetaVolumeHandlerFixture, HandleGetFileBaseLpnReq_testIfTheRequestSucceeds)
{
    EXPECT_CALL(*container, GetFileBaseLpn).WillOnce(Return(1234));

    EXPECT_EQ(handler->HandleGetFileBaseLpnReq(volType, msg), EID(SUCCESS));
    EXPECT_EQ(msg.completionData.fileBaseLpn, 1234);
}

TEST_F(MetaVolumeHandlerFixture, HandleGetFreeFileRegionSizeReq_testIfTheRequestSucceeds)
{
    EXPECT_CALL(*container, GetAvailableSpace).WillOnce(Return(9999));

    EXPECT_EQ(handler->HandleGetFreeFileRegionSizeReq(volType, msg), EID(SUCCESS));
    EXPECT_EQ(msg.completionData.fileSize, 9999);
}

TEST_F(MetaVolumeHandlerFixture, HandleCreateArrayReq_testIfTheRequestSucceeds)
{
    EXPECT_EQ(handler->HandleCreateArrayReq(volType, msg), EID(SUCCESS));
}

TEST_F(MetaVolumeHandlerFixture, HandleDeleteArrayReq_testIfTheRequestSucceeds)
{
    EXPECT_EQ(handler->HandleDeleteArrayReq(volType, msg), EID(SUCCESS));
}

TEST_F(MetaVolumeHandlerFixture, HandleGetMaxMetaLpnReq_testIfTheRequestSucceeds)
{
    EXPECT_CALL(*container, GetMaxLpn).WillOnce(Return(9999));

    EXPECT_EQ(handler->HandleGetMaxMetaLpnReq(volType, msg), EID(SUCCESS));
    EXPECT_EQ(msg.completionData.maxLpn, 9999);
}

TEST_F(MetaVolumeHandlerFixture, HandleGetMetaFileInodeListReq_testIfTheRequestSucceeds)
{
    std::vector<MetaFileInfoDumpCxt>* fileInfoList = new std::vector<MetaFileInfoDumpCxt>;
    msg.completionData.fileInfoListPointer = fileInfoList;

    EXPECT_CALL(*container, GetInodeList);

    EXPECT_EQ(handler->HandleGetMetaFileInodeListReq(msg),
        EID(SUCCESS));

    delete fileInfoList;
    msg.completionData.fileInfoListPointer = nullptr;
}

TEST_F(MetaVolumeHandlerFixture, HandleGetMetaFileInodeListReq_testIfTheRequestIsFailedWhenThePointerIsNullptr)
{
    msg.completionData.fileInfoListPointer = nullptr;

    EXPECT_EQ(handler->HandleGetMetaFileInodeListReq(msg),
        EID(MFS_INVALID_PARAMETER));
}

TEST_F(MetaVolumeHandlerFixture, HandleGetFileInodeReq_testIfTheRequestSucceeds)
{
    MetaFileInode inode;

    EXPECT_CALL(*container, LookupMetaVolumeType(Matcher<std::string&>(_), _))
        .WillRepeatedly(Return(EID(SUCCESS)));
    EXPECT_CALL(*container, LookupFileDescByName).WillRepeatedly(Return(0));
    EXPECT_CALL(*container, GetInode).WillRepeatedly(ReturnRef(inode));
    EXPECT_CALL(*container, CopyInodeToInodeInfo).WillOnce(Return(true));

    EXPECT_EQ(handler->HandleGetFileInodeReq(msg), EID(SUCCESS));

    EXPECT_EQ(msg.completionData.inodeInfoPointer->data.field.dataLocation,
        MetaStorageType::SSD);
}

TEST_F(MetaVolumeHandlerFixture, HandleGetFileInodeReq_testIfTheRequestIsFailedWhenTheFileIsNotExistedInSpecificVolume)
{
    EXPECT_CALL(*container, LookupMetaVolumeType(Matcher<std::string&>(_), _))
        .WillOnce(Return(EID(MFS_INVALID_PARAMETER)));

    EXPECT_EQ(handler->HandleGetFileInodeReq(msg),
        EID(MFS_FILE_NOT_FOUND));
}

TEST_F(MetaVolumeHandlerFixture, HandleGetFileInodeReq_testIfTheRequestIsFailedWhenTheInodeIsNotExisted)
{
    EXPECT_CALL(*container, LookupMetaVolumeType(Matcher<std::string&>(_), _))
        .WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*container, LookupFileDescByName(Matcher<std::string&>(_)))
        .WillOnce(Return(0));
    EXPECT_CALL(*container, CopyInodeToInodeInfo).WillOnce(Return(false));

    EXPECT_EQ(handler->HandleGetFileInodeReq(msg), EID(MFS_INVALID_PARAMETER));
}

TEST_F(MetaVolumeHandlerFixture, HandleEstimateDataChunkSizeReq_testIfTheResultHasTheValidValue)
{
    EXPECT_EQ(handler->HandleEstimateDataChunkSizeReq(msg), EID(SUCCESS));
    EXPECT_EQ(msg.completionData.dataChunkSize, 4032);
}
} // namespace pos
