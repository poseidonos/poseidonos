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

#include "meta_vol_mgr.h"

#include <vector>

MetaVolMgrClass metaVolMgr;
MetaFsMVMTopMgrClass& mvmTopMgr = metaVolMgr;

MetaVolMgrClass::MetaVolMgrClass(void)
: volumeSpcfReqHandler{},
  globalRequestHandler{}
{
    _InitReqHandler();
}

MetaVolMgrClass::~MetaVolMgrClass(void)
{
}

void
MetaVolMgrClass::_InitReqHandler(void)
{
    _RegisterVolumeSpcfReqHandler(MetaFsMoMReqType::FileOpen, &MetaVolMgrClass::_HandleOpenFileReq);
    _RegisterVolumeSpcfReqHandler(MetaFsMoMReqType::FileClose, &MetaVolMgrClass::_HandleCloseFileReq);
    _RegisterVolumeSpcfReqHandler(MetaFsMoMReqType::FileCreate, &MetaVolMgrClass::_HandleCreateFileReq);
    _RegisterVolumeSpcfReqHandler(MetaFsMoMReqType::FileDelete, &MetaVolMgrClass::_HandleDeleteFileReq);

    _RegisterVolumeSpcfReqHandler(MetaFsMoMReqType::CheckFileExist, &MetaVolMgrClass::_HandleCheckFileExist);
    _RegisterVolumeSpcfReqHandler(MetaFsMoMReqType::GetDataChunkSize, &MetaVolMgrClass::_HandleGetDataChunkSizeReq);
    _RegisterVolumeSpcfReqHandler(MetaFsMoMReqType::CheckFileAccessible, &MetaVolMgrClass::_HandleCheckFileAccessibleReq);
    _RegisterVolumeSpcfReqHandler(MetaFsMoMReqType::GetFileSize, &MetaVolMgrClass::_HandleGetFileSizeReq);
    _RegisterVolumeSpcfReqHandler(MetaFsMoMReqType::GetTargetMediaType, &MetaVolMgrClass::_HandleGetTargetMediaTypeReq);
    _RegisterVolumeSpcfReqHandler(MetaFsMoMReqType::GetFileBaseLpn, &MetaVolMgrClass::_HandleGetFileBaseLpnReq);
    _RegisterVolumeSpcfReqHandler(MetaFsMoMReqType::GetTheBiggestExtentSize, &MetaVolMgrClass::_HandleGetFreeFileRegionSizeReq);

    _RegisterGlobalMetaReqHandler(MetaFsMoMReqType::EstimateDataChunkSize, &MetaVolMgrClass::_HandleEstimateDataChunkSizeReq);
    // wbt
    _RegisterGlobalMetaReqHandler(MetaFsMoMReqType::GetMetaFileInfoList, &MetaVolMgrClass::_HandleGetMetaFileInodeListReq);
    _RegisterGlobalMetaReqHandler(MetaFsMoMReqType::GetMaxFileSizeLimit, &MetaVolMgrClass::_HandleGetMaxFileSizeLimitReq);
    _RegisterGlobalMetaReqHandler(MetaFsMoMReqType::GetFileInode, &MetaVolMgrClass::_HandleGetFileInodeReq);
}

int
MetaVolMgrClass::_GetReqHandlerIdx(MetaFsMoMReqType reqType)
{
    int handlerIdx;
    if (reqType < MetaFsMoMReqType::OnVolumeSpecificReq_Max)
    {
        handlerIdx = (int)reqType - (int)MetaFsMoMReqType::OnVolumeSpcfReq_Base;
        assert(handlerIdx >= 0 && handlerIdx < (int)MetaFsMoMReqType::OnVolumeSpcfReq_Count);
    }
    else
    {
        handlerIdx = (int)reqType - (int)MetaFsMoMReqType::NonVolumeSpcfReq_Base;
        assert(handlerIdx >= 0 && handlerIdx < (int)MetaFsMoMReqType::NonVolumeSpcfReq_Count);
    }

    return handlerIdx;
}

void
MetaVolMgrClass::_RegisterVolumeSpcfReqHandler(MetaFsMoMReqType reqType, MetaVolSpcfReqHandler handler)
{
    int handlerIdx = _GetReqHandlerIdx(reqType);
    volumeSpcfReqHandler[handlerIdx] = handler;
}

void
MetaVolMgrClass::_RegisterGlobalMetaReqHandler(MetaFsMoMReqType reqType, GlobalMetaReqHandler handler)
{
    int handlerIdx = _GetReqHandlerIdx(reqType);
    globalRequestHandler[handlerIdx] = handler;
}

MetaVolMgrClass&
MetaVolMgrClass::GetInstance(void)
{
    return metaVolMgr;
}

bool
MetaVolMgrClass::_IsSiblingModuleReady(void)
{
    if (!metaStorage->IsReady())
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_MODULE_NOT_READY,
            "Meta storage subsystem is not ready");

        return false;
    }

    return true;
}

void
MetaVolMgrClass::Init(MetaVolumeType volumeType, MetaLpnType maxVolPageNum)
{
    if (true == volContext.IsGivenVolumeExist(volumeType))
    {
        MFS_TRACE_WARN((int)IBOF_EVENT_ID::MFS_MODULE_ALREADY_READY,
            "You attempted to init. volumeMgr duplicately for the given volume type={}",
            (uint32_t)volumeType);

        return;
    }

    volContext.InitVolume(volumeType, maxVolPageNum);

    SetModuleInit();
}

bool
MetaVolMgrClass::Bringup(void)
{
    SetModuleReady();

    return true;
}

bool
MetaVolMgrClass::CreateVolume(MetaVolumeType volumeType)
{
    return volContext.CreateVolume(volumeType);
}

bool
MetaVolMgrClass::Open(bool isNPOR)
{
    return volContext.Open(isNPOR);
}

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
bool
MetaVolMgrClass::Compaction(bool isNPOR)
{
    return volContext.Compaction(isNPOR);
}
#endif

bool
MetaVolMgrClass::Close(bool& resetCxt)
{
    return volContext.Close(resetCxt /* output */);
}

std::pair<MetaVolumeType, IBOF_EVENT_ID>
MetaVolMgrClass::_LookupTargetMetaVolume(MetaFsMoMReqMsg& reqMsg)
{
    pair<MetaVolumeType, IBOF_EVENT_ID> rc = make_pair(MetaVolumeType::Max, IBOF_EVENT_ID::SUCCESS);

    // based on reqMsg, select metaVolMgr to assign proper instance to handle the request
    switch (reqMsg.reqType)
    {
        case MetaFsMoMReqType::FileCreate:
            if (0 == reqMsg.fileByteSize)
            {
                MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_INVALID_PARAMETER,
                    "The file size cannot be zero, fileSize={}", reqMsg.fileByteSize);
                rc.second = IBOF_EVENT_ID::MFS_INVALID_PARAMETER;
                break;
            }
            // fall-through

        case MetaFsMoMReqType::GetTheBiggestExtentSize:
            rc = volContext.DetermineVolumeToCreateFile(reqMsg.fileByteSize, reqMsg.fileProperty);

            if (rc.second != IBOF_EVENT_ID::SUCCESS)
            {
                MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_VOLUME_NOT_ENOUGH_SPACE,
                    "There is no NVRAM and SSD free space, {}", *reqMsg.fileName);
            }
            break;

        case MetaFsMoMReqType::FileOpen:
            rc = volContext.LookupMetaVolumeType(*reqMsg.fileName);

            if (rc.second != IBOF_EVENT_ID::SUCCESS)
            {
                reqMsg.completionData.openfd = -1;

                MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_FILE_NOT_OPEND,
                    "Cannot find \'{}\' file", *reqMsg.fileName);
            }
            break;

        case MetaFsMoMReqType::FileDelete:
            rc = volContext.LookupMetaVolumeType(*reqMsg.fileName);

            if (rc.second != IBOF_EVENT_ID::SUCCESS)
            {
                MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_FILE_DELETE_FAILED,
                    "Cannot find \'{}\' file", *reqMsg.fileName);
            }
            break;

        case MetaFsMoMReqType::CheckFileExist:
            rc = volContext.LookupMetaVolumeType(*reqMsg.fileName);
            break;

        default:
            rc = volContext.LookupMetaVolumeType(reqMsg.fd);

            if (rc.second != IBOF_EVENT_ID::SUCCESS)
            {
                MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_INVALID_PARAMETER,
                    "Cannot find the file, fd={}, request={}",
                    reqMsg.fd, reqMsg.reqType);
            }
            break;
    }

    return rc;
}

bool
MetaVolMgrClass::_IsValidVolumeType(MetaVolumeType volType)
{
    return volType < MetaVolumeType::Max;
}

bool
MetaVolMgrClass::_IsVolumeSpecificRequest(MetaFsMoMReqType reqType)
{
    if (reqType < MetaFsMoMReqType::OnVolumeSpecificReq_Max)
    {
        return true;
    }

    return false;
}

IBOF_EVENT_ID
MetaVolMgrClass::ProcessNewReq(MetaFsMoMReqMsg& reqMsg)
{
    IBOF_EVENT_ID rc = CheckReqSanity(reqMsg);
    if (rc != IBOF_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_INVALID_PARAMETER,
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

IBOF_EVENT_ID
MetaVolMgrClass::_HandleVolumeSpcfRequest(MetaFsMoMReqMsg& reqMsg)
{
    std::pair<MetaVolumeType, IBOF_EVENT_ID> rc = _LookupTargetMetaVolume(reqMsg);

    if (rc.second != IBOF_EVENT_ID::SUCCESS)
    {
        return rc.second;
    }

    if (!_IsValidVolumeType(rc.first))
    {
        return IBOF_EVENT_ID::MFS_FILE_NOT_FOUND;
    }
    else
    {
        return _ExecuteVolumeRequest(volContext.GetMetaVolume(rc.first), reqMsg);
    }
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleGlobalMetaRequest(MetaFsMoMReqMsg& reqMsg)
{
    return _ExecuteGlobalMetaRequest(reqMsg);
}

MetaVolSpcfReqHandler
MetaVolMgrClass::_DispatchVolumeSpcfReqHandler(MetaFsMoMReqType reqType)
{
    return volumeSpcfReqHandler[_GetReqHandlerIdx(reqType)];
}

GlobalMetaReqHandler
MetaVolMgrClass::_DispatchGlobalMetaReqHandler(MetaFsMoMReqType reqType)
{
    return globalRequestHandler[_GetReqHandlerIdx(reqType)];
}

IBOF_EVENT_ID
MetaVolMgrClass::_ExecuteVolumeRequest(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    MetaVolSpcfReqHandler reqHandler = _DispatchVolumeSpcfReqHandler(reqMsg.reqType);

    return (this->*reqHandler)(tgtMetaVol, reqMsg);
}

IBOF_EVENT_ID
MetaVolMgrClass::_ExecuteGlobalMetaRequest(MetaFsMoMReqMsg& reqMsg)
{
    GlobalMetaReqHandler reqHandler = _DispatchGlobalMetaReqHandler(reqMsg.reqType);

    return (this->*reqHandler)(reqMsg);
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleEstimateDataChunkSizeReq(MetaFsMoMReqMsg& reqMsg)
{
    reqMsg.completionData.dataChunkSize = volContext.CalculateDataChunkSizeInPage(reqMsg.fileProperty);

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleGetFreeFileRegionSizeReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    reqMsg.completionData.fileSize = volContext.GetTheBiggestExtentSize(tgtMetaVol);

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleCreateFileReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    if (!_CheckFileCreateReqSanity(tgtMetaVol, reqMsg))
    {
        return IBOF_EVENT_ID::MFS_FILE_CREATE_FAILED;
    }

    if (true == volContext.CreateFileInode(tgtMetaVol, reqMsg))
    {
        MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
            "{} file has been created. File size={}KB.",
            *reqMsg.fileName, reqMsg.fileByteSize / 1024);

        return IBOF_EVENT_ID::SUCCESS;
    }
    else
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_FILE_CREATE_FAILED,
            "Cannot create file inode due to I/O fail : \'{}\', reqType={}, fd={}",
            *reqMsg.fileName, reqMsg.reqType, reqMsg.fd);

        return IBOF_EVENT_ID::MFS_FILE_CREATE_FAILED;
    }
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleDeleteFileReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    if (IBOF_EVENT_ID::SUCCESS != _HandleCheckFileExist(tgtMetaVol, reqMsg))
    {
        return IBOF_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    // delete fd in fileMgr
    if (true != volContext.TrimData(tgtMetaVol, reqMsg))
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_FILE_TRIM_FAILED,
            "MFS FILE trim operation has been failed!!");

        return IBOF_EVENT_ID::MFS_FILE_TRIM_FAILED;
    }

    // delete fd in inodeMgr
    if (true == volContext.DeleteFileInode(tgtMetaVol, reqMsg))
    {
        MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
            "{} file has been deleted.", *reqMsg.fileName);

        return IBOF_EVENT_ID::SUCCESS;
    }
    else
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_FILE_DELETE_FAILED,
            "Cannot delete file inode due to I/O fail : \'{}\', reqType={}, fd={}",
            *reqMsg.fileName, reqMsg.reqType, reqMsg.fd);

        return IBOF_EVENT_ID::MFS_FILE_DELETE_FAILED;
    }
}

bool
MetaVolMgrClass::_CheckFileCreateReqSanity(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    if (true == volContext.IsFileInodeExist(*reqMsg.fileName))
    {
        MFS_TRACE_WARN((int)IBOF_EVENT_ID::MFS_INVALID_PARAMETER,
            "The given file already exists. fileName={}", *reqMsg.fileName);

        return false;
    }

    FileSizeType availableSpaceInVolume = volContext.GetTheBiggestExtentSize(tgtMetaVol);
    if (availableSpaceInVolume < reqMsg.fileByteSize)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_VOLUME_NOT_ENOUGH_SPACE,
            "The volume has not enough space to create file!... fileSize={}, availableSpaceInVolume={}",
            reqMsg.fileByteSize, availableSpaceInVolume);

        return false;
    }

    return true;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleOpenFileReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    IBOF_EVENT_ID rc = IBOF_EVENT_ID::SUCCESS;

    if (IBOF_EVENT_ID::SUCCESS != _HandleCheckFileExist(tgtMetaVol, reqMsg))
    {
        return IBOF_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    FileFDType fd = volContext.LookupFileDescByName(*reqMsg.fileName);
    rc = volContext.AddFileInActiveList(tgtMetaVol, fd);

    if (IBOF_EVENT_ID::SUCCESS == rc)
    {
        reqMsg.completionData.openfd = fd;
        if (MetaFsCommonConst::INVALID_FD == fd)
        {
            MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_FILE_OPEN_FAILED,
                "Invalid FD retrieved due to unknown error.");

            return IBOF_EVENT_ID::MFS_FILE_OPEN_FAILED;
        }

        MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
            "{} file open successs. File Descriptor is {}",
            reqMsg.fileName->c_str(), fd);
    }

    return rc;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleCheckFileExist(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    if (!volContext.IsFileInodeExist(*reqMsg.fileName))
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_FILE_NOT_FOUND,
            "Cannot find \'{}\' file, reqType={}, fd={}",
            *reqMsg.fileName, reqMsg.reqType, reqMsg.fd);

        return IBOF_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleCloseFileReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    if (!volContext.CheckFileInActive(tgtMetaVol, reqMsg.fd))
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_FILE_NOT_OPEND,
            "The file is not opened, fd={}", reqMsg.fd);

        return IBOF_EVENT_ID::MFS_FILE_NOT_OPEND;
    }
    volContext.RemoveFileFromActiveList(tgtMetaVol, reqMsg.fd);

    MFS_TRACE_INFO((int)IBOF_EVENT_ID::MFS_INFO_MESSAGE,
        "Given file has been closed successfully. (fd={})", reqMsg.fd);

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleCheckFileAccessibleReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    reqMsg.completionData.fileAccessible = volContext.CheckFileInActive(tgtMetaVol, reqMsg.fd);

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleGetDataChunkSizeReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    reqMsg.completionData.dataChunkSize = volContext.GetDataChunkSize(tgtMetaVol, reqMsg.fd);

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleGetFileSizeReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    reqMsg.completionData.fileSize = volContext.GetFileSize(tgtMetaVol, reqMsg.fd);

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleGetTargetMediaTypeReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    MetaVolumeType volumeType = tgtMetaVol.GetVolumeType();
    reqMsg.completionData.targetMediaType = MetaFsUtilLib::ConvertToMediaType(volumeType);

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleGetFileBaseLpnReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    reqMsg.completionData.fileBaseLpn = volContext.GetFileBaseLpn(tgtMetaVol, reqMsg.fd);

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleFileLockReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    if (true == volContext.IsLockAlreadyGranted(tgtMetaVol, reqMsg.fd))
    {
        return IBOF_EVENT_ID::MFS_FILE_ALREADY_LOCKED;
    }
    volContext.LockFile(tgtMetaVol, reqMsg.fd, reqMsg.lock);

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleFileUnlockReq(MetaVolumeClass& tgtMetaVol, MetaFsMoMReqMsg& reqMsg)
{
    volContext.UnlockFile(tgtMetaVol, reqMsg.fd);

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleGetMetaFileInodeListReq(MetaFsMoMReqMsg& reqMsg)
{
    std::vector<MetaFileInfoDumpCxt>* fileInfoList = new std::vector<MetaFileInfoDumpCxt>;
    reqMsg.completionData.fileInfoListPointer = nullptr;

    // find all valid files and get the inode entry
    for (auto volumeType : Enum<MetaVolumeType>())
    {
        if (!volContext.IsGivenVolumeExist(volumeType))
        {
            continue;
        }
        MetaVolumeClass& volume = volContext.GetMetaVolume(volumeType);
        for (int entryIdx = 0; entryIdx < MetaFsConfig::MAX_META_FILE_NUM_SUPPORT; entryIdx++)
        {
            if (volume.inodeMgr.IsFileInodeInUse(entryIdx))
            {
                MetaFileInfoDumpCxt fileInfoData;
                MetaFileInode& inode = volume.inodeMgr.GetInodeEntry(entryIdx);

                fileInfoData.fd = inode.data.basic.field.fd;
                fileInfoData.ctime = inode.data.basic.field.ctime;
                fileInfoData.fileName = inode.data.basic.field.fileName.ToString();
                fileInfoData.size = inode.data.basic.field.fileByteSize;
                fileInfoData.location = MetaFsUtilLib::ConvertToMediaTypeName(volumeType);

                fileInfoList->push_back(fileInfoData);
            }
        }
    }

    reqMsg.completionData.fileInfoListPointer = fileInfoList;

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleGetMaxFileSizeLimitReq(MetaFsMoMReqMsg& reqMsg)
{
    if (volContext.GetGlobalMaxFileSizeLimit() == 0)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_INVALID_PARAMETER,
            "Max file size limit is not set");

        return IBOF_EVENT_ID::MFS_INVALID_PARAMETER;
    }
    reqMsg.completionData.maxFileSizeByteLimit = volContext.GetGlobalMaxFileSizeLimit() * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;

    return IBOF_EVENT_ID::SUCCESS;
}

IBOF_EVENT_ID
MetaVolMgrClass::_HandleGetFileInodeReq(MetaFsMoMReqMsg& reqMsg)
{
    MetaVolumeType volumeType = volContext.LookupMetaVolumeType(*reqMsg.fileName).first;
    if (volumeType == MetaVolumeType::Invalid)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_FILE_NOT_FOUND,
            "Cannot find \'{}\' file", *reqMsg.fileName);

        return IBOF_EVENT_ID::MFS_FILE_NOT_FOUND;
    }
    MetaVolumeClass& volume = volContext.GetMetaVolume(volumeType);

    FileFDType fd = volContext.LookupFileDescByName(*reqMsg.fileName);

    MetaFileInodeInfo* inodeInfo = new MetaFileInodeInfo();
    MetaFileInode& inode = volume.inodeMgr.GetFileInode(fd);
    inode.SetMetaFileInfo(MetaFsUtilLib::ConvertToMediaType(volumeType), *inodeInfo);

    reqMsg.completionData.inodeInfoPointer = inodeInfo;

    return IBOF_EVENT_ID::SUCCESS;
}

MetaLpnType
MetaVolMgrClass::GetMaxMetaLpn(MetaVolumeType mediaType)
{
    return volContext.GetMaxMetaLpn(mediaType);
}
