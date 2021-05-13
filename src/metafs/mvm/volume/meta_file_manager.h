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

#include "meta_volume_type.h"
#include "mf_extent_mgr.h"
#include "mf_lock_type.h"
#include "mf_pagemap.h"
#include "metafs_common.h"
#include "src/metafs/storage/mss.h"
#include "on_volume_meta_region_mgr.h"

#include <string>

namespace pos
{
using FileDescriptorSet = std::unordered_set<FileDescriptorType>;

// meta file manager
// manage active files list, manage file lock, manages file extents in the meta volume
class MetaFileManager : public OnVolumeMetaRegionManager
{
public:
    explicit MetaFileManager(std::string arrayName);
    ~MetaFileManager(void);

    virtual void Init(MetaVolumeType volumeType, MetaLpnType regionBaseLpn, MetaLpnType maxVolumeLpn) override;
    virtual MetaLpnType GetRegionSizeInLpn(void) override;
    virtual void Bringup(void) override;
    virtual void Finalize(void) override;
    virtual void SetMss(MetaStorageSubsystem* metaStorage);
    virtual bool
    SaveContent(void) override
    { /*do nothing*/
        return true;
    }

    MetaFilePageMap AllocExtent(FileSizeType fileSize);
    void RemoveExtent(MetaLpnType baseLpn, FileSizeType fileSize);
    bool CheckFileInActive(FileDescriptorType fd);
    POS_EVENT_ID AddFileInActiveList(FileDescriptorType fd);
    void RemoveFileFromActiveList(FileDescriptorType fd);
    FileDescriptorType GetNextActiveFile(void);
    void GetExtentContent(MetaFileExtent* list);
    void SetExtentContent(MetaFileExtent* list);
    uint32_t GetUtilizationInPercent(void);
    const FileDescriptorSet& GetFDSetOfActiveFiles(void);
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
    FileDescriptorSet activeFiles;
    bool isAllocated;

    MetaFileExtentManager extentMgr;
    MetaStorageSubsystem* mfssIntf;
};
} // namespace pos
