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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include <vector>
#include <string>
#include "meta_volume_handler.h"
#include "mf_inode_mgr.h"

namespace pos
{
MetaVolumeHandler::MetaVolumeHandler(void)
: volContext(nullptr)
{
}

MetaVolumeHandler::~MetaVolumeHandler(void)
{
}

void
MetaVolumeHandler::InitHandler(MetaVolumeContext* volCxt)
{
    volContext = volCxt;
}

POS_EVENT_ID
MetaVolumeHandler::HandleOpenFileReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;

    if (POS_EVENT_ID::SUCCESS != HandleCheckFileExist(volType, reqMsg))
    {
        return POS_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    FileDescriptorType fd = volContext->LookupFileDescByName(*reqMsg.fileName, *reqMsg.arrayName);
    rc = volContext->AddFileInActiveList(volType, fd, *reqMsg.arrayName);

    if (POS_EVENT_ID::SUCCESS == rc)
    {
        reqMsg.completionData.openfd = fd;
        if (MetaFsCommonConst::INVALID_FD == fd)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_OPEN_FAILED,
                "Invalid FD retrieved due to unknown error.");

            return POS_EVENT_ID::MFS_FILE_OPEN_FAILED;
        }

        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "{} file open successs. File Descriptor is {}",
            reqMsg.fileName->c_str(), fd);
    }

    return rc;
}

POS_EVENT_ID
MetaVolumeHandler::HandleCheckFileExist(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    if (!volContext->IsFileInodeExist(*reqMsg.fileName, *reqMsg.arrayName))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_NOT_FOUND,
            "Cannot find \'{}\' file, reqType={}, fd={}",
            *reqMsg.fileName, reqMsg.reqType, reqMsg.fd);

        return POS_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleCloseFileReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    if (!volContext->CheckFileInActive(volType, reqMsg.fd, *reqMsg.arrayName))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_NOT_OPEND,
            "The file is not opened, fd={}", reqMsg.fd);

        return POS_EVENT_ID::MFS_FILE_NOT_OPEND;
    }
    volContext->RemoveFileFromActiveList(volType, reqMsg.fd, *reqMsg.arrayName);

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Given file has been closed successfully. (fd={})", reqMsg.fd);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleCreateFileReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    if (!_CheckFileCreateReqSanity(volType, reqMsg))
    {
        return POS_EVENT_ID::MFS_FILE_CREATE_FAILED;
    }

    if (true == volContext->CreateFileInode(volType, reqMsg))
    {
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "{} file has been created. File size={}KB.",
            *reqMsg.fileName, reqMsg.fileByteSize / 1024);

        return POS_EVENT_ID::SUCCESS;
    }
    else
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_CREATE_FAILED,
            "Cannot create file inode due to I/O fail : \'{}\', reqType={}, fd={}",
            *reqMsg.fileName, reqMsg.reqType, reqMsg.fd);

        return POS_EVENT_ID::MFS_FILE_CREATE_FAILED;
    }
}

POS_EVENT_ID
MetaVolumeHandler::HandleDeleteFileReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    if (POS_EVENT_ID::SUCCESS != HandleCheckFileExist(volType, reqMsg))
    {
        return POS_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    // delete fd in fileMgr
    if (true != volContext->TrimData(volType, reqMsg))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_TRIM_FAILED,
            "MFS FILE trim operation has been failed!!");

        return POS_EVENT_ID::MFS_FILE_TRIM_FAILED;
    }

    // delete fd in inodeMgr
    if (true == volContext->DeleteFileInode(volType, reqMsg))
    {
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "{} file has been deleted.", *reqMsg.fileName);

        return POS_EVENT_ID::SUCCESS;
    }
    else
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_DELETE_FAILED,
            "Cannot delete file inode due to I/O fail : \'{}\', reqType={}, fd={}",
            *reqMsg.fileName, reqMsg.reqType, reqMsg.fd);

        return POS_EVENT_ID::MFS_FILE_DELETE_FAILED;
    }
}

POS_EVENT_ID
MetaVolumeHandler::HandleCheckFileAccessibleReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.fileAccessible = volContext->CheckFileInActive(volType, reqMsg.fd, *reqMsg.arrayName);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetDataChunkSizeReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.dataChunkSize = volContext->GetDataChunkSize(volType, reqMsg.fd, *reqMsg.arrayName);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetFileSizeReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.fileSize = volContext->GetFileSize(volType, reqMsg.fd, *reqMsg.arrayName);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetTargetMediaTypeReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.targetMediaType = MetaFileUtil::ConvertToMediaType(volType);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetFileBaseLpnReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.fileBaseLpn = volContext->GetFileBaseLpn(volType, reqMsg.fd, *reqMsg.arrayName);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetFreeFileRegionSizeReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.fileSize = volContext->GetTheBiggestExtentSize(volType, *reqMsg.arrayName);

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleCreateArrayReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleDeleteArrayReq(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetMetaFileInodeListReq(MetaFsFileControlRequest& reqMsg)
{
    std::vector<MetaFileInfoDumpCxt>* fileInfoList = new std::vector<MetaFileInfoDumpCxt>;
    reqMsg.completionData.fileInfoListPointer = nullptr;

    // find all valid files and get the inode entry
    for (auto volumeType : Enum<MetaVolumeType>())
    {
        if (!volContext->IsGivenVolumeExist(volumeType, *reqMsg.arrayName))
        {
            continue;
        }
        MetaVolume& volume = volContext->GetMetaVolume(volumeType, *reqMsg.arrayName);
        MetaFileInodeManager& inodeMgr = volume.GetInodeInstance();
        for (int entryIdx = 0; entryIdx < MetaFsConfig::MAX_META_FILE_NUM_SUPPORT; entryIdx++)
        {
            if (inodeMgr.IsFileInodeInUse(entryIdx))
            {
                MetaFileInfoDumpCxt fileInfoData;
                MetaFileInode& inode = inodeMgr.GetInodeEntry(entryIdx);

                fileInfoData.fd = inode.data.basic.field.fd;
                fileInfoData.ctime = inode.data.basic.field.ctime;
                fileInfoData.fileName = inode.data.basic.field.fileName.ToString();
                fileInfoData.size = inode.data.basic.field.fileByteSize;
                fileInfoData.lpnBase = inode.data.basic.field.pagemap.baseMetaLpn;
                fileInfoData.lpnCount = inode.data.basic.field.pagemap.pageCnt;
                fileInfoData.location = MetaFileUtil::ConvertToMediaTypeName(volumeType);

                fileInfoList->push_back(fileInfoData);
            }
        }
    }

    reqMsg.completionData.fileInfoListPointer = fileInfoList;

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetMaxFileSizeLimitReq(MetaFsFileControlRequest& reqMsg)
{
    if (volContext->GetGlobalMaxFileSizeLimit() == 0)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "Max file size limit is not set");

        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }
    reqMsg.completionData.maxFileSizeByteLimit = volContext->GetGlobalMaxFileSizeLimit() * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetFileInodeReq(MetaFsFileControlRequest& reqMsg)
{
    MetaVolumeType volumeType = volContext->LookupMetaVolumeType(*reqMsg.fileName, *reqMsg.arrayName).first;
    if (volumeType == MetaVolumeType::Invalid)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_NOT_FOUND,
            "Cannot find \'{}\' file", *reqMsg.fileName);

        return POS_EVENT_ID::MFS_FILE_NOT_FOUND;
    }
    MetaVolume& volume = volContext->GetMetaVolume(volumeType, *reqMsg.arrayName);

    FileDescriptorType fd = volContext->LookupFileDescByName(*reqMsg.fileName, *reqMsg.arrayName);

    MetaFileInodeInfo* inodeInfo = new MetaFileInodeInfo();
    MetaFileInodeManager& inodeMgr = volume.GetInodeInstance();
    MetaFileInode& inode = inodeMgr.GetFileInode(fd, *reqMsg.arrayName);
    inode.SetMetaFileInfo(MetaFileUtil::ConvertToMediaType(volumeType), *inodeInfo);

    reqMsg.completionData.inodeInfoPointer = inodeInfo;

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeHandler::HandleEstimateDataChunkSizeReq(MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.dataChunkSize = volContext->CalculateDataChunkSizeInPage(reqMsg.fileProperty);

    return POS_EVENT_ID::SUCCESS;
}

bool
MetaVolumeHandler::_CheckFileCreateReqSanity(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    if (true == volContext->IsFileInodeExist(*reqMsg.fileName, *reqMsg.arrayName))
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "The given file already exists. fileName={}", *reqMsg.fileName);

        return false;
    }

    FileSizeType availableSpaceInVolume = volContext->GetTheBiggestExtentSize(volType, *reqMsg.arrayName);
    if (availableSpaceInVolume < reqMsg.fileByteSize)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_NOT_ENOUGH_SPACE,
            "The volume has not enough space to create file!... fileSize={}, availableSpaceInVolume={}",
            reqMsg.fileByteSize, availableSpaceInVolume);

        return false;
    }

    return true;
}
} // namespace pos
