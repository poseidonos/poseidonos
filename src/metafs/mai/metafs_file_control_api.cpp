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

#include "metafs_file_control_api.h"

#include "src/metafs/log/metafs_log.h"
#include "src/metafs/mai/meta_file_context_handler.h"

namespace pos
{
/* for test */
MetaFsFileControlApi::MetaFsFileControlApi(void)
: MetaFsFileControlApi(INT32_MAX, false, nullptr, nullptr, nullptr,
    nullptr, nullptr)
{
}

MetaFsFileControlApi::MetaFsFileControlApi(const int arrayId, const bool isNormal,
    MetaStorageSubsystem* storage, MetaFsManagementApi* mgmt, MetaVolumeManager* volMgr,
    std::unique_ptr<MetaFileContextHandler> handler, TelemetryPublisher* tp)
: arrayId(arrayId),
  isNormal(isNormal),
  storage(storage),
  mgmt(mgmt),
  volMgr(volMgr),
  tp(tp),
  fileContext(std::move(handler))
{
}

MetaFsFileControlApi::~MetaFsFileControlApi(void)
{
    delete volMgr;
}

POS_EVENT_ID
MetaFsFileControlApi::Create(std::string& fileName, uint64_t fileByteSize,
    MetaFilePropertySet prop, MetaVolumeType volumeType)
{
    if (!isNormal)
        return EID(MFS_MODULE_NOT_READY);

    POS_EVENT_ID rc = EID(SUCCESS);
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::FileCreate;
    reqMsg.fileName = &fileName;
    reqMsg.arrayId = arrayId;
    reqMsg.fileByteSize = fileByteSize;
    reqMsg.fileProperty = prop;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // validity check & MetaVolumeManager::HandleCreateFileReq()

    return rc;
}

void
MetaFsFileControlApi::Initialize(const uint64_t signature)
{
    assert(fileContext);
    fileContext->Initialize(signature);
}

POS_EVENT_ID
MetaFsFileControlApi::Delete(std::string& fileName, MetaVolumeType volumeType)
{
    if (!isNormal)
        return EID(MFS_MODULE_NOT_READY);

    POS_EVENT_ID rc = EID(SUCCESS);
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::FileDelete;
    reqMsg.fileName = &fileName;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg);

    return rc;
}

POS_EVENT_ID
MetaFsFileControlApi::Open(std::string& fileName, int& fd, MetaVolumeType volumeType)
{
    if (!isNormal)
        return EID(MFS_MODULE_NOT_READY);

    POS_EVENT_ID rc = EID(SUCCESS);
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::FileOpen;
    reqMsg.fileName = &fileName;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // validity check & MetaVolumeManager::HandleOpenFileReq()

    fd = reqMsg.completionData.openfd;

    if (EID(SUCCESS) == rc)
        fileContext->AddFileContext(fileName, fd, volumeType);

    return rc;
}

POS_EVENT_ID
MetaFsFileControlApi::Close(uint32_t fd, MetaVolumeType volumeType)
{
    if (!isNormal)
        return EID(MFS_MODULE_NOT_READY);

    POS_EVENT_ID rc = EID(SUCCESS);
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::FileClose;
    reqMsg.fd = fd;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // validity check &  MetaVolumeManager::HandleCloseFileReq()

    fileContext->RemoveFileContext(fd, reqMsg.volType);

    return rc;
}

POS_EVENT_ID
MetaFsFileControlApi::CheckFileExist(std::string& fileName, MetaVolumeType volumeType)
{
    if (!isNormal)
        return EID(MFS_MODULE_NOT_READY);

    POS_EVENT_ID rc = EID(SUCCESS);
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::CheckFileExist;
    reqMsg.fileName = &fileName;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg);

    return rc;
}

// return file size of corresponding file mapped to 'fd'.
// in fail case, return 0
size_t
MetaFsFileControlApi::GetFileSize(int fd, MetaVolumeType volumeType)
{
    if (!isNormal)
        return 0;

    POS_EVENT_ID rc = EID(SUCCESS);
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetFileSize;
    reqMsg.fd = fd;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::HandleGetFileSizeReq()

    if (EID(SUCCESS) == rc)
    {
        return reqMsg.completionData.fileSize;
    }
    else
    {
        return 0;
    }
}

// return data chunk size of corresponding file mapped to 'fd'.
// in fail case, return 0
size_t
MetaFsFileControlApi::GetAlignedFileIOSize(int fd, MetaVolumeType volumeType)
{
    if (!isNormal)
        return 0;

    POS_EVENT_ID rc = EID(SUCCESS);
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetDataChunkSize;
    reqMsg.fd = fd;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::HandleGetDataChunkSizeReq()

    if (EID(SUCCESS) == rc)
    {
        return reqMsg.completionData.dataChunkSize;
    }
    else
    {
        return 0;
    }
}

// return data chunk size according to the data integrity level
size_t
MetaFsFileControlApi::EstimateAlignedFileIOSize(MetaFilePropertySet& prop,
    MetaVolumeType volumeType)
{
    POS_EVENT_ID rc = EID(SUCCESS);
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::EstimateDataChunkSize;
    reqMsg.fileProperty = prop;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::HandleEstimateDataChunkSizeReq()

    if (EID(SUCCESS) == rc)
    {
        return reqMsg.completionData.dataChunkSize;
    }
    else
    {
        return 0;
    }
}

size_t
MetaFsFileControlApi::GetAvailableSpace(MetaFilePropertySet& prop,
    MetaVolumeType volumeType)
{
    POS_EVENT_ID rc = EID(SUCCESS);
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetAvailableSpace;
    reqMsg.fileByteSize = 0;
    reqMsg.fileProperty = prop;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::HandleGetFreeFileRegionSizeReq()

    if (EID(SUCCESS) == rc)
    {
        return reqMsg.completionData.fileSize;
    }
    else
    {
        return 0;
    }
}

size_t
MetaFsFileControlApi::GetMaxMetaLpn(MetaVolumeType type)
{
    POS_EVENT_ID rc = EID(SUCCESS);
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetMaxMetaLpn;
    reqMsg.volType = type;
    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::HandleGetMaxMetaLpnReq()

    if (EID(SUCCESS) == rc)
    {
        return reqMsg.completionData.maxLpn;
    }
    else
    {
        return 0;
    }
}

void
MetaFsFileControlApi::SetStatus(bool isNormal)
{
    this->isNormal = isNormal;
}

MetaFileContext*
MetaFsFileControlApi::GetFileInfo(FileDescriptorType fd, MetaVolumeType type)
{
    return fileContext->GetFileContext(fd, type);
}

std::vector<MetaFileInfoDumpCxt>
MetaFsFileControlApi::Wbt_GetMetaFileList(MetaVolumeType type)
{
    POS_EVENT_ID rc;
    MetaFsFileControlRequest reqMsg;
    std::vector<MetaFileInfoDumpCxt> result;

    result.clear();

    reqMsg.reqType = MetaFsFileControlType::GetMetaFileInfoList;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = type;
    reqMsg.completionData.fileInfoListPointer = &result;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::_HandleGetMetaFileInodeListReq()

    if (EID(SUCCESS) != rc)
    {
        MFS_TRACE_DEBUG((int)rc,
            "Request failed, type={}, arrayId={}", type, arrayId);
    }

    return result;
}

MetaFileInodeInfo*
MetaFsFileControlApi::Wbt_GetMetaFileInode(std::string& fileName, MetaVolumeType type)
{
    POS_EVENT_ID rc;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetFileInode;
    reqMsg.fileName = &fileName;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = type;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::_HandleGetFileInodeReq()

    if (EID(SUCCESS) == rc)
    {
        return reqMsg.completionData.inodeInfoPointer;
    }

    return nullptr;
}

void
MetaFsFileControlApi::InitVolume(MetaVolumeType volType, int arrayId, MetaLpnType maxVolPageNum)
{
    volMgr->InitVolume(volType, arrayId, maxVolPageNum);
}

bool
MetaFsFileControlApi::CreateVolume(MetaVolumeType volType)
{
    return volMgr->CreateVolume(volType);
}

bool
MetaFsFileControlApi::OpenVolume(bool isNPOR)
{
    return volMgr->OpenVolume(isNPOR);
}

bool
MetaFsFileControlApi::CloseVolume(bool& isNPOR)
{
    return volMgr->CloseVolume(isNPOR);
}

} // namespace pos
