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

#include "meta_volume.h"

#include <string>

#include "meta_volume_container.h"
#include "metafs_common.h"
#include "metafs_log.h"
#include "src/metafs/mvm/volume/inode_creator.h"
#include "src/metafs/mvm/volume/inode_deleter.h"

namespace pos
{
MetaVolume::MetaVolume(const int arrayId, const MetaVolumeType metaVolumeType,
    const MetaLpnType maxVolumePageNum, InodeManager* inodeMgr,
    CatalogManager* catalogMgr, InodeCreator* inodeCreator,
    InodeDeleter* inodeDeleter)
: volumeBaseLpn_(0),
  maxVolumeLpn_(maxVolumePageNum),
  volumeType_(metaVolumeType),
  volumeState_(MetaVolumeState::Default),
  inodeMgr_(inodeMgr),
  catalogMgr_(catalogMgr),
  sumOfRegionBaseLpns_(0),
  metaStorage_(nullptr),
  arrayId_(arrayId),
  trimBuffer_(nullptr),
  inodeCreator_(inodeCreator),
  inodeDeleter_(inodeDeleter)
{
    if (!inodeMgr_)
        inodeMgr_ = new InodeManager(arrayId_);

    if (!catalogMgr_)
        catalogMgr_ = new CatalogManager(arrayId_);

    if (!inodeCreator_)
        inodeCreator_ = new InodeCreator(inodeMgr_);

    if (!inodeDeleter_)
        inodeDeleter_ = new InodeDeleter(inodeMgr_);

    trimBuffer_ = Memory<MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES>::Alloc();
    if (!trimBuffer_)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
            "The trim buffer cannot be allocated by Memory.");
        assert(false);
    }
}

// LCOV_EXCL_START
MetaVolume::~MetaVolume(void)
{
    regionMgrMap_.clear();
    fd2VolTypehMap_.clear();
    fileKey2VolTypeMap_.clear();

    if (inodeMgr_)
    {
        delete inodeMgr_;
        inodeMgr_ = nullptr;
    }

    if (catalogMgr_)
    {
        delete catalogMgr_;
        catalogMgr_ = nullptr;
    }

    if (inodeCreator_)
    {
        delete inodeCreator_;
        inodeCreator_ = nullptr;
    }

    if (inodeDeleter_)
    {
        delete inodeDeleter_;
        inodeDeleter_ = nullptr;
    }

    if (trimBuffer_)
    {
        Memory<>::Free(trimBuffer_);
        trimBuffer_ = nullptr;
    }
}
// LCOV_EXCL_STOP

void
MetaVolume::Init(MetaStorageSubsystem* metaStorage)
{
    regionMgrMap_.insert({MetaRegionManagerType::VolCatalogMgr, catalogMgr_});
    regionMgrMap_.insert({MetaRegionManagerType::InodeMgr, inodeMgr_});

    InitVolumeBaseLpn();
    _SetupRegionInfoToRegionMgrs(metaStorage);

    this->metaStorage_ = metaStorage;
    volumeState_ = MetaVolumeState::Init;
}

void
MetaVolume::_SetupRegionInfoToRegionMgrs(MetaStorageSubsystem* metaStorage)
{
    MetaLpnType targetBaseLpn = volumeBaseLpn_;
    MetaLpnType newTargetBaseLpn = 0;

    sumOfRegionBaseLpns_ = 0;
    /*************************************************************
        catalogMgr_ | inodeMgr->Header | inodeMgr_->table
        fileMgr starts from here! => sumOfRegionBaseLpn
     ************************************************************/
    // catalogMgr_/inodeMgr_/fileMgr
    for (int idx = (int)MetaRegionManagerType::First; idx <= (int)MetaRegionManagerType::Last; ++idx)
    {
        OnVolumeMetaRegionManager& regionMgr = _GetRegionMgr((MetaRegionManagerType)idx);
        regionMgr.Init(volumeType_, targetBaseLpn, maxVolumeLpn_);
        regionMgr.SetMss(metaStorage);

        targetBaseLpn += regionMgr.GetRegionSizeInLpn();
    }

    // The NVRAM meta saves after SSD volume meta on shutdown process.
    if (volumeType_ == MetaVolumeType::SsdVolume)
    {
        sumOfRegionBaseLpns_ = inodeMgr_->GetMetaFileBaseLpn();

        newTargetBaseLpn += catalogMgr_->GetRegionSizeInLpn() +
            inodeMgr_->GetRegionSizeInLpn() +
            inodeMgr_->GetMetaFileBaseLpn();

        inodeMgr_->SetMetaFileBaseLpn(newTargetBaseLpn);
    }
}

OnVolumeMetaRegionManager&
MetaVolume::_GetRegionMgr(MetaRegionManagerType region)
{
    auto item = regionMgrMap_.find(region);

    return *item->second;
}

bool
MetaVolume::CreateVolume(void)
{
    if (!catalogMgr_->CreateCatalog(maxVolumeLpn_, MetaFsConfig::MAX_META_FILE_NUM_SUPPORT, false))
    {
        return false;
    }

    inodeMgr_->CreateInitialInodeContent(MetaFsConfig::MAX_META_FILE_NUM_SUPPORT);

    catalogMgr_->RegisterRegionInfo(MetaRegionType::FileInodeHdr,
        inodeMgr_->GetRegionBaseLpn(MetaRegionType::FileInodeHdr),
        inodeMgr_->GetRegionSizeInLpn(MetaRegionType::FileInodeHdr));

    catalogMgr_->RegisterRegionInfo(MetaRegionType::FileInodeTable,
        inodeMgr_->GetRegionBaseLpn(MetaRegionType::FileInodeTable),
        inodeMgr_->GetRegionSizeInLpn(MetaRegionType::FileInodeTable));

    if (!catalogMgr_->SaveContent())
        return false;

    if (!inodeMgr_->SaveContent())
        return false;

    volumeState_ = MetaVolumeState::Created;
    return true;
}

bool
MetaVolume::OpenVolume(MetaLpnType* info, bool isNPOR)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Trying to open meta volume(type: {})", (int)volumeType_);

    if (false == _LoadVolumeMeta(info, isNPOR))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE, "Load volume meta failed...");
        return false;
    }

    for (int idx = (int)MetaRegionManagerType::First; idx <= (int)MetaRegionManagerType::Last; ++idx)
    {
        _GetRegionMgr((MetaRegionManagerType)idx).Bringup();
    }

    inodeMgr_->PopulateFDMapWithVolumeType(fd2VolTypehMap_);
    inodeMgr_->PopulateFileNameWithVolumeType(fileKey2VolTypeMap_);

    volumeState_ = MetaVolumeState::Open;

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Finished opening meta volume(type: {})", (int)volumeType_);

    return true;
}

bool
MetaVolume::_LoadVolumeMeta(MetaLpnType* info, bool isNPOR)
{
    if (!isNPOR || volumeType_ == MetaVolumeType::SsdVolume)
    {
        if (!catalogMgr_->LoadVolCatalog())
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_LOAD_FAILED,
                "Failed to load volume catalog(volume type: {})", (int)volumeType_);
            volumeState_ = MetaVolumeState::Error;
            return false;
        }

        if (!inodeMgr_->LoadContent())
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_LOAD_FAILED,
                "Failed to load volume inode contents(volume type: {})", (int)volumeType_);
            volumeState_ = MetaVolumeState::Error;
            return false;
        }
    }
    else
    {
        // The NVRAM volume meta loads from the backuped meta in SSD volume area.
        if (!_RestoreContents(info))
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_SAVE_FAILED,
                "Failed to store NVRAM meta volume");
            return false;
        }
    }

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Successfully loaded the contents of meta volume(type: {}). isNPOR: {}",
        (int)volumeType_, isNPOR);

    return true;
}

bool
MetaVolume::CloseVolume(MetaLpnType* info, bool& resetContext)
{
    bool ret = true;
    if (volumeState_ != MetaVolumeState::Open)
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_META_VOLUME_ALREADY_CLOSED,
            "Volume state is not 'Open'. Current state is '{}'. Skip volume close procedure...",
            (int)volumeState_);
        return true; // just return true;
    }

    if (inodeMgr_->GetFileCountInActive())
    {
        resetContext = false; // do not clear any mfs context

        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED,
            "Need to close the {} opened files!!. Then, unmount the meta file mgmt",
            inodeMgr_->GetFileCountInActive());
        return false;
    }

    do
    {
        if (!catalogMgr_->SaveContent())
        {
            ret = false;
            break;
        }

        if (!inodeMgr_->SaveContent())
        {
            ret = false;
            break;
        }

        // The NVRAM volume meta backups to the SSD volume area.
        if (volumeType_ == MetaVolumeType::NvRamVolume)
        {
            if (!_BackupContents(info))
            {
                ret = false;
                break;
            }
        }

        volumeState_ = MetaVolumeState::Close;
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "CloseVolume Done : volumeType_: {}", (int)volumeType_);
    } while (false);

    for (int idx = (int)MetaRegionManagerType::First; idx <= (int)MetaRegionManagerType::Last; ++idx)
    {
        _GetRegionMgr((MetaRegionManagerType)idx).Finalize();
    }

    resetContext = true;

    return ret;
}

bool
MetaVolume::CopyInodeToInodeInfo(const FileDescriptorType fd,
    MetaFileInodeInfo* inodeInfo /* output */)
{
    if (!inodeInfo)
        return false;

    GetInode(fd).SetMetaFileInfo(MetaFileUtil::ConvertToMediaType(volumeType_), *inodeInfo);

    return true;
}

MetaLpnType
MetaVolume::GetRegionSizeInLpn(MetaRegionType regionType)
{
    switch (regionType)
    {
        case MetaRegionType::VolCatalog:
            return catalogMgr_->GetRegionSizeInLpn();

        case MetaRegionType::FileInodeHdr:
            return inodeMgr_->GetRegionSizeInLpn(regionType);

        case MetaRegionType::FileInodeTable:
            return inodeMgr_->GetRegionSizeInLpn(regionType);

        default:
            assert(false);
    }
}

void
MetaVolume::GetInodeList(std::vector<MetaFileInfoDumpCxt>*& fileInfoList) const
{
    for (uint32_t entryIdx = 0; entryIdx < MetaFsConfig::MAX_META_FILE_NUM_SUPPORT; entryIdx++)
    {
        if (inodeMgr_->IsFileInodeInUse(entryIdx))
        {
            MetaFileInfoDumpCxt fileInfoData;
            MetaFileInode& inode = inodeMgr_->GetInodeEntry(entryIdx);

            fileInfoData.fd = inode.data.basic.field.fd;
            fileInfoData.ctime = inode.data.basic.field.ctime;
            fileInfoData.fileName = inode.data.basic.field.fileName.ToString();
            fileInfoData.size = inode.data.basic.field.fileByteSize;
            fileInfoData.lpnBase = inode.data.basic.field.pagemap[0].GetStartLpn();
            fileInfoData.lpnCount = inode.data.basic.field.pagemap[0].GetCount();
            fileInfoData.location = MetaFileUtil::ConvertToMediaTypeName(volumeType_);

            fileInfoList->emplace_back(fileInfoData);
        }
    }
}

FileControlResult
MetaVolume::CreateFile(MetaFsFileControlRequest& reqMsg)
{
    auto result = inodeCreator_->Create(reqMsg);

    if (POS_EVENT_ID::SUCCESS != result.second)
        return result;

    StringHashType fileKey = MetaFileUtil::GetHashKeyFromFileName(*reqMsg.fileName);

    if (fileKey2VolTypeMap_.end() != fileKey2VolTypeMap_.find(fileKey))
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
            "The fileKey {} is already existed, fd: {}, fileName: {}",
            fileKey, result.first, *reqMsg.fileName);
        assert(false);
    }

    if (fd2VolTypehMap_.end() != fd2VolTypehMap_.find(result.first))
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MFS_ERROR_MESSAGE,
            "The fd {} is already existed, fileName: {}",
            result.first, *reqMsg.fileName);
        assert(false);
    }

    fileKey2VolTypeMap_.insert({fileKey, volumeType_});
    fd2VolTypehMap_.insert({result.first, volumeType_});

    return result;
}

FileControlResult
MetaVolume::DeleteFile(MetaFsFileControlRequest& reqMsg)
{
    auto result = inodeDeleter_->Delete(reqMsg);

    if (POS_EVENT_ID::SUCCESS != result.second)
        return result;

    fileKey2VolTypeMap_.erase(MetaFileUtil::GetHashKeyFromFileName(*reqMsg.fileName));
    fd2VolTypehMap_.erase(result.first);

    return result;
}

bool
MetaVolume::TrimData(MetaFsFileControlRequest& reqMsg)
{
    FileDescriptorType fd = inodeMgr_->LookupDescriptorByName(*reqMsg.fileName);

    for (auto& it : inodeMgr_->GetFileInode(fd).GetInodePageMap())
    {
        if (!_TrimData(inodeMgr_->GetFileInode(fd).GetStorageType(), it.GetStartLpn(), it.GetCount()))
        {
            POS_TRACE_INFO((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "Trim (startLpn: {}, lpnCount: {}) has been failed.",
                it.GetStartLpn(), it.GetCount());
            return false;
        }
    }
    return true;
}

bool
MetaVolume::_TrimData(MetaStorageType type, MetaLpnType start, MetaLpnType count)
{
    if (!metaStorage_)
    {
        MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE, "MetaStorageSubsystem is not ready");
        return false;
    }

    if (POS_EVENT_ID::SUCCESS != metaStorage_->TrimFileData(type, start, nullptr, count))
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "Meta file trim has been failed with NVMe DSM.");

        if (POS_EVENT_ID::SUCCESS != metaStorage_->WritePage(type, start, trimBuffer_, count))
        {
            POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
                "Meta file trim has been failed with zero writing.");

            return false;
        }
    }

    MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE, "Meta file trim is done");

    return true;
}

bool
MetaVolume::_BackupContents(MetaLpnType* info)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NVRAM volume meta '{}' backups to SSD volume region '{}'. Start...",
        (int)volumeType_, (int)MetaVolumeType::SsdVolume);

    MetaLpnType baseLpnInVol = info[(uint32_t)BackupInfo::BaseLpn];
    MetaLpnType lpnCnts = info[(uint32_t)BackupInfo::CatalogSize]; // catalog

    MetaLpnType iNodeHdrLpnCnts = info[(uint32_t)BackupInfo::InodeHdrSize];     // inode header
    MetaLpnType iNodeTableLpnCnts = info[(uint32_t)BackupInfo::InodeTableSize]; // inode table

    // The data is catalog contents on the NVRAM. (NVMe volume instance + SSDVolume file interface)
    if (!catalogMgr_->BackupContent(MetaVolumeType::SsdVolume, baseLpnInVol, lpnCnts))
        return false;

    baseLpnInVol += lpnCnts;

    // The data is inode contents on the NVRAM.
    if (!inodeMgr_->BackupContent(MetaVolumeType::SsdVolume, baseLpnInVol, iNodeHdrLpnCnts, iNodeTableLpnCnts))
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
        (int)volumeType_, (int)MetaVolumeType::SsdVolume);

    MetaLpnType baseLpnInVol = info[(uint32_t)BackupInfo::BaseLpn];
    MetaLpnType lpnCnts = info[(uint32_t)BackupInfo::CatalogSize]; // catalog

    MetaLpnType iNodeHdrLpnCnts = info[(uint32_t)BackupInfo::InodeHdrSize];     // inode header
    MetaLpnType iNodeTableLpnCnts = info[(uint32_t)BackupInfo::InodeTableSize]; // inode table

    if (!catalogMgr_->RestoreContent(MetaVolumeType::SsdVolume, baseLpnInVol, lpnCnts))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_SAVE_FAILED,
            "Failed to restore NVRAM catalog from SSD meta volume");
        return false;
    }

    baseLpnInVol += lpnCnts;

    if (!inodeMgr_->RestoreContent(MetaVolumeType::SsdVolume, baseLpnInVol, iNodeHdrLpnCnts, iNodeTableLpnCnts))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_SAVE_FAILED,
            "Failed to restore NVRAM inode contents from SSD meta volume");
        return false;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NVRAM volume meta has successfully been restoreed from SSD volume!");

    return true;
}

uint32_t
MetaVolume::GetUtilizationInPercent(void)
{
    return ((maxVolumeLpn_ - inodeMgr_->GetAvailableLpnCount()) * 100) / maxVolumeLpn_;
}
} // namespace pos
