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

#include "metafs_file_control_api.h"

namespace pos
{
MetaFsFileControlApi::MetaFsFileControlApi(std::string arrayName)
: arrayName(arrayName)
{
    bitmap = new BitMap(MetaFsConfig::MAX_VOLUME_CNT);
    bitmap->ResetBitmap();

    nameMapByfd.clear();
    idxMapByName.clear();

    volMgr = new MetaVolumeManager();
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
MetaFsFileControlApi::Create(std::string& fileName, uint64_t fileByteSize, MetaFilePropertySet prop)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::FileCreate;
    reqMsg.fileName = &fileName;
    reqMsg.arrayName = &arrayName;
    reqMsg.fileByteSize = fileByteSize;
    reqMsg.fileProperty = prop;

    rc = volMgr->HandleNewRequest(reqMsg); // validity check & MetaVolumeManager::HandleCreateFileReq()

    return rc;
}

POS_EVENT_ID
MetaFsFileControlApi::Delete(std::string& fileName)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::FileDelete;
    reqMsg.fileName = &fileName;
    reqMsg.arrayName = &arrayName;
    rc = volMgr->HandleNewRequest(reqMsg);

    return rc;
}

POS_EVENT_ID
MetaFsFileControlApi::Open(std::string& fileName, int& fd)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::FileOpen;
    reqMsg.fileName = &fileName;
    reqMsg.arrayName = &arrayName;
    rc = volMgr->HandleNewRequest(reqMsg); // validity check & MetaVolumeManager::HandleOpenFileReq()

    fd = reqMsg.completionData.openfd;

    _AddFileContext(fileName, fd);

    return rc;
}

POS_EVENT_ID
MetaFsFileControlApi::Close(uint32_t fd)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::FileClose;
    reqMsg.fd = fd;
    reqMsg.arrayName = &arrayName;
    rc = volMgr->HandleNewRequest(reqMsg); // validity check &  MetaVolumeManager::HandleCloseFileReq()

    _RemoveFileContext(fd);

    return rc;
}

POS_EVENT_ID
MetaFsFileControlApi::CheckFileExist(std::string& fileName)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::CheckFileExist;
    reqMsg.fileName = &fileName;
    reqMsg.arrayName = &arrayName;
    rc = volMgr->HandleNewRequest(reqMsg);

    return rc;
}

// return file size of corresponding file mapped to 'fd'.
// in fail case, return 0
size_t
MetaFsFileControlApi::GetFileSize(int fd)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetFileSize;
    reqMsg.fd = fd;
    reqMsg.arrayName = &arrayName;
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
MetaFsFileControlApi::GetAlignedFileIOSize(int fd)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetDataChunkSize;
    reqMsg.fd = fd;
    reqMsg.arrayName = &arrayName;
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
MetaFsFileControlApi::EstimateAlignedFileIOSize(MetaFilePropertySet& prop)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::EstimateDataChunkSize;
    reqMsg.fileProperty = prop;
    reqMsg.arrayName = &arrayName;
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
MetaFsFileControlApi::GetTheBiggestExtentSize(MetaFilePropertySet& prop)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    MetaFsFileControlRequest reqMsg;

    reqMsg.reqType = MetaFsFileControlType::GetTheBiggestExtentSize;
    reqMsg.fileByteSize = 0;
    reqMsg.fileProperty = prop;
    reqMsg.arrayName = &arrayName;
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

MetaFileContext*
MetaFsFileControlApi::GetFileInfo(FileDescriptorType fd)
{
    auto it = nameMapByfd.find(fd);

    // the list already has fd's context
    if (it != nameMapByfd.end())
    {
        auto result = idxMapByName.find(it->second);

        assert(result != idxMapByName.end());

        return &cxtList[result->second];
    }

    return nullptr;
}

std::vector<MetaFileInfoDumpCxt>
MetaFsFileControlApi::Wbt_GetMetaFileList(void)
{
    POS_EVENT_ID rc;
    MetaFsFileControlRequest reqMsg;
    std::vector<MetaFileInfoDumpCxt> result;

    result.clear();

    reqMsg.reqType = MetaFsFileControlType::GetMetaFileInfoList;
    reqMsg.arrayName = &arrayName;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::_HandleGetMetaFileInodeListReq()

    if (rc == POS_EVENT_ID::SUCCESS)
    {
        std::vector<MetaFileInfoDumpCxt>* fileInfoListPointer = reqMsg.completionData.fileInfoListPointer;

        if (fileInfoListPointer == nullptr)
        {
            return result;
        }

        for (unsigned int i = 0; i < (*fileInfoListPointer).size(); i++)
        {
            result.push_back((*fileInfoListPointer)[i]);
        }

        delete fileInfoListPointer;
    }

    return result;
}

FileSizeType
MetaFsFileControlApi::Wbt_GetMaxFileSizeLimit(void)
{
    POS_EVENT_ID rc;
    MetaFsFileControlRequest reqMsg;
    FileSizeType result = 0;

    reqMsg.reqType = MetaFsFileControlType::GetMaxFileSizeLimit;
    reqMsg.arrayName = &arrayName;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::_HandleGetMaxFileSizeLimitReq()

    if (rc == POS_EVENT_ID::SUCCESS)
    {
        result = reqMsg.completionData.maxFileSizeByteLimit;
    }

    return result;
}

MetaFileInodeDumpCxt
MetaFsFileControlApi::Wbt_GetMetaFileInode(std::string& fileName)
{
    POS_EVENT_ID rc;
    MetaFsFileControlRequest reqMsg;
    MetaFileInodeDumpCxt result;

    reqMsg.reqType = MetaFsFileControlType::GetFileInode;
    reqMsg.fileName = &fileName;
    reqMsg.arrayName = &arrayName;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::_HandleGetFileInodeReq()

    if (rc == POS_EVENT_ID::SUCCESS)
    {
        result.inodeInfo = *reqMsg.completionData.inodeInfoPointer;
    }

    return result;
}

MetaFileInodeInfo*
MetaFsFileControlApi::_GetFileInode(std::string& fileName)
{
    MetaFsFileControlRequest reqMsg;
    POS_EVENT_ID rc;

    reqMsg.reqType = MetaFsFileControlType::GetFileInode;
    reqMsg.fileName = &fileName;
    reqMsg.arrayName = &arrayName;

    rc = volMgr->HandleNewRequest(reqMsg); // MetaVolumeManager::_HandleGetFileInodeReq()

    if (POS_EVENT_ID::SUCCESS == rc)
    {
        MetaFileInodeInfo* fileInodePointer = reqMsg.completionData.inodeInfoPointer;

        return fileInodePointer;
    }

    return nullptr;
}

void
MetaFsFileControlApi::_AddFileContext(std::string& fileName, FileDescriptorType fd)
{
    // get the inode
    MetaFileInodeInfo* info = _GetFileInode(fileName);

    // get the position
    uint32_t index = 0;
    {
        SPIN_LOCK_GUARD_IN_SCOPE(iLock);
        index = bitmap->FindFirstZero();
        bitmap->SetBit(index);
        nameMapByfd.insert(std::pair<FileDescriptorType, std::string>(fd, fileName));
        idxMapByName.insert(std::pair<std::string, uint32_t>(fileName, index));
    }

    // update
    MetaFileContext* context = &cxtList[index];
    context->isActivated = info->data.field.inUse;
    context->storageType = info->data.field.dataLocation;
    context->sizeInByte = info->data.field.fileByteSize;
    context->fileBaseLpn = info->data.field.extentMap.baseMetaLpn;
    context->chunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
}

void
MetaFsFileControlApi::_RemoveFileContext(FileDescriptorType fd)
{
    auto it1 = nameMapByfd.find(fd);
    std::string fileName = it1->second;

    auto it2 = idxMapByName.find(fileName);
    uint32_t index = it2->second;

    {
        SPIN_LOCK_GUARD_IN_SCOPE(iLock);
        nameMapByfd.erase(it1);
        idxMapByName.erase(it2);
        bitmap->ClearBit(index);
    }

    cxtList[index].Reset();
}

void
MetaFsFileControlApi::SetMss(MetaStorageSubsystem* metaStorage)
{
    volMgr->SetMss(metaStorage);
}

void
MetaFsFileControlApi::InitVolume(MetaVolumeType volType, std::string arrayName, MetaLpnType maxVolPageNum)
{
    volMgr->InitVolume(volType, arrayName, maxVolPageNum);
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

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
bool
MetaFsFileControlApi::Compaction(bool isNPOR)
{
    return volMgr->Compaction(isNPOR);
}
#endif
} // namespace pos
