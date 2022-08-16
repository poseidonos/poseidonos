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

#include "catalog_manager.h"

#include "metafs_config.h"
#include "metafs_log.h"
#include "src/metafs/storage/mss.h"

#include <string>

namespace pos
{
CatalogManager::CatalogManager(int arrayId)
: OnVolumeMetaRegionManager(arrayId),
  catalog(nullptr)
{
}

CatalogManager::CatalogManager(Catalog* catalog, int arrayId)
: CatalogManager(arrayId)
{
    this->catalog = catalog;
}

CatalogManager::~CatalogManager(void)
{
    delete catalog;
}

void
CatalogManager::Init(MetaVolumeType volumeType, MetaLpnType baseLpn, MetaLpnType maxVolumeLpn)
{
    OnVolumeMetaRegionManager::Init(volumeType, baseLpn, maxVolumeLpn);

    if (nullptr == catalog)
    {
        catalog = new Catalog(volumeType, baseLpn);
    }
    else
    {
        MFS_TRACE_WARN(EID(MFS_WARNING_INIT_AGAIN),
            "catalog is already initialized.");
    }
}

void
CatalogManager::RegisterRegionInfo(MetaRegionType regionType, MetaLpnType baseLpn, MetaLpnType maxLpn)
{
    catalog->RegisterRegionInfo(regionType, baseLpn, maxLpn);
}

MetaLpnType
CatalogManager::GetRegionSizeInLpn(void)
{
    return catalog->GetLpnCntOfRegion();
}

void
CatalogManager::Bringup(void)
{
}

void
CatalogManager::Finalize(void)
{
}

void
CatalogManager::SetMss(MetaStorageSubsystem* metaStorage)
{
    catalog->SetMss(metaStorage);
}

bool
CatalogManager::LoadVolCatalog(void)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Load volume catalog content...");

    if (true == catalog->Load())
    {
        return catalog->CheckValidity();
    }
    else
    {
        MFS_TRACE_ERROR(EID(MFS_META_LOAD_FAILED),
            "Load I/O for MFS catalog content has failed...");
    }

    return false;
}

bool
CatalogManager::RestoreContent(MetaVolumeType targetVol, MetaLpnType baseLpn, MetaLpnType lpnCnts)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE), "[CatalogContent Restore] vol= {}", targetVol);

    MetaStorageType media = MetaFileUtil::ConvertToMediaType(targetVol);
    bool isSuccess = catalog->Load(media, baseLpn, 0 /* idx */, lpnCnts);
    if (isSuccess)
    {
        isSuccess = catalog->CheckValidity();
    }
    else
    {
        MFS_TRACE_ERROR(EID(MFS_META_LOAD_FAILED),
            "Restore I/O for MFS catalog content has failed...");
    }

    return isSuccess;
}

bool
CatalogManager::CreateCatalog(MetaLpnType maxVolumeLpn, uint32_t maxFileSupportNum, bool save)
{
    bool isSuccess = true;

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Create volume catalog content...");
    catalog->Create(maxVolumeLpn, maxFileSupportNum);

    RegisterRegionInfo(MetaRegionType::VolCatalog,
        catalog->GetBaseLpn(),
        catalog->GetLpnCntOfRegion());

    if (save)
    {
        isSuccess = SaveContent();
    }

    return isSuccess;
}

bool
CatalogManager::SaveContent(void)
{
    assert(true == catalog->CheckValidity());

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Save volume catalog content...");

    if (true != catalog->Store())
    {
        MFS_TRACE_ERROR(EID(MFS_META_SAVE_FAILED),
            "Store I/O for MFS catalog content has failed...");

        return false;
    }

    return true;
}

bool
CatalogManager::BackupContent(MetaVolumeType targetVol, MetaLpnType baseLpn, MetaLpnType lpnCnts)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE), "[CatalogContent backup] vol= {}, baseLpn={}, LpnCnts={}",
        targetVol, baseLpn, lpnCnts);

    MetaStorageType media = MetaFileUtil::ConvertToMediaType(targetVol);
    bool isSuccess = catalog->Store(media, baseLpn, 0 /*buf idx*/, lpnCnts);
    if (isSuccess != true)
    {
        MFS_TRACE_ERROR(EID(MFS_META_SAVE_FAILED),
            "NVRAM meta backup has failed...");
    }

    return isSuccess;
}
} // namespace pos
