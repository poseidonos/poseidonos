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

#include "meta_file_mgr.h"
#include "meta_region_mgr_type.h"
#include "meta_vol_catalog_mgr.h"
#include "meta_vol_state.h"
#include "mf_inode_mgr.h"

class MetaVolumeClass
{
public:
    explicit MetaVolumeClass(MetaVolumeType volumeType, MetaLpnType maxVolumePageNum = 0);
    virtual ~MetaVolumeClass(void);

    virtual void InitVolumeBaseLpn(void) = 0;
    virtual bool IsOkayToStore(FileSizeType fileByteSize, MetaFilePropertySet& prop) = 0;

    void Init(void);
    bool OpenVolume(MetaLpnType* info, bool isNPOR);
    bool CloseVolume(MetaLpnType* info, bool& resetContext /*output*/);
    bool CreateNewVolume(void);
    bool IsVolumeOpened(void);
#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    bool Compaction(void);
#endif
    void BuildFreeFDMap(std::map<FileFDType, FileFDType>& dstFreeFDMap);
    void BuildFDMap(std::unordered_map<FileFDType, MetaVolumeType>& targetMap);
    void BuildFileNameMap(std::unordered_map<StringHashType, MetaVolumeType>& targetMap);
    void BuildFDLookupMap(std::unordered_map<StringHashType, FileFDType>& targetMap);
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

    MetaVolCatalogMgrClass&
    GetCatalogInstance(void)
    {
        return catalogMgr;
    }

    MetaFileInodeMgrClass&
    GetInodeInstance(void)
    {
        return inodeMgr;
    }

    MetaFileMgrClass&
    GetFileInstance(void)
    {
        return fileMgr;
    }

protected:
    MetaLpnType volumeBaseLpn;
    MetaLpnType maxVolumeLpn;
    MetaVolumeType volumeType;
    MetaVolumeState volumeState;
    static const uint32_t META_VOL_CAPACITY_FULL_LIMIT_IN_PERCENT = 99;

private:
    friend class MetaVolMgrClass;

    OnVolumeMetaRegionMgr& _GetRegionMgr(VolMetaRegionMgrType region);
    void _RegisterRegionMgr(VolMetaRegionMgrType region, OnVolumeMetaRegionMgr& mgr);
    void _BringupMgrs(void);
    void _FinalizeMgrs(void);
    void _SetupRegionInfoToRegionMgrs(void);
    bool _LoadAllVolumeMeta(MetaLpnType* info, bool isNPOR);
    MetaStorageType _GetVolStorageType(void);

    std::unordered_map<VolMetaRegionMgrType, OnVolumeMetaRegionMgr*, EnumTypeHash<VolMetaRegionMgrType>> regionMgrMap;
    MetaFileMgrClass fileMgr;
    MetaFileInodeMgrClass inodeMgr;
    MetaVolCatalogMgrClass catalogMgr;
    bool inUse;

    MetaLpnType sumOfRegionBaseLpns;
    bool _BackupContents(MetaLpnType* info);
    bool _RestoreContents(MetaLpnType* info);
};
