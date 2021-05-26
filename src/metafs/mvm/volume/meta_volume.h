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
#include <string>
#include "meta_file_manager.h"
#include "volume_catalog_manager.h"
#include "meta_volume_state.h"
#include "mf_inode_mgr.h"

namespace pos
{
enum class MetaRegionManagerType
{
    First = 0,
    VolCatalogMgr = First,
    InodeMgr,
    FileMgr,
    Last = FileMgr,

    Max,
};

class MetaVolume
{
public:
    explicit MetaVolume(std::string arrayName, MetaVolumeType volumeType, MetaLpnType maxVolumePageNum = 0);
    explicit MetaVolume(MetaFileManager* fileMgr, MetaFileInodeManager* inodeMgr,
            VolumeCatalogManager* catalogMgr, std::string arrayName,
            MetaVolumeType volumeType, MetaLpnType maxVolumePageNum = 0);
    virtual ~MetaVolume(void);

    virtual void InitVolumeBaseLpn(void) = 0;
    virtual bool IsOkayToStore(FileSizeType fileByteSize, MetaFilePropertySet& prop) = 0;

    void Init(MetaStorageSubsystem* metaStorage);
    bool OpenVolume(MetaLpnType* info, bool isNPOR);
    bool CloseVolume(MetaLpnType* info, bool& resetContext /*output*/);
    bool CreateNewVolume(void);
    bool IsVolumeOpened(void);
#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    bool Compaction(void);
#endif
    void BuildFreeFDMap(std::map<FileDescriptorType, FileDescriptorType>& dstFreeFDMap);
    void BuildFDMap(std::unordered_map<FileDescriptorType, MetaVolumeType>& targetMap);
    void BuildFileNameMap(std::unordered_map<StringHashType, MetaVolumeType>& targetMap);
    void BuildFDLookupMap(std::unordered_map<StringHashType, FileDescriptorType>& targetMap);
    MetaLpnType GetVolumeBaseLpn(void);
    uint32_t GetUtilizationInPercent(void);
    MetaVolumeType GetVolumeType(void);
    size_t GetTheBiggestExtentSize(void);

    bool CopyExtentContent(void);

    const MetaLpnType
    GetBaseLpn(void)
    {
        return sumOfRegionBaseLpns;
    }

    const MetaLpnType
    GetMaxLpn(void)
    {
        return maxVolumeLpn;
    }

    VolumeCatalogManager&
    GetCatalogInstance(void)
    {
        return *catalogMgr;
    }

    MetaFileInodeManager&
    GetInodeInstance(void)
    {
        return *inodeMgr;
    }

    MetaFileManager&
    GetFileInstance(void)
    {
        return *fileMgr;
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
    bool _LoadAllVolumeMeta(MetaLpnType* info, bool isNPOR);
    MetaStorageType _GetVolStorageType(void);

    std::unordered_map<MetaRegionManagerType, OnVolumeMetaRegionManager*, EnumTypeHash<MetaRegionManagerType>> regionMgrMap;
    MetaFileManager* fileMgr;
    MetaFileInodeManager* inodeMgr;
    VolumeCatalogManager* catalogMgr;
    bool inUse;

    MetaLpnType sumOfRegionBaseLpns;
    bool _BackupContents(MetaLpnType* info);
    bool _RestoreContents(MetaLpnType* info);

    std::string arrayName;
};
} // namespace pos
