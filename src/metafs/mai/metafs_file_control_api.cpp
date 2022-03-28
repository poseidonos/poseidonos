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

namespace pos
{
MetaFsFileControlApi::MetaFsFileControlApi(void)
{
}

MetaFsFileControlApi::MetaFsFileControlApi(const int arrayId, MetaStorageSubsystem* storage,
    MetaFsManagementApi* mgmt, MetaVolumeManager* volMgr)
{
    this->arrayId = arrayId;

    bitmap = new BitMap(MetaFsConfig::MAX_VOLUME_CNT);
    bitmap->ResetBitmap();

    nameMapByfd.clear();
    idxMapByName.clear();

    this->storage = storage;
    this->mgmt = mgmt;
    this->volMgr = (nullptr == volMgr) ? new MetaVolumeManager(arrayId, storage) : volMgr;
}

MetaFsFileControlApi::~MetaFsFileControlApi(void)
{
    if (nullptr != bitmap)
        delete bitmap;

    nameMapByfd.clear();
    idxMapByName.clear();

    delete volMgr;
}

POS_EVENT_ID
MetaFsFileControlApi::Create(std::string& fileName, uint64_t fileByteSize,
    MetaFilePropertySet prop, MetaVolumeType volumeType)
{
    if (!isNormal)
        return POS_EVENT_ID::MFS_MODULE_NOT_READY;

    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
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

POS_EVENT_ID
MetaFsFileControlApi::Delete(std::string& fileName, MetaVolumeType volumeType)
{
    if (!isNormal)
        return POS_EVENT_ID::MFS_MODULE_NOT_READY;

    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
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
        return POS_EVENT_ID::MFS_MODULE_NOT_READY;

    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::FileOpen;
    reqMsg.fileName = &fileName;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // validity check & MetaVolumeManager::HandleOpenFileReq()

    fd = reqMsg.completionData.openfd;

    if (POS_EVENT_ID::SUCCESS == rc)
        _AddFileContext(fileName, fd, reqMsg.volType);

    return rc;
}

POS_EVENT_ID
MetaFsFileControlApi::Close(uint32_t fd, MetaVolumeType volumeType)
{
    if (!isNormal)
        return POS_EVENT_ID::MFS_MODULE_NOT_READY;

    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::FileClose;
    reqMsg.fd = fd;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // validity check &  MetaVolumeManager::HandleCloseFileReq()

    _RemoveFileContext(fd, reqMsg.volType);

    return rc;
}

POS_EVENT_ID
MetaFsFileControlApi::CheckFileExist(std::string& fileName, MetaVolumeType volumeType)
{
    if (!isNormal)
        return POS_EVENT_ID::MFS_MODULE_NOT_READY;

    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
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

    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetFileSize;
    reqMsg.fd = fd;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::HandleGetFileSizeReq()

    if (POS_EVENT_ID::SUCCESS == rc)
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

    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetDataChunkSize;
    reqMsg.fd = fd;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::HandleGetDataChunkSizeReq()

    if (POS_EVENT_ID::SUCCESS == rc)
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
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::EstimateDataChunkSize;
    reqMsg.fileProperty = prop;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::HandleEstimateDataChunkSizeReq()

    if (POS_EVENT_ID::SUCCESS == rc)
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
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetAvailableSpace;
    reqMsg.fileByteSize = 0;
    reqMsg.fileProperty = prop;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = volumeType;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::HandleGetFreeFileRegionSizeReq()

    if (POS_EVENT_ID::SUCCESS == rc)
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
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetMaxMetaLpn;
    reqMsg.volType = type;
    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::HandleGetMaxMetaLpnReq()

    if (POS_EVENT_ID::SUCCESS == rc)
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
    SPIN_LOCK_GUARD_IN_SCOPE(iLock);

    auto it = nameMapByfd.find(make_pair(type, fd));

    // the list already has fd's context
    if (it != nameMapByfd.end())
    {
        auto result = idxMapByName.find(make_pair(type, it->second));

        assert(result != idxMapByName.end());

        return &cxtList[result->second];
    }

    return nullptr;
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

    if (POS_EVENT_ID::SUCCESS != rc)
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

    if (POS_EVENT_ID::SUCCESS == rc)
    {
        return reqMsg.completionData.inodeInfoPointer;
    }

    return nullptr;
}

MetaFileInodeInfo*
MetaFsFileControlApi::_GetFileInode(std::string& fileName, MetaVolumeType type)
{
    MetaFsFileControlRequest reqMsg;
    POS_EVENT_ID rc;

    reqMsg.reqType = MetaFsFileControlType::GetFileInode;
    reqMsg.fileName = &fileName;
    reqMsg.arrayId = arrayId;
    reqMsg.volType = type;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::_HandleGetFileInodeReq()

    if (POS_EVENT_ID::SUCCESS == rc)
    {
        MetaFileInodeInfo* fileInodePointer = reqMsg.completionData.inodeInfoPointer;

        return fileInodePointer;
    }

    return nullptr;
}

void
MetaFsFileControlApi::_AddFileContext(std::string& fileName,
    FileDescriptorType fd, MetaVolumeType type)
{
    uint32_t index = 0;
    MetaFileInodeInfo* info = nullptr;

    {
        SPIN_LOCK_GUARD_IN_SCOPE(iLock);

        // find first
        if (nameMapByfd.find(make_pair(type, fd)) != nameMapByfd.end())
        {
            return;
        }

        // get the inode
        info = _GetFileInode(fileName, type);

        // get the position
        index = bitmap->FindFirstZero();
        if (index >= MetaFsConfig::MAX_VOLUME_CNT)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_NEED_MORE_CONTEXT_SLOT,
                "Metafile count={}", index);
            return;
        }

        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "FileContext is allocated index={}, arrayId={}", index, arrayId);

        bitmap->SetBit(index);
        nameMapByfd.insert({make_pair(type, fd), fileName});
        idxMapByName.insert({make_pair(type, fileName), index});
    }

    // update
    MetaFileContext* context = &cxtList[index];
    context->isActivated = info->data.field.inUse;
    context->storageType = info->data.field.dataLocation;
    context->sizeInByte = info->data.field.fileByteSize;
    context->fileBaseLpn = info->data.field.extentMap[0].GetStartLpn();
    context->chunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    context->extentsCount = info->data.field.extentCnt;
    context->extents = info->data.field.extentMap;
    context->signature = mgmt->GetEpochSignature();
    context->storage = storage;
    assert(context->extentsCount != 0);
}

void
MetaFsFileControlApi::_RemoveFileContext(FileDescriptorType fd,
    MetaVolumeType type)
{
    SPIN_LOCK_GUARD_IN_SCOPE(iLock);

    auto it1 = nameMapByfd.find(make_pair(type, fd));
    if (it1 != nameMapByfd.end())
    {
        std::string fileName = it1->second;

        auto it2 = idxMapByName.find(make_pair(type, fileName));
        uint32_t index = it2->second;

        nameMapByfd.erase(it1);
        idxMapByName.erase(it2);
        bitmap->ClearBit(index);

        cxtList[index].Reset();

        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "FileContext is deallocated index={}, arrayId={}", index, arrayId);
    }
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
