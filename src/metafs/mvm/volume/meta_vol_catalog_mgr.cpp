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

#include "meta_vol_catalog_mgr.h"

#include "mfs_io_config.h"
#include "mfs_log.h"
#include "mss.h"

MetaVolCatalogMgrClass::MetaVolCatalogMgrClass(void)
: catalog(nullptr)
{
}

MetaVolCatalogMgrClass::~MetaVolCatalogMgrClass(void)
{
    delete catalog;
}

void
MetaVolCatalogMgrClass::Init(MetaVolumeType volumeType, MetaLpnType baseLpn, MetaLpnType maxVolumeLpn)
{
    OnVolumeMetaRegionMgr::Init(volumeType, baseLpn, maxVolumeLpn);

    if (nullptr == catalog)
    {
        catalog = new MetaVolCatalog(volumeType, baseLpn);
    }
    else
    {
        MFS_TRACE_WARN((int)IBOF_EVENT_ID::MFS_WARNING_INIT_AGAIN,
            "catalog is already initialized.");
    }
}

void
MetaVolCatalogMgrClass::RegisterRegionInfo(MetaRegionType regionType, MetaLpnType baseLpn, MetaLpnType maxLpn)
{
    catalog->RegisterRegionInfo(regionType, baseLpn, maxLpn);
}

MetaLpnType
MetaVolCatalogMgrClass::GetRegionSizeInLpn(void)
{
    return _GetLpnCnt();
}

void
MetaVolCatalogMgrClass::Bringup(void)
{
}

void
MetaVolCatalogMgrClass::Finalize(void)
{
}

MetaLpnType
MetaVolCatalogMgrClass::_GetLpnCnt(void)
{
    return catalog->GetLpnCntOfRegion();
}

size_t
MetaVolCatalogMgrClass::_GetVolCatalogByteSize(void)
{
    return sizeof(MetaVolCatalogContent);
}

bool
MetaVolCatalogMgrClass::CheckVolumeValidity(void)
{
    return catalog->CheckValidity();
}

bool
MetaVolCatalogMgrClass::_CheckContentVality(void)
{
    bool isSuccess = true;

    // load is done but data doesn't exist somehow.
    if (false == CheckVolumeValidity())
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_VOLUME_CATALOG_INVALID,
            "Volume catalog is invalid. Try to recover broken volume catalog catalogs..");
        isSuccess = false;
    }

    return isSuccess;
}

bool
MetaVolCatalogMgrClass::LoadVolCatalog(void)
{
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Load volume catalog content...");
    bool isSuccess = catalog->Load();
    if (isSuccess)
    {
        isSuccess = _CheckContentVality();
    }
    else
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_LOAD_FAILED,
            "Load I/O for MFS catalog content has failed...");
    }

    return isSuccess;
}

bool
MetaVolCatalogMgrClass::RestoreContent(MetaVolumeType targetVol, MetaLpnType baseLpn, MetaLpnType lpnCnts)
{
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE, "[CatalogContent Restore] vol= {}", targetVol);

    MetaStorageType media = MetaFsUtilLib::ConvertToMediaType(targetVol);
    bool isSuccess = catalog->Load(media, baseLpn, 0 /* idx */, lpnCnts);
    if (isSuccess)
    {
        isSuccess = _CheckContentVality();
    }
    else
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_LOAD_FAILED,
            "Restore I/O for MFS catalog content has failed...");
    }

    return isSuccess;
}

bool
MetaVolCatalogMgrClass::CreateVolumeCatalog(MetaLpnType maxVolumeLpn, uint32_t maxFileSupportNum, bool save)
{
    bool isSuccess = true;

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
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
MetaVolCatalogMgrClass::SaveContent(void)
{
    assert(true == CheckVolumeValidity());

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Save volume catalog content...");

    if (true != catalog->Store())
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_SAVE_FAILED,
            "Store I/O for MFS catalog content has failed...");

        return false;
    }

    return true;
}

bool
MetaVolCatalogMgrClass::BackupContent(MetaVolumeType targetVol, MetaLpnType baseLpn, MetaLpnType lpnCnts)
{
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE, "[CatalogContent backup] vol= {}, baseLpn={}, LpnCnts={}",
        targetVol, baseLpn, lpnCnts);

    MetaStorageType media = MetaFsUtilLib::ConvertToMediaType(targetVol);
    bool isSuccess = catalog->Store(media, baseLpn, 0 /*buf idx*/, lpnCnts);
    if (isSuccess != true)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_META_SAVE_FAILED,
            "NVRAM meta backup has failed...");
    }

    return isSuccess;
}

bool
MetaVolCatalogMgrClass::_RecoverVolCatalog(void)
{
    // find valid slot and recover
    return true;
}
