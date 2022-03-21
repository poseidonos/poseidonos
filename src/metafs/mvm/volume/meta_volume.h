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

#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "catalog_manager.h"
#include "inode_manager.h"
#include "meta_volume_state.h"
#include "src/metafs/mvm/volume/inode_creator.h"
#include "src/metafs/mvm/volume/inode_deleter.h"

namespace pos
{
enum class MetaRegionManagerType
{
    First = 0,
    VolCatalogMgr = First,
    InodeMgr,
    Last = InodeMgr,

    Max,
};

using FileControlResult = std::pair<FileDescriptorType, POS_EVENT_ID>;

class MetaVolume
{
public:
    MetaVolume(void) = delete;
    MetaVolume(const int arrayId, const MetaVolumeType volumeType,
        const MetaLpnType maxVolumePageNum = 0, InodeManager* inodeMgr = nullptr,
        CatalogManager* catalogMgr = nullptr, InodeCreator* inodeCreator = nullptr,
        InodeDeleter* inodeDeleter = nullptr);
    virtual ~MetaVolume(void);

    virtual void InitVolumeBaseLpn(void) = 0;
    virtual bool IsOkayToStore(FileSizeType fileByteSize, MetaFilePropertySet& prop) = 0;

    virtual void Init(MetaStorageSubsystem* metaStorage);

    // control meta volume
    virtual bool CreateVolume(void);
    virtual bool OpenVolume(MetaLpnType* info, bool isNPOR);
    virtual bool CloseVolume(MetaLpnType* info, bool& resetContext /*output*/);

    // control files
    virtual FileControlResult CreateFile(MetaFsFileControlRequest& reqMsg);
    virtual FileControlResult DeleteFile(MetaFsFileControlRequest& reqMsg);
    virtual bool TrimData(MetaFsFileControlRequest& reqMsg);

    // interfaces
    virtual MetaLpnType GetRegionSizeInLpn(MetaRegionType regionType);
    virtual uint32_t GetUtilizationInPercent(void);
    virtual void GetInodeList(std::vector<MetaFileInfoDumpCxt>*& fileInfoList) const;
    virtual bool CopyInodeToInodeInfo(const FileDescriptorType fd,
        MetaFileInodeInfo* inodeInfo /* output */);
    virtual bool CheckFileInActive(const FileDescriptorType fd) const
    {
        return inodeMgr_->CheckFileInActive(fd);
    }
    virtual POS_EVENT_ID AddFileInActiveList(const FileDescriptorType fd) const
    {
        return inodeMgr_->AddFileInActiveList(fd);
    }
    virtual void RemoveFileFromActiveList(const FileDescriptorType fd) const
    {
        inodeMgr_->RemoveFileFromActiveList(fd);
    }
    virtual MetaFileInode& GetInode(const FileDescriptorType fd) const
    {
        return inodeMgr_->GetFileInode(fd);
    }
    virtual size_t GetAvailableSpace(void) const
    {
        return inodeMgr_->GetAvailableSpace();
    }
    virtual bool IsGivenFileCreated(const StringHashType fileKey) const
    {
        return inodeMgr_->IsGivenFileCreated(fileKey);
    }
    virtual FileSizeType GetFileSize(const FileDescriptorType fd) const
    {
        return inodeMgr_->GetFileSize(fd);
    }
    virtual FileSizeType GetDataChunkSize(const FileDescriptorType fd) const
    {
        return inodeMgr_->GetDataChunkSize(fd);
    }
    virtual MetaLpnType GetFileBaseLpn(const FileDescriptorType fd) const
    {
        return inodeMgr_->GetFileBaseLpn(fd);
    }
    virtual FileDescriptorType LookupDescriptorByName(const std::string& fileName) const
    {
        return inodeMgr_->LookupDescriptorByName(fileName);
    }
    virtual std::string LookupNameByDescriptor(const FileDescriptorType fd) const
    {
        return inodeMgr_->LookupNameByDescriptor(fd);
    }
    virtual MetaLpnType GetTheLastValidLpn(void) const
    {
        return inodeMgr_->GetTheLastValidLpn();
    }
    virtual MetaVolumeType GetVolumeType(void) const
    {
        return volumeType_;
    }
    virtual MetaLpnType GetBaseLpn(void) const
    {
        return sumOfRegionBaseLpns_;
    }
    virtual MetaLpnType GetMaxLpn(void) const
    {
        return maxVolumeLpn_;
    }

protected:
    static const uint32_t META_VOL_CAPACITY_FULL_LIMIT_IN_PERCENT = 99;
    MetaLpnType volumeBaseLpn_ = 0;
    MetaLpnType maxVolumeLpn_ = 0;
    MetaVolumeType volumeType_ = MetaVolumeType::Max;
    MetaVolumeState volumeState_ = MetaVolumeState::Default;

private:
    OnVolumeMetaRegionManager& _GetRegionMgr(MetaRegionManagerType region);
    void _SetupRegionInfoToRegionMgrs(MetaStorageSubsystem* metaStorage);
    bool _LoadVolumeMeta(MetaLpnType* info, bool isNPOR);

    bool _TrimData(MetaStorageType type, MetaLpnType start, MetaLpnType count);

    bool _BackupContents(MetaLpnType* info);
    bool _RestoreContents(MetaLpnType* info);

    std::unordered_map<MetaRegionManagerType, OnVolumeMetaRegionManager*, EnumTypeHash<MetaRegionManagerType>> regionMgrMap_;
    InodeManager* inodeMgr_;
    CatalogManager* catalogMgr_;

    MetaLpnType sumOfRegionBaseLpns_;
    MetaStorageSubsystem* metaStorage_;

    std::unordered_map<FileDescriptorType, MetaVolumeType> fd2VolTypehMap_;
    std::unordered_map<StringHashType, MetaVolumeType> fileKey2VolTypeMap_;

    int arrayId_;
    void* trimBuffer_;

    InodeCreator* inodeCreator_;
    InodeDeleter* inodeDeleter_;
};
} // namespace pos
