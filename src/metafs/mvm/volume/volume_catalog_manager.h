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

#include "volume_catalog.h"
#include "on_volume_meta_region_mgr.h"

#include <string>

namespace pos
{
class VolumeCatalogManager : public OnVolumeMetaRegionManager
{
public:
    explicit VolumeCatalogManager(int arrayId);
    ~VolumeCatalogManager(void);

    virtual void Init(MetaVolumeType volumeType, MetaLpnType regionBaseLpn, MetaLpnType maxVolumeLpn) override;
    virtual MetaLpnType GetRegionSizeInLpn(void) override;
    virtual void Bringup(void) override;
    virtual bool SaveContent(void) override;
    virtual void Finalize(void) override;
    virtual void SetMss(MetaStorageSubsystem* metaStorage);

    bool LoadVolCatalog(void);
    bool CheckVolumeValidity(void);
    bool CreateVolumeCatalog(MetaLpnType maxVolumeLpn, uint32_t maxFileSupportNum, bool save = true);
    void RegisterRegionInfo(MetaRegionType regionType, MetaLpnType baseLpn, MetaLpnType maxLpn);

    bool BackupContent(MetaVolumeType tgtVol, MetaLpnType baseLpn, MetaLpnType lpnCnts);
    bool RestoreContent(MetaVolumeType tgtVol, MetaLpnType baseLpn, MetaLpnType lpnCnts);

private:
    MetaLpnType _GetLpnCnt(void);
    size_t _GetVolCatalogByteSize(void);
    MetaLpnType _GetBaseLpnOfVolCatalogSlot(void);
    bool _RecoverVolCatalog(void);

    static const uint32_t TOTAL_VOL_CATALOG_SLOT_CNT = 1 + 3; // 1 + 3 mirrors

    bool _CheckContentVality(void);
    VolumeCatalog* catalog;
};
} // namespace pos
