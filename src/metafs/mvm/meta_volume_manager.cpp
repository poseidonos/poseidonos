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

#include "meta_volume_manager.h"
#include <string>
#include <vector>

namespace pos
{
MetaVolumeManager metaVolMgr;
MetaVolumeManager& mvmTopMgr = metaVolMgr;

MetaVolumeManager::MetaVolumeManager(void)
: volumeSpcfReqHandler{},
  globalRequestHandler{}
{
    _InitRequestHandler();
}

MetaVolumeManager::~MetaVolumeManager(void)
{
}

void
MetaVolumeManager::_InitRequestHandler(void)
{
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::FileOpen, &MetaVolumeHandler::HandleOpenFileReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::FileClose, &MetaVolumeHandler::HandleCloseFileReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::FileCreate, &MetaVolumeHandler::HandleCreateFileReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::FileDelete, &MetaVolumeHandler::HandleDeleteFileReq);

    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::CheckFileExist, &MetaVolumeHandler::HandleCheckFileExist);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::GetDataChunkSize, &MetaVolumeHandler::HandleGetDataChunkSizeReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::CheckFileAccessible, &MetaVolumeHandler::HandleCheckFileAccessibleReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::GetFileSize, &MetaVolumeHandler::HandleGetFileSizeReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::GetTargetMediaType, &MetaVolumeHandler::HandleGetTargetMediaTypeReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::GetFileBaseLpn, &MetaVolumeHandler::HandleGetFileBaseLpnReq);
    _RegisterVolumeSpcfReqHandler(MetaFsFileControlType::GetTheBiggestExtentSize, &MetaVolumeHandler::HandleGetFreeFileRegionSizeReq);

    _RegisterGlobalMetaReqHandler(MetaFsFileControlType::EstimateDataChunkSize, &MetaVolumeHandler::HandleEstimateDataChunkSizeReq);
    // wbt
    _RegisterGlobalMetaReqHandler(MetaFsFileControlType::GetMetaFileInfoList, &MetaVolumeHandler::HandleGetMetaFileInodeListReq);
    _RegisterGlobalMetaReqHandler(MetaFsFileControlType::GetMaxFileSizeLimit, &MetaVolumeHandler::HandleGetMaxFileSizeLimitReq);
    _RegisterGlobalMetaReqHandler(MetaFsFileControlType::GetFileInode, &MetaVolumeHandler::HandleGetFileInodeReq);
}

int
MetaVolumeManager::_GetRequestHandlerIndex(MetaFsFileControlType reqType)
{
    int handlerIdx;
    if (reqType < MetaFsFileControlType::OnVolumeSpecificReq_Max)
    {
        handlerIdx = (int)reqType - (int)MetaFsFileControlType::OnVolumeSpcfReq_Base;
        assert(handlerIdx >= 0 && handlerIdx < (int)MetaFsFileControlType::OnVolumeSpcfReq_Count);
    }
    else
    {
        handlerIdx = (int)reqType - (int)MetaFsFileControlType::NonVolumeSpcfReq_Base;
        assert(handlerIdx >= 0 && handlerIdx < (int)MetaFsFileControlType::NonVolumeSpcfReq_Count);
    }

    return handlerIdx;
}

void
MetaVolumeManager::_RegisterVolumeSpcfReqHandler(MetaFsFileControlType reqType, MetaVolSpcfReqHandler handler)
{
    int handlerIdx = _GetRequestHandlerIndex(reqType);
    volumeSpcfReqHandler[handlerIdx] = handler;
}

void
MetaVolumeManager::_RegisterGlobalMetaReqHandler(MetaFsFileControlType reqType, GlobalMetaReqHandler handler)
{
    int handlerIdx = _GetRequestHandlerIndex(reqType);
    globalRequestHandler[handlerIdx] = handler;
}

MetaVolumeManager&
MetaVolumeManager::GetInstance(void)
{
    return metaVolMgr;
}

const char*
MetaVolumeManager::GetModuleName(void)
{
    return "MVM topMgr";
}

POS_EVENT_ID
MetaVolumeManager::CheckReqSanity(MetaFsFileControlRequest& reqMsg)
{
    POS_EVENT_ID sc = _CheckSanityBasic(reqMsg);
    if (sc != POS_EVENT_ID::SUCCESS)
    {
        return sc;
    }

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MetaVolumeManager::_CheckSanityBasic(MetaFsFileControlRequest& reqMsg)
{
    if (false == reqMsg.IsValid())
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "Given request is incorrect. Please check parameters.");
        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    return POS_EVENT_ID::SUCCESS;
}

bool
MetaVolumeManager::_IsSiblingModuleReady(void)
{
    if (!metaStorage->IsReady())
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MODULE_NOT_READY,
            "Meta storage subsystem is not ready");

        return false;
    }

    return true;
}

void
MetaVolumeManager::Init(MetaVolumeType volumeType, std::string arrayName, MetaLpnType maxVolPageNum)
{
    if (true == volContext.IsGivenVolumeExist(volumeType, arrayName))
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_MODULE_ALREADY_READY,
            "You attempted to init. volumeMgr duplicately for the given volume type={}",
            (uint32_t)volumeType);

        return;
    }

    volContext.InitContext(volumeType, arrayName, maxVolPageNum);
    volHandler.InitHandler(&volContext);

    SetModuleInit();
}

bool
MetaVolumeManager::Bringup(void)
{
    SetModuleReady();

    return true;
}

bool
MetaVolumeManager::CreateVolume(MetaVolumeType volumeType, std::string arrayName)
{
    return volContext.CreateVolume(volumeType, arrayName);
}

bool
MetaVolumeManager::Open(bool isNPOR, std::string arrayName)
{
    return volContext.Open(isNPOR, arrayName);
}

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
bool
MetaVolumeManager::Compaction(bool isNPOR, std::string arrayName)
{
    return volContext.Compaction(isNPOR, arrayName);
}
#endif

bool
MetaVolumeManager::Close(bool& resetCxt, std::string arrayName)
{
    return volContext.Close(resetCxt /* output */, arrayName);
}

std::pair<MetaVolumeType, POS_EVENT_ID>
MetaVolumeManager::_LookupTargetMetaVolume(MetaFsFileControlRequest& reqMsg)
{
    pair<MetaVolumeType, POS_EVENT_ID> rc = make_pair(MetaVolumeType::Max, POS_EVENT_ID::SUCCESS);

    // based on reqMsg, select metaVolMgr to assign proper instance to handle the request
    switch (reqMsg.reqType)
    {
        case MetaFsFileControlType::FileCreate:
            if (0 == reqMsg.fileByteSize)
            {
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
                    "The file size cannot be zero, fileSize={}", reqMsg.fileByteSize);
                rc.second = POS_EVENT_ID::MFS_INVALID_PARAMETER;
                break;
            }
            // fall-through

        case MetaFsFileControlType::GetTheBiggestExtentSize:
            rc = volContext.DetermineVolumeToCreateFile(reqMsg.fileByteSize, reqMsg.fileProperty, *reqMsg.arrayName);

            if (rc.second != POS_EVENT_ID::SUCCESS)
            {
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_NOT_ENOUGH_SPACE,
                    "There is no NVRAM and SSD free space");
            }
            break;

        case MetaFsFileControlType::FileOpen:
            rc = volContext.LookupMetaVolumeType(*reqMsg.fileName, *reqMsg.arrayName);

            if (rc.second != POS_EVENT_ID::SUCCESS)
            {
                reqMsg.completionData.openfd = -1;

                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_NOT_OPEND,
                    "Cannot find \'{}\' file in array \'{}\'", *reqMsg.fileName, *reqMsg.arrayName);
            }
            break;

        case MetaFsFileControlType::FileDelete:
            rc = volContext.LookupMetaVolumeType(*reqMsg.fileName, *reqMsg.arrayName);

            if (rc.second != POS_EVENT_ID::SUCCESS)
            {
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_DELETE_FAILED,
                    "Cannot find \'{}\' file", *reqMsg.fileName);
            }
            break;

        case MetaFsFileControlType::CheckFileExist:
            rc = volContext.LookupMetaVolumeType(*reqMsg.fileName, *reqMsg.arrayName);
            break;

        default:
            rc = volContext.LookupMetaVolumeType(reqMsg.fd, *reqMsg.arrayName);

            if (rc.second != POS_EVENT_ID::SUCCESS)
            {
                MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
                    "Cannot find the file, fd={}, request={}",
                    reqMsg.fd, reqMsg.reqType);
            }
            break;
    }

    return rc;
}

bool
MetaVolumeManager::_IsValidVolumeType(MetaVolumeType volType)
{
    return volType < MetaVolumeType::Max;
}

bool
MetaVolumeManager::_IsVolumeSpecificRequest(MetaFsFileControlType reqType)
{
    if (reqType < MetaFsFileControlType::OnVolumeSpecificReq_Max)
    {
        return true;
    }

    return false;
}

POS_EVENT_ID
MetaVolumeManager::ProcessNewReq(MetaFsFileControlRequest& reqMsg)
{
    POS_EVENT_ID rc = CheckReqSanity(reqMsg);
    if (rc != POS_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "Failed sanity check. request type={}, rc={}",
            (int)reqMsg.reqType, (int)rc);

        return rc;
    }

    if (_IsVolumeSpecificRequest(reqMsg.reqType))
    {
        rc = _HandleVolumeSpcfRequest(reqMsg);
    }
    else
    {
        rc = _HandleGlobalMetaRequest(reqMsg);
    }

    return rc;
}

POS_EVENT_ID
MetaVolumeManager::_HandleVolumeSpcfRequest(MetaFsFileControlRequest& reqMsg)
{
    std::pair<MetaVolumeType, POS_EVENT_ID> rc = _LookupTargetMetaVolume(reqMsg);

    if (rc.second != POS_EVENT_ID::SUCCESS)
    {
        return rc.second;
    }

    if (!_IsValidVolumeType(rc.first))
    {
        return POS_EVENT_ID::MFS_FILE_NOT_FOUND;
    }
    else
    {
        return _ExecuteVolumeRequest(rc.first, reqMsg);
    }
}

POS_EVENT_ID
MetaVolumeManager::_HandleGlobalMetaRequest(MetaFsFileControlRequest& reqMsg)
{
    return _ExecuteGlobalMetaRequest(reqMsg);
}

MetaVolSpcfReqHandler
MetaVolumeManager::_DispatchVolumeSpcfReqHandler(MetaFsFileControlType reqType)
{
    return volumeSpcfReqHandler[_GetRequestHandlerIndex(reqType)];
}

GlobalMetaReqHandler
MetaVolumeManager::_DispatchGlobalMetaReqHandler(MetaFsFileControlType reqType)
{
    return globalRequestHandler[_GetRequestHandlerIndex(reqType)];
}

POS_EVENT_ID
MetaVolumeManager::_ExecuteVolumeRequest(MetaVolumeType volType, MetaFsFileControlRequest& reqMsg)
{
    MetaVolSpcfReqHandler reqHandler = _DispatchVolumeSpcfReqHandler(reqMsg.reqType);

    return (volHandler.*reqHandler)(volType, reqMsg);
}

POS_EVENT_ID
MetaVolumeManager::_ExecuteGlobalMetaRequest(MetaFsFileControlRequest& reqMsg)
{
    GlobalMetaReqHandler reqHandler = _DispatchGlobalMetaReqHandler(reqMsg.reqType);

    return (volHandler.*reqHandler)(reqMsg);
}

MetaLpnType
MetaVolumeManager::GetMaxMetaLpn(MetaVolumeType mediaType, std::string arrayName)
{
    return volContext.GetMaxMetaLpn(mediaType, arrayName);
}

POS_EVENT_ID
MetaVolumeManager::CheckFileAccessible(FileDescriptorType fd, std::string arrayName)
{
    POS_EVENT_ID rc;

    MetaFsFileControlRequest req;
    req.reqType = MetaFsFileControlType::CheckFileAccessible;
    req.fd = fd;
    req.arrayName = &arrayName;
    rc = ProcessNewReq(req);

    return rc;
}

POS_EVENT_ID
MetaVolumeManager::GetFileSize(FileDescriptorType fd, std::string arrayName, FileSizeType& outFileByteSize)
{
    MetaFsFileControlRequest req;
    req.reqType = MetaFsFileControlType::GetFileSize;
    req.fd = fd;
    req.arrayName = &arrayName;
    POS_EVENT_ID ret = ProcessNewReq(req);
    if (ret == POS_EVENT_ID::SUCCESS)
    {
        outFileByteSize = req.completionData.fileSize;
    }
    else
    {
        outFileByteSize = 0;
    }

    return ret;
}

POS_EVENT_ID
MetaVolumeManager::GetDataChunkSize(FileDescriptorType fd, std::string arrayName, FileSizeType& outDataChunkSize)
{
    MetaFsFileControlRequest req;
    req.reqType = MetaFsFileControlType::GetDataChunkSize;
    req.fd = fd;
    req.arrayName = &arrayName;
    POS_EVENT_ID ret = ProcessNewReq(req);
    if (ret == POS_EVENT_ID::SUCCESS)
    {
        outDataChunkSize = req.completionData.dataChunkSize;
    }
    return ret;
}

POS_EVENT_ID
MetaVolumeManager::GetTargetMediaType(FileDescriptorType fd, std::string arrayName, MetaStorageType& outTargetMediaType)
{
    MetaFsFileControlRequest req;
    req.reqType = MetaFsFileControlType::GetTargetMediaType;
    req.fd = fd;
    req.arrayName = &arrayName;
    POS_EVENT_ID ret = ProcessNewReq(req);
    if (ret == POS_EVENT_ID::SUCCESS)
    {
        outTargetMediaType = req.completionData.targetMediaType;
    }
    return ret;
}

POS_EVENT_ID
MetaVolumeManager::GetFileBaseLpn(FileDescriptorType fd, std::string arrayName, MetaLpnType& outFileBaseLpn)
{
    MetaFsFileControlRequest req;
    req.reqType = MetaFsFileControlType::GetFileBaseLpn;
    req.fd = fd;
    req.arrayName = &arrayName;
    POS_EVENT_ID ret = ProcessNewReq(req);
    if (ret == POS_EVENT_ID::SUCCESS)
    {
        outFileBaseLpn = req.completionData.fileBaseLpn;
    }
    return ret;
}
} // namespace pos
