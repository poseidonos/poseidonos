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

#include <string>
#include "meta_volume.h"
#include "meta_volume_container.h"
#include "metafs_common.h"
#include "metafs_log.h"

namespace pos
{
MetaVolume::MetaVolume(void)
: volumeBaseLpn(),
  maxVolumeLpn(0),
  volumeType(MetaVolumeType::Max),
  volumeState(MetaVolumeState::Default),
  inodeMgr(nullptr),
  catalogMgr(nullptr),
  inUse(false),
  sumOfRegionBaseLpns(0),
  metaStorage(nullptr),
  arrayId(INT32_MAX)
{
}

MetaVolume::MetaVolume(int arrayId, MetaVolumeType metaVolumeType,
            MetaLpnType maxVolumePageNum, InodeManager* inodeMgr,
            CatalogManager* catalogMgr)
: MetaVolume()
{
    this->inodeMgr = inodeMgr;
    if (nullptr == inodeMgr)
        this->inodeMgr = new InodeManager(arrayId);

    this->catalogMgr = catalogMgr;
    if (nullptr == catalogMgr)
        this->catalogMgr = new CatalogManager(arrayId);

    maxVolumeLpn = maxVolumePageNum;
    volumeType = metaVolumeType;
    this->arrayId = arrayId;
}

MetaVolume::~MetaVolume(void)
{
    regionMgrMap.clear();
    fd2VolTypehMap.clear();
    fileKey2VolTypeMap.clear();

    delete inodeMgr;
    delete catalogMgr;

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "MetaVolume: {}: destructed",
        (uint32_t)volumeType);
}

void
MetaVolume::Init(MetaStorageSubsystem* metaStorage)
{
    _RegisterRegionMgr(MetaRegionManagerType::VolCatalogMgr, *catalogMgr);
    _RegisterRegionMgr(MetaRegionManagerType::InodeMgr, *inodeMgr);

    sumOfRegionBaseLpns = 0;

    InitVolumeBaseLpn();
    _SetupRegionInfoToRegionMgrs(metaStorage);

    this->metaStorage = metaStorage;
    volumeState = MetaVolumeState::Init;
}

void
MetaVolume::_SetupRegionInfoToRegionMgrs(MetaStorageSubsystem* metaStorage)
{
    MetaLpnType targetBaseLpn = volumeBaseLpn;

    sumOfRegionBaseLpns = 0;
    /*************************************************************
        catalogMgr | inodeMgr->Header | inodeMgr->table
        fileMgr starts from here! => sumOfRegionBaseLpn
     ************************************************************/
    // catalogMgr/inodeMgr/fileMgr
    for (int idx = (int)MetaRegionManagerType::First; idx <= (int)MetaRegionManagerType::Last; ++idx)
    {
        OnVolumeMetaRegionManager& regionMgr = _GetRegionMgr((MetaRegionManagerType)idx);
        regionMgr.Init(volumeType, targetBaseLpn, maxVolumeLpn);
        regionMgr.SetMss(metaStorage);

        targetBaseLpn += regionMgr.GetRegionSizeInLpn();
    }

    MetaLpnType newTargetBaseLpn = 0;
    // The NVRAM meta saves after SSD volume meta on shutdown process.
    if (GetVolumeType() == MetaVolumeType::SsdVolume)
    {
        sumOfRegionBaseLpns = inodeMgr->GetMetaFileBaseLpn();

        newTargetBaseLpn += catalogMgr->GetRegionSizeInLpn() +
            inodeMgr->GetRegionSizeInLpn() +
            inodeMgr->GetMetaFileBaseLpn();

        inodeMgr->SetMetaFileBaseLpn(newTargetBaseLpn);
    }
}

OnVolumeMetaRegionManager&
MetaVolume::_GetRegionMgr(MetaRegionManagerType region)
{
    auto item = regionMgrMap.find(region);

    return *item->second;
}

void
MetaVolume::_RegisterRegionMgr(MetaRegionManagerType region, OnVolumeMetaRegionManager& mgr)
{
    regionMgrMap.insert(std::make_pair(region, &mgr));
}

void
MetaVolume::_BringupMgrs(void)
{
    for (int idx = (int)MetaRegionManagerType::First; idx <= (int)MetaRegionManagerType::Last; ++idx)
    {
        MetaRegionManager& regionMgr = _GetRegionMgr((MetaRegionManagerType)idx);
        regionMgr.Bringup();
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Bringup Managers...done");
}

void
MetaVolume::_FinalizeMgrs(void)
{
    for (int idx = (int)MetaRegionManagerType::First; idx <= (int)MetaRegionManagerType::Last; ++idx)
    {
        MetaRegionManager& regionMgr = _GetRegionMgr((MetaRegionManagerType)idx);
        regionMgr.Finalize();
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Managers finalized...done");
}

bool
MetaVolume::CreateVolume(void)
{
    bool isSuccess = true;
    isSuccess = catalogMgr->CreateCatalog(maxVolumeLpn, MetaFsConfig::MAX_META_FILE_NUM_SUPPORT, false);
    if (true != isSuccess)
    {
        return false;
    }

    inodeMgr->CreateInitialInodeContent(MetaFsConfig::MAX_META_FILE_NUM_SUPPORT);

    catalogMgr->RegisterRegionInfo(MetaRegionType::FileInodeHdr,
        inodeMgr->GetRegionBaseLpn(MetaRegionType::FileInodeHdr),
        inodeMgr->GetRegionSizeInLpn(MetaRegionType::FileInodeHdr));

    catalogMgr->RegisterRegionInfo(MetaRegionType::FileInodeTable,
        inodeMgr->GetRegionBaseLpn(MetaRegionType::FileInodeTable),
        inodeMgr->GetRegionSizeInLpn(MetaRegionType::FileInodeTable));

    do
    {
        if (true != catalogMgr->SaveContent())
        {
            break;
        }
        if (true != inodeMgr->SaveContent())
        {
            break;
        }
        volumeState = MetaVolumeState::Created;
        return true;
    } while (false);

    return false;
}

bool
MetaVolume::OpenVolume(MetaLpnType* info, bool isNPOR)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Open volume: {}",
        (int)volumeType);

    if (false == _LoadVolumeMeta(info, isNPOR))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Load volume meta failed...");
        return false;
    }

    _BringupMgrs();

    inodeMgr->PopulateFDMapWithVolumeType(fd2VolTypehMap);
    inodeMgr->PopulateFileNameWithVolumeType(fileKey2VolTypeMap);

    volumeState = MetaVolumeState::Open;

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "OpenVolume done");
    return true;
}

bool
MetaVolume::_LoadVolumeMeta(MetaLpnType* info, bool isNPOR)
{
    bool isSuccess;
    MetaVolumeType volType = GetVolumeType();

    if (isNPOR == false || volType == MetaVolumeType::SsdVolume)
    {
        isSuccess = catalogMgr->LoadVolCatalog();
        if (false == isSuccess)
        {
            volumeState = MetaVolumeState::Error;
            return false;
        }

        isSuccess = inodeMgr->LoadInodeContent();
        if (false == isSuccess)
        {
            volumeState = MetaVolumeState::Error;
            return false;
        }
    }
    else
    {
        // The NVRAM volume meta loads from the backuped meta in SSD volume area.
        if (true != _RestoreContents(info))
        {
            return false;
        }
    }

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "All volume meta contents have been loaded. isNPOR={}", isNPOR);

    return true;
}

bool
MetaVolume::CloseVolume(MetaLpnType* info, bool& resetContext)
{
    bool ret = true;
    if (volumeState != MetaVolumeState::Open)
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_META_VOLUME_ALREADY_CLOSED,
            "Volume state is not 'Open'. Current state is '{}'. Skip volume close procedure...",
            (int)volumeState);
        return true; // just return true;
    }

    size_t size = inodeMgr->GetFileCountInActive();
    if (size != 0)
    {
        resetContext = false; // do not clear any mfs context

        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED,
            "Need to close the {} opened files!!. Then, unmount the meta file mgmt",
            size);
        return false;
    }

    do
    {
        if (true != catalogMgr->SaveContent())
        {
            ret = false;
            break;
        }

        if (true != inodeMgr->SaveContent())
        {
            ret = false;
            break;
        }

        // The NVRAM volume meta backups to the SSD volume area.
        if (GetVolumeType() == MetaVolumeType::NvRamVolume)
        {
            if (true != _BackupContents(info))
            {
                ret = false;
                break;
            }
        }
    } while (false);

    if (true == ret)
    {
        volumeState = MetaVolumeState::Close;
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "CloseVolume Done : MetaVolType={}", (int)volumeType);
    }
    _FinalizeMgrs();
    resetContext = true; // clear mfs context

    return ret;
}

MetaFileInode&
MetaVolume::GetInode(FileDescriptorType fd)
{
    return inodeMgr->GetFileInode(fd);
}

bool
MetaVolume::CopyInodeToInodeInfo(FileDescriptorType fd,
    MetaFileInodeInfo* inodeInfo /* output */)
{
    bool result = false;

    if (nullptr != inodeInfo)
    {
        MetaFileInode& inode = GetInode(fd);
        inode.SetMetaFileInfo(MetaFileUtil::ConvertToMediaType(volumeType), *inodeInfo);

        result = true;
    }

    return result;
}

uint32_t
MetaVolume::GetUtilizationInPercent(void)
{
    return inodeMgr->GetUtilizationInPercent();
}

size_t
MetaVolume::GetAvailableSpace(void)
{
    return inodeMgr->GetAvailableSpace();
}

MetaLpnType
MetaVolume::GetRegionSizeInLpn(MetaRegionType regionType)
{
    MetaLpnType result = 0;

    switch (regionType)
    {
    case MetaRegionType::VolCatalog:
        result = catalogMgr->GetRegionSizeInLpn();
        break;

    case MetaRegionType::FileInodeHdr:
        result = inodeMgr->GetRegionSizeInLpn(MetaRegionType::FileInodeHdr);
        break;

    case MetaRegionType::FileInodeTable:
        result = inodeMgr->GetRegionSizeInLpn(MetaRegionType::FileInodeTable);
        break;

    default:
        assert(false);
        break;
    }

    return result;
}

MetaVolumeType
MetaVolume::GetVolumeType(void)
{
    return volumeType;
}

MetaLpnType
MetaVolume::GetTheLastValidLpn(void)
{
    return inodeMgr->GetTheLastValidLpn();
}

void
MetaVolume::GetInodeList(std::vector<MetaFileInfoDumpCxt>*& fileInfoList)
{
    for (int entryIdx = 0; entryIdx < (int)MetaFsConfig::MAX_META_FILE_NUM_SUPPORT; entryIdx++)
    {
        if (inodeMgr->IsFileInodeInUse(entryIdx))
        {
            MetaFileInfoDumpCxt fileInfoData;
            MetaFileInode& inode = inodeMgr->GetInodeEntry(entryIdx);

            fileInfoData.fd = inode.data.basic.field.fd;
            fileInfoData.ctime = inode.data.basic.field.ctime;
            fileInfoData.fileName = inode.data.basic.field.fileName.ToString();
            fileInfoData.size = inode.data.basic.field.fileByteSize;
            fileInfoData.lpnBase = inode.data.basic.field.pagemap[0].GetStartLpn();
            fileInfoData.lpnCount = inode.data.basic.field.pagemap[0].GetCount();
            fileInfoData.location = MetaFileUtil::ConvertToMediaTypeName(volumeType);

            fileInfoList->push_back(fileInfoData);
        }
    }
}

FileControlResult
MetaVolume::CreateFile(MetaFsFileControlRequest& reqMsg)
{
    auto result = inodeMgr->CreateFileInode(reqMsg);

    if (POS_EVENT_ID::SUCCESS != result.second)
    {
        return result;
    }

    FileDescriptorType fd = result.first;
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(*reqMsg.fileName);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "fileHashKey: {}, fd: {}", fileKey, fd);

    {
        auto item = fileKey2VolTypeMap.find(fileKey);
        assert(fileKey2VolTypeMap.end() == item);
        fileKey2VolTypeMap.insert(std::make_pair(fileKey, volumeType));
    }
    {
        auto item = fd2VolTypehMap.find(fd);
        assert(fd2VolTypehMap.end() == item);
        fd2VolTypehMap.insert(std::make_pair(fd, volumeType));
    }

    return result;
}

FileControlResult
MetaVolume::DeleteFile(MetaFsFileControlRequest& reqMsg)
{
    auto result = inodeMgr->DeleteFileInode(reqMsg);

    if (POS_EVENT_ID::SUCCESS != result.second)
    {
        return result;
    }

    FileDescriptorType fd = result.first;
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(*reqMsg.fileName);

    fileKey2VolTypeMap.erase(fileKey);
    fd2VolTypehMap.erase(fd);

    return result;
}

bool
MetaVolume::CheckFileInActive(FileDescriptorType fd)
{
    return inodeMgr->CheckFileInActive(fd);
}

POS_EVENT_ID
MetaVolume::AddFileInActiveList(FileDescriptorType fd)
{
    return inodeMgr->AddFileInActiveList(fd);
}

bool
MetaVolume::IsGivenFileCreated(StringHashType fileKey)
{
    return inodeMgr->IsGivenFileCreated(fileKey);
}

void
MetaVolume::RemoveFileFromActiveList(FileDescriptorType fd)
{
    inodeMgr->RemoveFileFromActiveList(fd);
}

FileSizeType
MetaVolume::GetFileSize(FileDescriptorType fd)
{
    return inodeMgr->GetFileSize(fd);
}

FileSizeType
MetaVolume::GetDataChunkSize(FileDescriptorType fd)
{
    return inodeMgr->GetDataChunkSize(fd);
}

MetaLpnType
MetaVolume::GetFileBaseLpn(FileDescriptorType fd)
{
    return inodeMgr->GetFileBaseLpn(fd);
}

bool
MetaVolume::TrimData(MetaFsFileControlRequest& reqMsg)
{
    FileDescriptorType fd = inodeMgr->LookupDescriptorByName(*reqMsg.fileName);
    MetaFileInode& inode = inodeMgr->GetFileInode(fd);
    std::vector<MetaFileExtent> pageMap = inode.GetInodePageMap();
    MetaStorageType type = inode.GetStorageType();
    bool result = true;

    for (auto& it : pageMap)
    {
        MetaLpnType start = it.GetStartLpn();
        MetaLpnType count = it.GetCount();
        if (false == _TrimData(type, start, count))
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "DSM of start={}, count={} is failed", start, count);
            result = false;
        }
    }
    return result;
}

bool
MetaVolume::_TrimData(MetaStorageType type, MetaLpnType start, MetaLpnType count)
{
    void* buf = nullptr;
    POS_EVENT_ID ret = POS_EVENT_ID::SUCCESS;

    if (nullptr == metaStorage)
    {
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "MetaStorageSubsystem is not ready");
        return false;
    }

    ret = metaStorage->TrimFileData(type, start, buf, count);

    if (ret != POS_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "MFS file trim has been failed with NVMe Admin TRIM CMD.");

        const FileSizeType pageSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
        FileSizeType fileSize = count * pageSize;

        buf = Memory<pageSize>::Alloc(fileSize / pageSize);
        assert(buf != nullptr);

        // write all zeros
        ret = metaStorage->WritePage(type, start, buf, count); // should be async.

        if (ret != POS_EVENT_ID::SUCCESS)
        {
            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "MFS file trim has been failed with zero writing.");

            Memory<>::Free(buf);
            return false;
        }
        Memory<>::Free(buf);
    }

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "MFS file trim has been deleted!!!");

    return true;
}

FileDescriptorType
MetaVolume::LookupDescriptorByName(std::string& fileName)
{
    return inodeMgr->LookupDescriptorByName(fileName);
}

std::string
MetaVolume::LookupNameByDescriptor(FileDescriptorType fd)
{
    return inodeMgr->LookupNameByDescriptor(fd);
}

bool
MetaVolume::_BackupContents(MetaLpnType* info)
{
    bool isSuccess = true;
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NVRAM volume meta '{}' backups to SSD volume region '{}'. Start...",
        (int)GetVolumeType(), (int)MetaVolumeType::SsdVolume);

    MetaLpnType baseLpnInVol = info[(uint32_t)BackupInfo::BaseLpn];
    MetaLpnType lpnCnts = info[(uint32_t)BackupInfo::CatalogSize]; // catalog

    MetaLpnType iNodeHdrLpnCnts = info[(uint32_t)BackupInfo::InodeHdrSize];     // inode header
    MetaLpnType iNodeTableLpnCnts = info[(uint32_t)BackupInfo::InodeTableSize]; // inode table

    // The data is catalog contents on the NVRAM. (NVMe volume instance + SSDVolume file interface)
    isSuccess = catalogMgr->BackupContent(MetaVolumeType::SsdVolume, baseLpnInVol, lpnCnts);
    if (!isSuccess)
    {
        return false;
    }

    baseLpnInVol += lpnCnts;

    // The data is inode contents on the NVRAM.
    isSuccess = inodeMgr->BackupContent(MetaVolumeType::SsdVolume, baseLpnInVol, iNodeHdrLpnCnts, iNodeTableLpnCnts);
    if (!isSuccess)
    {
        return false;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NVRAM volume meta has successfully backuped to SSD volume!");
    return isSuccess;
}

bool
MetaVolume::_RestoreContents(MetaLpnType* info)
{
    bool isSuccess = true;
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NVRAM volume meta '{}' is restored from SSD volume region '{}'. Start...",
        (int)GetVolumeType(), (int)MetaVolumeType::SsdVolume);

    MetaLpnType baseLpnInVol = info[(uint32_t)BackupInfo::BaseLpn];
    MetaLpnType lpnCnts = info[(uint32_t)BackupInfo::CatalogSize]; // catalog

    MetaLpnType iNodeHdrLpnCnts = info[(uint32_t)BackupInfo::InodeHdrSize];     // inode header
    MetaLpnType iNodeTableLpnCnts = info[(uint32_t)BackupInfo::InodeTableSize]; // inode table

    isSuccess = catalogMgr->RestoreContent(MetaVolumeType::SsdVolume, baseLpnInVol, lpnCnts);
    if (!isSuccess)
    {
        return false;
    }

    baseLpnInVol += lpnCnts;

    isSuccess = inodeMgr->RestoreContent(MetaVolumeType::SsdVolume, baseLpnInVol, iNodeHdrLpnCnts, iNodeTableLpnCnts);
    if (!isSuccess)
    {
        return false;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NVRAM volume meta has successfully been restoreed from SSD volume!");

    return isSuccess;
}
} // namespace pos
