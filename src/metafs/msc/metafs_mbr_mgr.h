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

#include <array>
#include <string>
#include "metafs_mbr.h"
#include "meta_storage_info.h"
#include "msc_req.h"
#include "meta_region_mgr.h"
#include "src/lib/bitmap.h"

namespace pos
{
class MetaFsMBRManager : public MetaRegionManager
{
public:
    MetaFsMBRManager(void);
    ~MetaFsMBRManager(void);

    virtual void Init(std::string arrayName, MetaStorageType mediaType, MetaLpnType baseLpn, MetaLpnType maxLpn = 0) override;
    virtual void Bringup(void) override;
    virtual bool SaveContent(void) override;
    virtual MetaLpnType GetRegionSizeInLpn(void) override;
    virtual void Finalize(void) override;

    bool IsValidMBRExist(void);
    uint64_t GetEpochSignature(void);
    bool LoadMBR(std::string arrayName);
    void BuildMBR(void);
    bool CreateMBR(std::string arrayName);
    void RegisterVolumeGeometry(MetaStorageInfo& mediaInfo);
    MetaFsStorageIoInfoList& GetAllStoragePartitionInfo(void);

    static const uint32_t FILESYSTEM_MBR_BASE_LPN = 0;

    void SetPowerStatus(bool isShutDownOff);
    bool GetPowerStatus(void);
    void InvalidMBR(void);

private:
    MetaFsMBR* mbr;
};

class MetaVolumeMbrMap
{
public:
    MetaVolumeMbrMap(void);
    ~MetaVolumeMbrMap(void);

    void Init(std::string& arrayName, MetaStorageType mediaType, MetaLpnType baseLpn, MetaLpnType maxLpn = 0);
    void Remove(std::string& arrayName);
    void RegisterVolumeGeometry(std::string& arrayName, MetaStorageInfo& mediaInfo);
    bool IsValidMBRExist(std::string& arrayName);
    MetaFsStorageIoInfoList& GetAllStoragePartitionInfo(std::string& arrayName);
    MetaLpnType GetRegionSizeInLpn(std::string& arrayName);
    bool CreateMBR(std::string& arrayName);
    bool LoadMBR(std::string& arrayName);
    void InvalidMBR(std::string& arrayName);
    uint64_t GetEpochSignature(std::string& arrayName);
    void SetPowerStatus(std::string& arrayName, bool isNPOR);
    bool GetPowerStatus(std::string& arrayName);
    bool SaveContent(std::string& arrayName);
    bool IsMbrLoaded(std::string& arrayName);
    uint32_t GetMountedMbrCount(void);

private:
    BitMap* mbrBitmap;
    std::unordered_map<std::string, uint32_t> mbrMap;
    MetaFsMBRManager* mbrList[MetaFsConfig::MAX_ARRAY_CNT];
};

// extern MetaFsMBRManager metafsMBRMgr;
} // namespace pos
