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

#include "meta_volume_handler.h"

#include <string>
#include <vector>

#include "src/metafs/mvm/volume/inode_manager.h"

namespace pos
{
MetaVolumeHandler::MetaVolumeHandler(MetaVolumeContainer* volContainer)
: volContainer(volContainer)
{
}

MetaVolumeHandler::~MetaVolumeHandler(void)
{
}

POS_EVENT_ID
MetaVolumeHandler::HandleOpenFileReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    if (POS_EVENT_ID::SUCCESS != HandleCheckFileExist(volType, reqMsg))
    {
        return POS_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    FileDescriptorType fd = volContainer->LookupFileDescByName(*reqMsg.fileName);
    POS_EVENT_ID rc = volContainer->AddFileInActiveList(volType, fd);

    if (POS_EVENT_ID::SUCCESS == rc)
    {
        reqMsg.completionData.openfd = fd;
        if (MetaFsCommonConst::INVALID_FD == fd)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_OPEN_FAILED,
                "Invalid FD retrieved due to unknown error. arrayId: {}, volumeType: {}",
                reqMsg.arrayId, (int)volType);

            return POS_EVENT_ID::MFS_FILE_OPEN_FAILED;
        }

        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "{} file has been open. fd: {}, arrayId: {}, volumeType: {}",
            *reqMsg.fileName, fd, reqMsg.arrayId, (int)volType);
    }
    else
    {
        MFS_TRACE_ERROR((int)rc,
            "{} file has been open twice. fd: {}, arrayId: {}, volumeType: {}",
            *reqMsg.fileName, fd, reqMsg.arrayId, (int)volType);
    }

    return rc;
}

POS_EVENT_ID
MetaVolumeHandler::HandleCheckFileExist(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    if (!volContainer->IsGivenFileCreated(*reqMsg.fileName))
    {
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "{} file was not found. fd: {}, arrayId: {}, volumeType: {}",
            *reqMsg.fileName, reqMsg.fd, reqMsg.arrayId, (int)volType);
        return POS_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleCloseFileReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    if (!volContainer->CheckFileInActive(volType, reqMsg.fd))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_NOT_OPENED,
            "The file is not open, fd: {}, arrayId: {}, volumeType: {}",
            reqMsg.fd, reqMsg.arrayId, (int)volType);
        return POS_EVENT_ID::MFS_FILE_NOT_OPENED;
    }

    volContainer->RemoveFileFromActiveList(volType, reqMsg.fd);

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "The file has been closed. fd: {}, arrayId: {}, volumeType: {}",
        reqMsg.fd, reqMsg.arrayId, (int)volType);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleCreateFileReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    if (!_CheckFileCreateReqSanity(volType, reqMsg))
    {
        return POS_EVENT_ID::MFS_FILE_CREATE_FAILED;
    }

    if (volContainer->CreateFile(volType, reqMsg))
    {
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "{} file has been created. byteSize: {}, arrayId: {}, volumeType: {}",
            *reqMsg.fileName, reqMsg.fileByteSize, reqMsg.arrayId, (int)volType);

        return POS_EVENT_ID::SUCCESS;
    }
    else
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_CREATE_FAILED,
            "Cannot create file inode due to I/O fail : \'{}\', reqType: {}, fd: {}, volumeType: {}",
            *reqMsg.fileName, reqMsg.reqType, reqMsg.fd, (int)volType);

        return POS_EVENT_ID::MFS_FILE_CREATE_FAILED;
    }
}

POS_EVENT_ID
MetaVolumeHandler::HandleDeleteFileReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    if (POS_EVENT_ID::SUCCESS != HandleCheckFileExist(volType, reqMsg))
    {
        return POS_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    // delete fd in fileMgr
    if (!volContainer->TrimData(volType, reqMsg))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_TRIM_FAILED,
            "Trim operation has been failed.");

        return POS_EVENT_ID::MFS_FILE_TRIM_FAILED;
    }

    // delete fd in inodeMgr
    if (volContainer->DeleteFile(volType, reqMsg))
    {
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "{} file has been deleted. fd: {}, arrayId: {}, volumeType: {}",
            *reqMsg.fileName, reqMsg.fd, reqMsg.arrayId, (int)volType);

        return POS_EVENT_ID::SUCCESS;
    }
    else
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_DELETE_FAILED,
            "Cannot delete file inode due to I/O fail : \'{}\', reqType: {}, fd: {}, volumeType: {}",
            *reqMsg.fileName, reqMsg.reqType, reqMsg.fd, (int)volType);

        return POS_EVENT_ID::MFS_FILE_DELETE_FAILED;
    }
}

POS_EVENT_ID
MetaVolumeHandler::HandleCheckFileAccessibleReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.fileAccessible = volContainer->CheckFileInActive(volType, reqMsg.fd);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetDataChunkSizeReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.dataChunkSize = volContainer->GetDataChunkSize(volType, reqMsg.fd);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetFileSizeReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.fileSize = volContainer->GetFileSize(volType, reqMsg.fd);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetTargetMediaTypeReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.targetMediaType = MetaFileUtil::ConvertToMediaType(volType);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetFileBaseLpnReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.fileBaseLpn = volContainer->GetFileBaseLpn(volType, reqMsg.fd);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetFreeFileRegionSizeReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.fileSize = volContainer->GetAvailableSpace(volType);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleCreateArrayReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleDeleteArrayReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetMaxMetaLpnReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.maxLpn = volContainer->GetMaxLpn(volType);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetMetaFileInodeListReq(MetaFsFileControlRequest& reqMsg)
{
    std::vector<MetaFileInfoDumpCxt>* fileInfoList = reqMsg.completionData.fileInfoListPointer;

    if (nullptr == fileInfoList)
    {
        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    // find all valid files and get the inode entry
    volContainer->GetInodeList(fileInfoList, reqMsg.volType);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetFileInodeReq(MetaFsFileControlRequest& reqMsg)
{
    MetaVolumeType volumeType = reqMsg.volType;
    POS_EVENT_ID rc = volContainer->LookupMetaVolumeType(*reqMsg.fileName,
        volumeType);
    if (rc != POS_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_NOT_FOUND,
            "Cannot find \'{}\' file", *reqMsg.fileName);

        return POS_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    MetaFileInodeInfo* inodeInfo = new MetaFileInodeInfo();
    FileDescriptorType fd = volContainer->LookupFileDescByName(*reqMsg.fileName);

    if (volContainer->CopyInodeToInodeInfo(fd, volumeType, inodeInfo))
    {
        reqMsg.completionData.inodeInfoPointer = inodeInfo;
    }
    else
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "The inodeInfo is not valid.");

        rc = POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    return rc;
}

POS_EVENT_ID
MetaVolumeHandler::HandleEstimateDataChunkSizeReq(MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.dataChunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;

    return POS_EVENT_ID::SUCCESS;
}

bool
MetaVolumeHandler::_CheckFileCreateReqSanity(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    if (volContainer->IsGivenFileCreated(*reqMsg.fileName))
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "{} file is already existed. arrayId: {}",
            *reqMsg.fileName, reqMsg.arrayId);

        return false;
    }

    const FileSizeType availableSpaceInVolume = volContainer->GetAvailableSpace(volType);
    if (availableSpaceInVolume < reqMsg.fileByteSize)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_NOT_ENOUGH_SPACE,
            "The volume has not enough space to create file. request byteSize: {}, availableSpaceInVolume: {}",
            reqMsg.fileByteSize, availableSpaceInVolume);

        return false;
    }

    return true;
}
} // namespace pos
