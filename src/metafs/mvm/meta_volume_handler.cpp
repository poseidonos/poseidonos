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
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
MetaVolumeHandler::MetaVolumeHandler(MetaVolumeContainer* volContainer, TelemetryPublisher* tp)
: volContainer(volContainer),
  tp(tp)
{
}

MetaVolumeHandler::~MetaVolumeHandler(void)
{
}

POS_EVENT_ID
MetaVolumeHandler::HandleOpenFileReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    POS_EVENT_ID rc = EID(SUCCESS);

    do
    {
        rc = HandleCheckFileExist(volType, reqMsg);
        if (EID(SUCCESS) != rc)
        {
            POS_TRACE_INFO((int)rc, "[MetaFile Control] The volume is not found. volumeType: {}", (int)volType);
            break;
        }

        FileDescriptorType fd = volContainer->LookupFileDescByName(*reqMsg.fileName);
        if (MetaFsCommonConst::INVALID_FD == fd)
        {
            rc = EID(MFS_FILE_NOT_FOUND);
            POS_TRACE_ERROR((int)rc,
                "[MetaFile Control] {} file is not found. arrayId: {}, volumeType: {}",
                *reqMsg.fileName, reqMsg.arrayId, (int)volType);
            break;
        }

        reqMsg.completionData.openfd = fd;
        rc = volContainer->AddFileInActiveList(volType, fd);
        if (EID(SUCCESS) == rc)
        {
            POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
                "[MetaFile Control] {} file has been open. fd: {}, arrayId: {}, volumeType: {}",
                *reqMsg.fileName, fd, reqMsg.arrayId, (int)volType);
        }
        else
        {
            POS_TRACE_ERROR((int)rc,
                "[MetaFile Control] {} file has been open twice. fd: {}, arrayId: {}, volumeType: {}",
                *reqMsg.fileName, fd, reqMsg.arrayId, (int)volType);
            break;
        }
    } while (0);

    _PublishMetricConditionally(TEL40014_METAFS_FILE_OPEN_REQUEST, POSMetricTypes::MT_COUNT,
        reqMsg.arrayId, volType, reqMsg.fileProperty.type, (EID(SUCCESS) == rc));

    return rc;
}

POS_EVENT_ID
MetaVolumeHandler::HandleCheckFileExist(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    if (!volContainer->IsGivenVolumeExist(volType))
    {
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
            "The volume is not found. volumeType: {}", (int)volType);
        return EID(MFS_META_VOLUME_NOT_FOUND);
    }

    if (!volContainer->IsGivenFileCreated(volType, *reqMsg.fileName))
    {
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
            "{} file was not found. fd: {}, arrayId: {}, volumeType: {}",
            *reqMsg.fileName, reqMsg.fd, reqMsg.arrayId, (int)volType);
        return EID(MFS_FILE_NOT_FOUND);
    }

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaVolumeHandler::HandleCloseFileReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    POS_EVENT_ID rc = EID(SUCCESS);

    if (!volContainer->CheckFileInActive(volType, reqMsg.fd))
    {
        rc = EID(MFS_FILE_NOT_OPENED);
        POS_TRACE_ERROR((int)rc,
            "[MetaFile Control] The file is not open, fd: {}, arrayId: {}, volumeType: {}",
            reqMsg.fd, reqMsg.arrayId, (int)volType);
    }
    else
    {
        volContainer->RemoveFileFromActiveList(volType, reqMsg.fd);

        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
            "[MetaFile Control] The file has been closed. fd: {}, arrayId: {}, volumeType: {}",
            reqMsg.fd, reqMsg.arrayId, (int)volType);
    }

    _PublishMetricConditionally(TEL40015_METAFS_FILE_CLOSE_REQUEST, POSMetricTypes::MT_COUNT,
        reqMsg.arrayId, volType, reqMsg.fileProperty.type, (EID(SUCCESS) == rc));

    return rc;
}

POS_EVENT_ID
MetaVolumeHandler::HandleCreateFileReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    POS_EVENT_ID rc = EID(SUCCESS);

    do
    {
        rc = _CheckFileCreateReqSanity(volType, reqMsg);
        if (EID(SUCCESS) != rc)
        {
            break;
        }

        if (volContainer->CreateFile(volType, reqMsg))
        {
            POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
                "[MetaFile Control] {} file has been created. byteSize: {}, arrayId: {}, volumeType: {}",
                *reqMsg.fileName, reqMsg.fileByteSize, reqMsg.arrayId, (int)volType);
        }
        else
        {
            rc = EID(MFS_FILE_CREATE_FAILED);
            POS_TRACE_ERROR((int)rc,
                "[MetaFile Control] Cannot create file inode due to I/O fail : \'{}\', reqType: {}, fd: {}, volumeType: {}",
                *reqMsg.fileName, reqMsg.reqType, reqMsg.fd, (int)volType);
            break;
        }
    } while (0);

    _PublishMetricConditionally(TEL40013_METAFS_FILE_CREATE_REQUEST, POSMetricTypes::MT_COUNT,
        reqMsg.arrayId, volType, reqMsg.fileProperty.type, (EID(SUCCESS) == rc));

    return rc;
}

POS_EVENT_ID
MetaVolumeHandler::HandleDeleteFileReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    POS_EVENT_ID rc = EID(SUCCESS);

    do
    {
        rc = HandleCheckFileExist(volType, reqMsg);
        if (EID(SUCCESS) != rc)
        {
            break;
        }

        // delete fd in fileMgr
        if (!volContainer->TrimData(volType, reqMsg))
        {
            rc = EID(MFS_FILE_TRIM_FAILED);
            POS_TRACE_ERROR((int)rc,
                "Trim operation for {} has been failed. arrayId:{}, volumeType: {}",
                *reqMsg.fileName, reqMsg.arrayId, (int)volType);
            break;
        }

        // delete fd in inodeMgr, reqMsg.fd is not valid
        if (volContainer->DeleteFile(volType, reqMsg))
        {
            POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
                "[MetaFile Control] {} file has been deleted. arrayId: {}, volumeType: {}",
                *reqMsg.fileName, reqMsg.arrayId, (int)volType);
        }
        else
        {
            rc = EID(MFS_FILE_DELETE_FAILED);
            POS_TRACE_ERROR((int)rc,
                "[MetaFile Control] Cannot delete file inode due to I/O fail : \'{}\', reqType: {}, volumeType: {}",
                *reqMsg.fileName, reqMsg.reqType, (int)volType);
            break;
        }
    } while (0);

    _PublishMetricConditionally(TEL40016_METAFS_FILE_DELETE_REQUEST, POSMetricTypes::MT_COUNT,
        reqMsg.arrayId, volType, reqMsg.fileProperty.type, (EID(SUCCESS) == rc));

    return rc;
}

POS_EVENT_ID
MetaVolumeHandler::HandleCheckFileAccessibleReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.fileAccessible = volContainer->CheckFileInActive(volType, reqMsg.fd);

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetDataChunkSizeReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.dataChunkSize = volContainer->GetDataChunkSize(volType, reqMsg.fd);

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetFileSizeReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.fileSize = volContainer->GetFileSize(volType, reqMsg.fd);

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetTargetMediaTypeReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.targetMediaType = MetaFileUtil::ConvertToMediaType(volType);

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetFileBaseLpnReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.fileBaseLpn = volContainer->GetFileBaseLpn(volType, reqMsg.fd);

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetFreeFileRegionSizeReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.fileSize = volContainer->GetAvailableSpace(volType);

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaVolumeHandler::HandleCreateArrayReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    return EID(SUCCESS);
}

POS_EVENT_ID
MetaVolumeHandler::HandleDeleteArrayReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    return EID(SUCCESS);
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetMaxMetaLpnReq(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.maxLpn = volContainer->GetMaxLpn(volType);

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetMetaFileInodeListReq(MetaFsFileControlRequest& reqMsg)
{
    std::vector<MetaFileInfoDumpCxt>* fileInfoList = reqMsg.completionData.fileInfoListPointer;

    if (nullptr == fileInfoList)
    {
        return EID(MFS_INVALID_PARAMETER);
    }

    // find all valid files and get the inode entry
    volContainer->GetInodeList(fileInfoList, reqMsg.volType);

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaVolumeHandler::HandleGetFileInodeReq(MetaFsFileControlRequest& reqMsg)
{
    MetaVolumeType volumeType = reqMsg.volType;
    POS_EVENT_ID rc = volContainer->LookupMetaVolumeType(*reqMsg.fileName, volumeType);
    if (rc != EID(SUCCESS))
    {
        POS_TRACE_ERROR(EID(MFS_FILE_NOT_FOUND),
            "Cannot find \'{}\' file", *reqMsg.fileName);

        return EID(MFS_FILE_NOT_FOUND);
    }

    MetaFileInodeInfo* inodeInfo = new MetaFileInodeInfo();
    FileDescriptorType fd = volContainer->LookupFileDescByName(*reqMsg.fileName);

    if (volContainer->CopyInodeToInodeInfo(fd, volumeType, inodeInfo))
    {
        reqMsg.completionData.inodeInfoPointer = inodeInfo;
    }
    else
    {
        POS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
            "The inodeInfo is not valid.");

        rc = EID(MFS_INVALID_PARAMETER);
        delete inodeInfo;
    }

    return rc;
}

POS_EVENT_ID
MetaVolumeHandler::HandleEstimateDataChunkSizeReq(MetaFsFileControlRequest& reqMsg)
{
    reqMsg.completionData.dataChunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;

    return EID(SUCCESS);
}

POS_EVENT_ID
MetaVolumeHandler::_CheckFileCreateReqSanity(const MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    POS_EVENT_ID rc = EID(SUCCESS);

    if (volContainer->IsGivenFileCreated(volType, *reqMsg.fileName))
    {
        rc = EID(MFS_FILE_NAME_EXISTED);
        POS_TRACE_INFO((int)rc, "{} file is already existed. arrayId: {}",
            *reqMsg.fileName, reqMsg.arrayId);

        return rc;
    }

    const FileSizeType availableSpaceInVolume = volContainer->GetAvailableSpace(volType);
    if (availableSpaceInVolume < reqMsg.fileByteSize)
    {
        rc = EID(MFS_META_VOLUME_NOT_ENOUGH_SPACE);
        POS_TRACE_INFO((int)rc,
            "The volume has not enough space to create file. request byteSize: {}, availableSpaceInVolume: {}",
            reqMsg.fileByteSize, availableSpaceInVolume);
        return rc;
    }

    return rc;
}

void
MetaVolumeHandler::_PublishMetricConditionally(const std::string& name,
    const POSMetricTypes metricType, const int arrayId,
    const MetaVolumeType volType, const MetaFileType fileType,
    const bool requestResult)
{
    if (tp)
    {
        POSMetric m(name, metricType);
        m.SetCountValue(1);
        m.AddLabel("array_id", std::to_string(arrayId));
        m.AddLabel("volume_type", std::to_string((int)volType));
        m.AddLabel("file_type", std::to_string((int)fileType));
        m.AddLabel("result", requestResult ? "success" : "failed");
        tp->PublishMetric(m);
    }
}
} // namespace pos
