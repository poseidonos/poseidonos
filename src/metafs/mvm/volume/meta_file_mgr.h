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

#include "meta_vol_type.h"
#include "mf_extent_mgr.h"
#include "mf_lock_type.h"
#include "mf_pagemap.h"
#include "mfs_common.h"
#include "mss.h"
#include "on_volume_meta_region_mgr.h"

using FileFDSet = std::unordered_set<FileFDType>;

// meta file manager
// manage active files list, manage file lock, manages file extents in the meta volume
class MetaFileMgrClass : public OnVolumeMetaRegionMgr
{
public:
    MetaFileMgrClass(void);
    ~MetaFileMgrClass(void);

    virtual void Init(MetaVolumeType volumeType, MetaLpnType regionBaseLpn, MetaLpnType maxVolumeLpn) override;
    virtual MetaLpnType GetRegionSizeInLpn(void) override;
    virtual void Bringup(void) override;
    virtual void Finalize(void) override;
    virtual bool
    SaveContent(void) override
    { /*do nothing*/
        return true;
    }

    MetaFilePageMap AllocExtent(FileSizeType fileSize);
    void RemoveExtent(MetaLpnType baseLpn, FileSizeType fileSize);
    bool CheckFileInActive(FileFDType fd);
    IBOF_EVENT_ID AddFileInActiveList(FileFDType fd);
    void RemoveFileFromActiveList(FileFDType fd);
    FileFDType GetNextActiveFile(void);
    void GetExtentContent(MetaFileExtentContent* list);
    void SetExtentContent(MetaFileExtentContent* list);
    bool IsLockAlreadyGranted(FileFDType fd);
    void LockFile(FileFDType fd, MDFileLockTypeEnum lock);
    void UnlockFile(FileFDType fd);
    uint32_t GetUtilizationInPercent(void);
    const FileFDSet& GetFDSetOfActiveFiles(void);
    size_t GetTheBiggestExtentSize(void);
    bool IsAllocated(void);

    MetaLpnType
    GetFileBaseLpn(void)
    {
        return extentMgr.GetFileBaseLpn();
    }
    void
    SetFileBaseLpn(MetaLpnType BaseLpn)
    {
        extentMgr.SetFileBaseLpn(BaseLpn);
    }
    bool TrimData(MetaStorageType media, MetaLpnType startLpn, MetaLpnType numTrimLpns);

private:
    FileFDSet activeFiles;
    std::unordered_map<FileFDType, MDFileLockTypeEnum> flock; // file lock
    bool isAllocated;

    MetaFileExtentMgrClass extentMgr;
    MetaStorageSubsystem* mfssIntf;
};
