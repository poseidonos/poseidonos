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
MetaVolume::MetaVolume(std::string arrayName, MetaVolumeType metaVolumeType, MetaLpnType maxVolumePageNum)
: volumeBaseLpn(),
  maxVolumeLpn(maxVolumePageNum),
  volumeType(metaVolumeType),
  volumeState(MetaVolumeState::Default),
  inUse(false),
  sumOfRegionBaseLpns(0),
  arrayName(arrayName)
{
    fileMgr = new MetaFileManager(arrayName);
    inodeMgr = new MetaFileInodeManager(arrayName);
    catalogMgr = new VolumeCatalogManager(arrayName);
}

MetaVolume::MetaVolume(MetaFileManager* fileMgr, MetaFileInodeManager* inodeMgr,
        VolumeCatalogManager* catalogMgr, std::string arrayName,
        MetaVolumeType metaVolumeType, MetaLpnType maxVolumePageNum)
: volumeBaseLpn(),
  maxVolumeLpn(maxVolumePageNum),
  volumeType(metaVolumeType),
  volumeState(MetaVolumeState::Default),
  inUse(false),
  sumOfRegionBaseLpns(0),
  arrayName(arrayName)
{
    this->fileMgr = fileMgr;
    this->inodeMgr = inodeMgr;
    this->catalogMgr = catalogMgr;
}

MetaVolume::~MetaVolume(void)
{
    regionMgrMap.clear();

    delete fileMgr;
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
    _RegisterRegionMgr(MetaRegionManagerType::FileMgr, *fileMgr);

    sumOfRegionBaseLpns = 0;

    InitVolumeBaseLpn();
    _SetupRegionInfoToRegionMgrs(metaStorage);

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
    for (auto region : Enum<MetaRegionManagerType>()) // catalogMgr/inodeMgr/fileMgr
    {
        OnVolumeMetaRegionManager& regionMgr = _GetRegionMgr(region);
        regionMgr.Init(volumeType, targetBaseLpn, maxVolumeLpn);
        regionMgr.SetMss(metaStorage);

        targetBaseLpn += regionMgr.GetRegionSizeInLpn();
    }

    MetaLpnType newTargetBaseLpn = 0;
    // The NVRAM meta saves after SSD volume meta on shutdown process.
    if (GetVolumeType() == MetaVolumeType::SsdVolume)
    {
        sumOfRegionBaseLpns = fileMgr->GetFileBaseLpn();

        newTargetBaseLpn += catalogMgr->GetRegionSizeInLpn() +
            inodeMgr->GetRegionSizeInLpn() +
            fileMgr->GetFileBaseLpn();

        fileMgr->SetFileBaseLpn(newTargetBaseLpn);
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
    for (auto region : Enum<MetaRegionManagerType>())
    {
        MetaRegionManager& regionMgr = _GetRegionMgr(region);
        regionMgr.Bringup();
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Bringup Managers...done");
}

void
MetaVolume::_FinalizeMgrs(void)
{
    for (auto region : Enum<MetaRegionManagerType>())
    {
        MetaRegionManager& regionMgr = _GetRegionMgr(region);
        regionMgr.Finalize();
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Managers finalized...done");
}

bool
MetaVolume::CreateNewVolume(void)
{
    bool isSuccess = true;
    isSuccess = catalogMgr->CreateVolumeCatalog(maxVolumeLpn, MetaFsConfig::MAX_META_FILE_NUM_SUPPORT, false);
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

    fileMgr->GetExtentContent(inodeMgr->GetInodeHdrExtentMapBase());

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

    if (false == _LoadAllVolumeMeta(info, isNPOR))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Load volume meta failed...");
        return false;
    }
    _BringupMgrs();

    volumeState = MetaVolumeState::Open;

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "OpenVolume done");
    return true;
}

bool
MetaVolume::_LoadAllVolumeMeta(MetaLpnType* info, bool isNPOR)
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
    { // The NVRAM volume meta loads from the backuped meta in SSD volume area.
        if (true != _RestoreContents(info))
        {
            return false;
        }
    }

    fileMgr->SetExtentContent(inodeMgr->GetInodeHdrExtentMapBase());

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
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

    FileDescriptorSet fdSet = fileMgr->GetFDSetOfActiveFiles();
    if (fdSet.size() != 0)
    {
        resetContext = false; // do not clear any mfs context

        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED,
            "Need to close the {} opened files!!. Then, unmount the meta file mgmt",
            fdSet.size());
        return false;
    }

    do
    {
        if (true != catalogMgr->SaveContent())
        {
            ret = false;
            break;
        }
        if (true == fileMgr->IsAllocated())
        {
            fileMgr->GetExtentContent(inodeMgr->GetInodeHdrExtentMapBase());
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

bool
MetaVolume::IsVolumeOpened(void)
{
    return (volumeState == MetaVolumeState::Open) ? true : false;
}

void
MetaVolume::BuildFDMap(std::unordered_map<FileDescriptorType, MetaVolumeType>& targetMap)
{
    inodeMgr->PopulateFDMapWithVolumeType(targetMap);
}

void
MetaVolume::BuildFileNameMap(std::unordered_map<StringHashType, MetaVolumeType>& targetMap)
{
    inodeMgr->PopulateFileNameWithVolumeType(targetMap);
}

void
MetaVolume::BuildFreeFDMap(std::map<FileDescriptorType, FileDescriptorType>& freeFDMap)
{
    inodeMgr->RemoveFDsInUse(freeFDMap);
}

void
MetaVolume::BuildFDLookupMap(std::unordered_map<StringHashType, FileDescriptorType>& targetMap)
{
    inodeMgr->PopulateFileKeyWithFD(targetMap);
}

uint32_t
MetaVolume::GetUtilizationInPercent(void)
{
    return fileMgr->GetUtilizationInPercent();
}

size_t
MetaVolume::GetTheBiggestExtentSize(void)
{
    return fileMgr->GetTheBiggestExtentSize();
}

MetaVolumeType
MetaVolume::GetVolumeType(void)
{
    return volumeType;
}

bool
MetaVolume::CopyExtentContent(void)
{
    fileMgr->GetExtentContent(inodeMgr->GetInodeHdrExtentMapBase());

    return true;
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

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
bool
MetaVolume::Compaction(void)
{
    if (true == inodeMgr->Compaction())
    {
        fileMgr->SetExtentContent(inodeMgr->GetInodeHdrExtentMapBase());
        return true;
    }
    else
        return false;
}
#endif
} // namespace pos
