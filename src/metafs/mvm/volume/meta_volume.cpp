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

#include <string>
#include "meta_volume.h"
#include "meta_volume_container.h"
#include "metafs_common.h"
#include "metafs_log.h"
#include "src/metafs/mvm/volume/inode_creator.h"
#include "src/metafs/mvm/volume/inode_deleter.h"

namespace pos
{
MetaVolume::MetaVolume(void)
{
}

MetaVolume::MetaVolume(int arrayId, MetaVolumeType metaVolumeType,
            MetaLpnType maxVolumePageNum, InodeManager* inodeMgr,
            CatalogManager* catalogMgr, InodeCreator* inodeCreator,
            InodeDeleter* inodeDeleter)
: MetaVolume()
{
    this->inodeMgr = (nullptr == inodeMgr) ? new InodeManager(arrayId) : inodeMgr;
    this->catalogMgr = (nullptr == catalogMgr) ? new CatalogManager(arrayId) : catalogMgr;

    maxVolumeLpn = maxVolumePageNum;
    volumeType = metaVolumeType;
    this->arrayId = arrayId;

    trimBuffer = Memory<MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES>::Alloc();
    assert(trimBuffer != nullptr);

    this->inodeCreator = (nullptr == inodeCreator) ? new InodeCreator(this->inodeMgr) : inodeCreator;
    this->inodeDeleter = (nullptr == inodeDeleter) ? new InodeDeleter(this->inodeMgr) : inodeDeleter;
}

// LCOV_EXCL_START
MetaVolume::~MetaVolume(void)
{
    regionMgrMap.clear();
    fd2VolTypehMap.clear();
    fileKey2VolTypeMap.clear();

    delete inodeMgr;
    delete catalogMgr;

    if (nullptr != trimBuffer)
        Memory<>::Free(trimBuffer);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "MetaVolume: {}: destructed", (uint32_t)volumeType);

    delete inodeCreator;
    delete inodeDeleter;
}
// LCOV_EXCL_STOP

void
MetaVolume::Init(MetaStorageSubsystem* metaStorage)
{
    regionMgrMap.insert({ MetaRegionManagerType::VolCatalogMgr, catalogMgr });
    regionMgrMap.insert({ MetaRegionManagerType::InodeMgr, inodeMgr });

    InitVolumeBaseLpn();
    _SetupRegionInfoToRegionMgrs(metaStorage);

    this->metaStorage = metaStorage;
    volumeState = MetaVolumeState::Init;
}

void
MetaVolume::_SetupRegionInfoToRegionMgrs(MetaStorageSubsystem* metaStorage)
{
    MetaLpnType targetBaseLpn = volumeBaseLpn;
    MetaLpnType newTargetBaseLpn = 0;

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

bool
MetaVolume::CreateVolume(void)
{
    bool isSuccess = catalogMgr->CreateCatalog(maxVolumeLpn, MetaFsConfig::MAX_META_FILE_NUM_SUPPORT, false);

    if (true == isSuccess)
    {
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
    }

    return false;
}

bool
MetaVolume::OpenVolume(MetaLpnType* info, bool isNPOR)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Trying to open meta volume(type: {})", (int)volumeType);

    if (false == _LoadVolumeMeta(info, isNPOR))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE, "Load volume meta failed...");
        return false;
    }

    for (int idx = (int)MetaRegionManagerType::First; idx <= (int)MetaRegionManagerType::Last; ++idx)
    {
        MetaRegionManager& regionMgr = _GetRegionMgr((MetaRegionManagerType)idx);
        regionMgr.Bringup();
    }

    inodeMgr->PopulateFDMapWithVolumeType(fd2VolTypehMap);
    inodeMgr->PopulateFileNameWithVolumeType(fileKey2VolTypeMap);

    volumeState = MetaVolumeState::Open;

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Finished opening meta volume(type: {})", (int)volumeType);

    return true;
}

bool
MetaVolume::_LoadVolumeMeta(MetaLpnType* info, bool isNPOR)
{
    if (isNPOR == false || GetVolumeType() != MetaVolumeType::NvRamVolume)
    {
        if (false == catalogMgr->LoadVolCatalog())
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_LOAD_FAILED,
                "Failed to load volume catalog(volume type: {})", (int)volumeType);
            volumeState = MetaVolumeState::Error;
            return false;
        }

        if (false == inodeMgr->LoadContent())
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_LOAD_FAILED,
                "Failed to load volume inode contents(volume type: {})", (int)volumeType);
            volumeState = MetaVolumeState::Error;
            return false;
        }
    }
    else
    {
        // The NVRAM volume meta loads from the backuped meta in SSD volume area.
        if (true != _RestoreContents(info))
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_SAVE_FAILED,
                "Failed to store NVRAM meta volume");
            return false;
        }
    }

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Successfully loaded the contents of meta volume(type: {}). isNPOR={}",
        (int)volumeType, isNPOR);

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
            "Need to close the {} opened files!!. Then, unmount the meta file mgmt", size);
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

    for (int idx = (int)MetaRegionManagerType::First; idx <= (int)MetaRegionManagerType::Last; ++idx)
    {
        MetaRegionManager& regionMgr = _GetRegionMgr((MetaRegionManagerType)idx);
        regionMgr.Finalize();
    }

    resetContext = true;

    return ret;
}

bool
MetaVolume::CopyInodeToInodeInfo(FileDescriptorType fd,
    MetaFileInodeInfo* inodeInfo /* output */)
{
    if (nullptr == inodeInfo)
        return false;

    MetaFileInode& inode = GetInode(fd);
    inode.SetMetaFileInfo(MetaFileUtil::ConvertToMediaType(volumeType), *inodeInfo);

    return true;
}

MetaLpnType
MetaVolume::GetRegionSizeInLpn(MetaRegionType regionType)
{
    switch (regionType)
    {
    case MetaRegionType::VolCatalog:
        return catalogMgr->GetRegionSizeInLpn();

    case MetaRegionType::FileInodeHdr:
        return inodeMgr->GetRegionSizeInLpn(regionType);

    case MetaRegionType::FileInodeTable:
        return inodeMgr->GetRegionSizeInLpn(regionType);

    default:
        assert(false);
    }
}

MetaVolumeType
MetaVolume::GetVolumeType(void)
{
    return volumeType;
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
    auto result = inodeCreator->Create(reqMsg);

    if (POS_EVENT_ID::SUCCESS != result.second)
        return result;

    FileDescriptorType fd = result.first;
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(*reqMsg.fileName);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "fileHashKey: {}, fd: {}", fileKey, fd);

    assert(fileKey2VolTypeMap.end() == fileKey2VolTypeMap.find(fileKey));
    fileKey2VolTypeMap.insert(std::make_pair(fileKey, volumeType));

    assert(fd2VolTypehMap.end() == fd2VolTypehMap.find(fd));
    fd2VolTypehMap.insert(std::make_pair(fd, volumeType));

    return result;
}

FileControlResult
MetaVolume::DeleteFile(MetaFsFileControlRequest& reqMsg)
{
    auto result = inodeDeleter->Delete(reqMsg);

    if (POS_EVENT_ID::SUCCESS != result.second)
        return result;

    FileDescriptorType fd = result.first;
    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(*reqMsg.fileName);

    fileKey2VolTypeMap.erase(fileKey);
    fd2VolTypehMap.erase(fd);

    return result;
}

bool
MetaVolume::TrimData(MetaFsFileControlRequest& reqMsg)
{
    FileDescriptorType fd = inodeMgr->LookupDescriptorByName(*reqMsg.fileName);
    MetaFileInode& inode = inodeMgr->GetFileInode(fd);
    std::vector<MetaFileExtent> pageMap = inode.GetInodePageMap();

    for (auto& it : pageMap)
    {
        if (false == _TrimData(inode.GetStorageType(), it.GetStartLpn(), it.GetCount()))
        {
            POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "DSM of start={}, count={} is failed", it.GetStartLpn(), it.GetCount());
            return false;
        }
    }
    return true;
}

bool
MetaVolume::_TrimData(MetaStorageType type, MetaLpnType start, MetaLpnType count)
{
    void* buf = nullptr;
    POS_EVENT_ID ret = POS_EVENT_ID::SUCCESS;

    if (nullptr == metaStorage)
    {
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE, "MetaStorageSubsystem is not ready");
        return false;
    }

    ret = metaStorage->TrimFileData(type, start, buf, count);

    if (ret != POS_EVENT_ID::SUCCESS)
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "MFS file trim has been failed with NVMe Admin TRIM CMD.");

        // write all zeros
        ret = metaStorage->WritePage(type, start, trimBuffer, count); // should be async.

        if (ret != POS_EVENT_ID::SUCCESS)
        {
            MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "MFS file trim has been failed with zero writing.");

            return false;
        }
    }

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE, "MFS file trim has been deleted!!!");

    return true;
}

bool
MetaVolume::_BackupContents(MetaLpnType* info)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NVRAM volume meta '{}' backups to SSD volume region '{}'. Start...",
        (int)GetVolumeType(), (int)MetaVolumeType::SsdVolume);

    MetaLpnType baseLpnInVol = info[(uint32_t)BackupInfo::BaseLpn];
    MetaLpnType lpnCnts = info[(uint32_t)BackupInfo::CatalogSize]; // catalog

    MetaLpnType iNodeHdrLpnCnts = info[(uint32_t)BackupInfo::InodeHdrSize];     // inode header
    MetaLpnType iNodeTableLpnCnts = info[(uint32_t)BackupInfo::InodeTableSize]; // inode table

    // The data is catalog contents on the NVRAM. (NVMe volume instance + SSDVolume file interface)
    if (false == catalogMgr->BackupContent(MetaVolumeType::SsdVolume, baseLpnInVol, lpnCnts))
        return false;

    baseLpnInVol += lpnCnts;

    // The data is inode contents on the NVRAM.
    if (false == inodeMgr->BackupContent(MetaVolumeType::SsdVolume, baseLpnInVol, iNodeHdrLpnCnts, iNodeTableLpnCnts))
        return false;

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NVRAM volume meta has successfully backuped to SSD volume!");
    return true;
}

bool
MetaVolume::_RestoreContents(MetaLpnType* info)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NVRAM volume meta '{}' is restored from SSD volume region '{}'. Start...",
        (int)GetVolumeType(), (int)MetaVolumeType::SsdVolume);

    MetaLpnType baseLpnInVol = info[(uint32_t)BackupInfo::BaseLpn];
    MetaLpnType lpnCnts = info[(uint32_t)BackupInfo::CatalogSize]; // catalog

    MetaLpnType iNodeHdrLpnCnts = info[(uint32_t)BackupInfo::InodeHdrSize];     // inode header
    MetaLpnType iNodeTableLpnCnts = info[(uint32_t)BackupInfo::InodeTableSize]; // inode table

    if (false == catalogMgr->RestoreContent(MetaVolumeType::SsdVolume, baseLpnInVol, lpnCnts))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_SAVE_FAILED,
            "Failed to restore NVRAM catalog from SSD meta volume");
        return false;
    }

    baseLpnInVol += lpnCnts;

    if (false == inodeMgr->RestoreContent(MetaVolumeType::SsdVolume, baseLpnInVol, iNodeHdrLpnCnts, iNodeTableLpnCnts))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_SAVE_FAILED,
            "Failed to restore NVRAM inode contents from SSD meta volume");
        return false;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NVRAM volume meta has successfully been restoreed from SSD volume!");

    return true;
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

MetaFileInode&
MetaVolume::GetInode(FileDescriptorType fd)
{
    return inodeMgr->GetFileInode(fd);
}

uint32_t
MetaVolume::GetUtilizationInPercent(void)
{
    return ((maxVolumeLpn - inodeMgr->GetAvailableLpnCount()) * 100) / maxVolumeLpn;
}

size_t
MetaVolume::GetAvailableSpace(void)
{
    return inodeMgr->GetAvailableSpace();
}

MetaLpnType
MetaVolume::GetTheLastValidLpn(void)
{
    return inodeMgr->GetTheLastValidLpn();
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
} // namespace pos
