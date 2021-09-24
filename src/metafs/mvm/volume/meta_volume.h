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

#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <utility>
#include <vector>
#include "catalog_manager.h"
#include "meta_volume_state.h"
#include "inode_manager.h"

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
    MetaVolume(void);
    MetaVolume(int arrayId, MetaVolumeType volumeType,
        MetaLpnType maxVolumePageNum = 0, InodeManager* inodeMgr = nullptr,
        CatalogManager* catalogMgr = nullptr);
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
    virtual bool CheckFileInActive(FileDescriptorType fd);
    virtual POS_EVENT_ID AddFileInActiveList(FileDescriptorType fd);
    virtual void RemoveFileFromActiveList(FileDescriptorType fd);

    // trim or dsm
    virtual bool TrimData(MetaFsFileControlRequest& reqMsg);

    // interfaces
    virtual MetaFileInode& GetInode(FileDescriptorType fd);
    virtual void GetInodeList(std::vector<MetaFileInfoDumpCxt>*& fileInfoList);
    virtual bool CopyInodeToInodeInfo(FileDescriptorType fd,
        MetaFileInodeInfo* inodeInfo /* output */);

    virtual uint32_t GetUtilizationInPercent(void);
    virtual size_t GetAvailableSpace(void);

    virtual bool IsGivenFileCreated(StringHashType fileKey);
    virtual FileSizeType GetFileSize(FileDescriptorType fd);
    virtual FileSizeType GetDataChunkSize(FileDescriptorType fd);
    virtual MetaLpnType GetFileBaseLpn(FileDescriptorType fd);

    virtual FileDescriptorType LookupDescriptorByName(std::string& fileName);
    virtual std::string LookupNameByDescriptor(FileDescriptorType fd);

    virtual MetaLpnType GetRegionSizeInLpn(MetaRegionType regionType);
    virtual MetaVolumeType GetVolumeType(void);

    virtual MetaLpnType GetTheLastValidLpn(void);

    virtual MetaLpnType
    GetBaseLpn(void)
    {
        return sumOfRegionBaseLpns;
    }

    virtual MetaLpnType
    GetMaxLpn(void)
    {
        return maxVolumeLpn;
    }

protected:
    MetaLpnType volumeBaseLpn;
    MetaLpnType maxVolumeLpn;
    MetaVolumeType volumeType;
    MetaVolumeState volumeState;
    static const uint32_t META_VOL_CAPACITY_FULL_LIMIT_IN_PERCENT = 99;

private:
    OnVolumeMetaRegionManager& _GetRegionMgr(MetaRegionManagerType region);
    void _RegisterRegionMgr(MetaRegionManagerType region, OnVolumeMetaRegionManager& mgr);
    void _BringupMgrs(void);
    void _FinalizeMgrs(void);
    void _SetupRegionInfoToRegionMgrs(MetaStorageSubsystem* metaStorage);
    bool _LoadVolumeMeta(MetaLpnType* info, bool isNPOR);

    bool _TrimData(MetaStorageType type, MetaLpnType start, MetaLpnType count);

    bool _BackupContents(MetaLpnType* info);
    bool _RestoreContents(MetaLpnType* info);

    std::unordered_map<MetaRegionManagerType, OnVolumeMetaRegionManager*, EnumTypeHash<MetaRegionManagerType>> regionMgrMap;
    InodeManager* inodeMgr;
    CatalogManager* catalogMgr;
    bool inUse;

    MetaLpnType sumOfRegionBaseLpns;
    MetaStorageSubsystem* metaStorage;

    std::unordered_map<FileDescriptorType, MetaVolumeType> fd2VolTypehMap;
    std::unordered_map<StringHashType, MetaVolumeType> fileKey2VolTypeMap;

    int arrayId;
};
} // namespace pos
