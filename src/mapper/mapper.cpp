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

#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

#include "src/mapper/mapper.h"
#include "src/mapper/map_flushed_event.h"
#include "src/mapper/reversemap/reverse_map.h"
#include "src/mapper_service/mapper_service.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/sys_event/volume_event.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"

namespace pos
{
MpageList IMapFlush::DEFAULT_DIRTYPAGE_SET;

Mapper::Mapper(TelemetryClient* tc_, TelemetryPublisher* tp_, MapperWbt* mapperWbt_, VSAMapManager* vsaMapMan_, StripeMapManager* stripeMan_, ReverseMapManager* revMapMan_, MapperAddressInfo* addrInfo_, IArrayInfo* iarrayInfo, MetaFs* metaFs_)
: addrInfo(addrInfo_),
  vsaMapManager(vsaMapMan_),
  stripeMapManager(stripeMan_),
  reverseMapManager(revMapMan_),
  mapperWbt(mapperWbt_),
  metaFs(metaFs_),
  tp(tp_),
  tc(tc_),
  isInitialized(false),
  numMapLoadedVol(0),
  numMountedVol(0)
{
    arrayName = iarrayInfo->GetName();
    if (addrInfo == nullptr)
    {
        addrInfo = new MapperAddressInfo(iarrayInfo);
    }
    if (tp == nullptr)
    {
        tp = new TelemetryPublisher(("Mapper"));
        tp->AddDefaultLabel("array_name", arrayName);
    }
    if (vsaMapManager == nullptr)
    {
        vsaMapManager = new VSAMapManager(tp, addrInfo);
    }
    if (stripeMapManager == nullptr)
    {
        stripeMapManager = new StripeMapManager(tp, nullptr, addrInfo);
    }
    if (reverseMapManager == nullptr)
    {
        reverseMapManager = new ReverseMapManager(vsaMapManager, stripeMapManager, nullptr, addrInfo);
    }
    if (mapperWbt == nullptr)
    {
        mapperWbt = new MapperWbt(addrInfo, vsaMapManager, stripeMapManager, reverseMapManager);
    }
    _ClearVolumeState();
}

Mapper::Mapper(IArrayInfo* iarrayInfo, MetaFs* metaFs_)
: Mapper(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, iarrayInfo, metaFs_)
{
    tc = TelemetryClientSingleton::Instance();
}

// LCOV_EXCL_START
Mapper::~Mapper(void)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper Destructor] in Array:{} was Destroyed", arrayName);
    _Dispose();
    if (mapperWbt != nullptr)
    {
        delete mapperWbt;
        mapperWbt = nullptr;
    }
    if (reverseMapManager != nullptr)
    {
        delete reverseMapManager;
        reverseMapManager = nullptr;
    }
    if (stripeMapManager != nullptr)
    {
        delete stripeMapManager;
        stripeMapManager = nullptr;
    }
    if (vsaMapManager != nullptr)
    {
        delete vsaMapManager;
        vsaMapManager = nullptr;
    }
    if (tp != nullptr)
    {
        delete tp;
        tp = nullptr;
    }
    if (addrInfo != nullptr)
    {
        delete addrInfo;
        addrInfo = nullptr;
    }
}
// LCOV_EXCL_STOP

void
Mapper::Dispose(void)
{
    if (isInitialized == true)
    {
        POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper Dispose] MAPPER Disposed, init:{} array:{}", isInitialized, arrayName);
        StoreAll();
        _Dispose();
    }
}

void
Mapper::Shutdown(void)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper Shutdown] MAPPER Shutdown, init:{} array:{}", isInitialized, arrayName);
    _Dispose();
}

int
Mapper::Init(void)
{
    int ret = 0;
    if (isInitialized == false)
    {
        POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper Init] array:{}", arrayName);
        if (metaFs == nullptr)
        {
            metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(addrInfo->GetArrayId());
        }
        if ((tc != nullptr) && (tp != nullptr))
        {
            tc->RegisterPublisher(tp);
        }
        int mpageSize = _GetMpageSize();
        addrInfo->SetupAddressInfo(mpageSize);
        POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper Init] VsaMap Init, array:{}", arrayName);
        ret = vsaMapManager->Init();
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(VSAMAP_NOT_LOADED), "[Mapper Init] Failed to Initialize VSAMapManager, array:{}", arrayName);
            assert(false);
            return ret;
        }
        vsaMapManager->WaitAllPendingIoDone();
        POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper Init] StripeMap Init, array:{}", arrayName);
        ret = stripeMapManager->Init();
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(VSAMAP_NOT_LOADED), "[Mapper Init] Failed to Initialize StripeMap, array:{}", arrayName);
            assert(false);
            return ret;
        }
        stripeMapManager->WaitAllPendingIoDone();
        POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper Init] ReverseMap Init, array:{}", arrayName);
        reverseMapManager->Init();
        _RegisterToMapperService();
        isInitialized = true;
    }
    assert(ret == 0);
    return ret;
}

void
Mapper::Flush(void)
{
    // no-op for IMountSequence
}

int
Mapper::FlushDirtyMpages(int volId, EventSmartPtr event)
{
    if (volId == STRIPE_MAP_ID)
    {
        return stripeMapManager->FlushTouchedPages(event);
    }

    assert(volId < MAX_VOLUME_COUNT);
    VolState state = volState[volId].GetState();
    if ((state != BACKGROUND_MOUNTED) && (state != FOREGROUND_MOUNTED))
    {
        return 0;
    }
    return vsaMapManager->FlushTouchedPages(volId, event);
}

int
Mapper::FlushDirtyMpagesGiven(int volId, EventSmartPtr event, MpageList dirtyPages)
{
    if (volId == STRIPE_MAP_ID)
    {
        return stripeMapManager->FlushDirtyPagesGiven(dirtyPages, event);
    }

    assert(volId < MAX_VOLUME_COUNT);
    VolState state = volState[volId].GetState();
    if ((state != BACKGROUND_MOUNTED) && (state != FOREGROUND_MOUNTED))
    {
        return 0;
    }
    return vsaMapManager->FlushDirtyPagesGiven(volId, dirtyPages, event);
}

int
Mapper::StoreAll(void)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper StoreAll] Flush All Synchronously, array:{}", arrayName);
    stripeMapManager->WaitAllPendingIoDone();
    vsaMapManager->WaitAllPendingIoDone();

    int ret = stripeMapManager->FlushTouchedPages(nullptr);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper StoreAll] Failed To Flush All StripeMap, array:{}", arrayName);
        return ret;
    }
    ret = vsaMapManager->FlushAllMaps();
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper StoreAll] Failed To Flush All VSAMap, array:{}", arrayName);
        return ret;
    }
    stripeMapManager->WaitAllPendingIoDone();
    vsaMapManager->WaitAllPendingIoDone();
    return ret;
}

int
Mapper::EnableInternalAccess(int volId)
{
    VolState state = volState[volId].GetState();
    if ((VolState::VOLUME_DELETING == state) || (VolState::NOT_EXIST == state))
    {
        return -EID(VSAMAP_LOAD_FAILURE);
    }
    if (VolState::EXIST_UNLOADED == state)
    {
        if (_LoadVolumeMeta(volId) < 0)
        {
            POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "[Mapper EnableInternalAccess] failed to load VolumeId:{}, state:{} array:{}", volId, volState[volId].GetState(), arrayName);
            return -EID(VSAMAP_LOAD_FAILURE);
        }
        else
        {
            POS_TRACE_INFO(EID(VSAMAP_LOAD_FAILURE), "[Mapper EnableInternalAccess] Load VolumeId:{}, state:{} array:{} Issued (ret:NEED_RETRY)", volId, volState[volId].GetState(), arrayName);
            return NEED_RETRY;
        }
    }
    else if (VolState::VOLUME_LOADING == state)
    {
        if (vsaMapManager->IsVolumeLoaded(volId) == true)
        {
            std::unique_lock<std::mutex> lock(volState[volId].GetVolStateLock());
            state = volState[volId].GetState();
            if (state != VolState::FOREGROUND_MOUNTED)
            {
                volState[volId].SetState(VolState::BACKGROUND_MOUNTED);
                POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper EnableInternalAccess] VolumeId:{} was loaded LOADING >> BG_MOUNTED, arrayName:{} @VolumeMounted", volId, arrayName);
            }
        }
        else
        {
            return NEED_RETRY;
        }
    }
    return 0;
}

VirtualBlkAddr
Mapper::GetVSAInternal(int volId, BlkAddr rba, int& retry)
{
    retry = OK_READY;
    int ret = EnableInternalAccess(volId);
    if (ret == NEED_RETRY)
    {
        retry = NEED_RETRY;
        return UNMAP_VSA;
    }
    else if (ret < 0)
    {
        return UNMAP_VSA;
    }
    else
    {
        return vsaMapManager->GetVSAWoCond(volId, rba);
    }
}

int
Mapper::SetVSAsInternal(int volId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    int ret = EnableInternalAccess(volId);
    if ((ret < 0) || (ret == NEED_RETRY))
    {
        // there is no NEED_RETY Handling sequence for SetVSAsInternal in caller(GC).
        POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "[Mapper SetVSAInternal] VolumeId:{} array:{} volume is not accessible", volId, arrayName);
        return -EID(VSAMAP_LOAD_FAILURE);
    }
    return vsaMapManager->SetVSAsWoCond(volId, startRba, virtualBlks);
}

VirtualBlkAddr
Mapper::GetVSAWithSyncOpen(int volId, BlkAddr rba)
{
    int ret = EnableInternalAccess(volId);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "[Mapper GetVSAWithSyncOpen] Failed to Load VolumeId:{} array:{}", volId, arrayName);
        return UNMAP_VSA;
    }
    if (ret == NEED_RETRY)
    {
        vsaMapManager->WaitVolumePendingIoDone(volId);
    }
    return vsaMapManager->GetVSAWoCond(volId, rba);
}

int
Mapper::SetVSAsWithSyncOpen(int volId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    int ret = EnableInternalAccess(volId);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "[Mapper SetVSAsWithSyncOpen] Failed to Load VolumeId:{} array:{}", volId, arrayName);
        return ret;
    }
    if (ret == NEED_RETRY)
    {
        vsaMapManager->WaitVolumePendingIoDone(volId);
    }
    return vsaMapManager->SetVSAsWoCond(volId, startRba, virtualBlks);
}

MpageList
Mapper::GetDirtyVsaMapPages(int volId, BlkAddr startRba, uint64_t numBlks)
{
    return vsaMapManager->GetDirtyVsaMapPages(volId, startRba, numBlks);
}

int
Mapper::GetVSAs(int volId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray)
{
    return vsaMapManager->GetVSAs(volId, startRba, numBlks, vsaArray);
}

int
Mapper::SetVSAs(int volId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    return vsaMapManager->SetVSAs(volId, startRba, virtualBlks);
}

VirtualBlkAddr
Mapper::GetRandomVSA(BlkAddr rba)
{
    return vsaMapManager->GetRandomVSA(rba);
}

int64_t
Mapper::GetNumUsedBlks(int volId)
{
    VolState state = volState[volId].GetState();
    if (state != VolState::NOT_EXIST)
    {
        int ret = EnableInternalAccess(volId);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "[Mapper GetNumUsedBlks] Failed to Load VolumeId:{} array:{}", volId, arrayName);
            return -1;
        }
        if (ret == NEED_RETRY)
        {
            vsaMapManager->WaitVolumePendingIoDone(volId);
        }
        return vsaMapManager->GetNumUsedBlks(volId);
    }
    else
    {
        // if volume doesn't exist, there is no used block.
        return 0;
    }
}

int
Mapper::VolumeCreated(int volId, uint64_t volSizeByte)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VolumeCreate] CREATE_VOLUME Volume:{}, size:{} array:{}", volId, volSizeByte, arrayName);
    assert(volSizeByte > 0);
    VolState state = volState[volId].GetState();
    if (state != VolState::NOT_EXIST)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VolumeCreate] Volume:{} array:{} already exist, state:{}", volId, addrInfo->GetArrayName(), volState[volId].GetState());
        return (int)POS_EVENT_ID::VOL_EVENT_FAIL;
    }
    if (vsaMapManager->CreateVsaMapContent(nullptr, volId, volSizeByte, false) != 0)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VolumeCreate] failed to create vsaMap VolumeId:{} array:{}", volId, addrInfo->GetArrayName());
        return (int)POS_EVENT_ID::VOL_EVENT_FAIL;
    }
    vsaMapManager->WaitVolumePendingIoDone(volId);
    volState[volId].SetState(VolState::BACKGROUND_MOUNTED);
    volState[volId].SetSize(volSizeByte);
    vsaMapManager->EnableVsaMapInternalAccess(volId);
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeCreate] VolumeId:{} array:{} JUST_CREATED >> BG_MOUNTED", volId, arrayName);
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

int
Mapper::VolumeMounted(int volId, uint64_t volSizeByte)
{
    VolState state = volState[volId].GetState();
    if ((VolState::VOLUME_DELETING == state) || (state == VolState::NOT_EXIST))
    {
        POS_TRACE_ERROR(EID(MAPPER_SUCCESS), "[Mapper VolumeMount] failed to mount VolumeId:{} array:{}, state:{}", volId, arrayName, state);
        return (int)POS_EVENT_ID::VOL_EVENT_FAIL;
    }
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VolumeMount] MOUNT_VOLUME Volume:{}, state:{}, array:{}", volId, state, arrayName);
    if (_LoadVolumeMeta(volId) < 0)
    {
        POS_TRACE_ERROR(EID(MAPPER_SUCCESS), "[Mapper VolumeMount] failed to load VolumeId:{} array:{}", volId, arrayName);
        return (int)POS_EVENT_ID::VOL_EVENT_FAIL;
    }
    volState[volId].SetState(VolState::FOREGROUND_MOUNTED);
    vsaMapManager->WaitVolumePendingIoDone(volId);
    ++numMountedVol;
    POSMetricValue v;
    v.gauge = numMountedVol;
    tp->PublishData(TEL33006_MAP_MOUNTED_VOL_CNT, v, MT_GAUGE);
    vsaMapManager->EnableVsaMapAccess(volId);
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeMount] VolumeId:{} array:{} was FG_MOUNTED @VolumeMounted", volId, arrayName);
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

int
Mapper::VolumeLoaded(int volId, uint64_t volSizeByte)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VolumeLoaded] LOAD VOLUME Volume:{} size:{} array:{}", volId, volSizeByte, arrayName);
    assert(volSizeByte > 0);
    if (volState[volId].GetState() == VolState::VOLUME_DELETING)
    {
        return (int)POS_EVENT_ID::VOL_EVENT_FAIL;
    }
    volState[volId].SetState(VolState::EXIST_UNLOADED);
    volState[volId].SetSize(volSizeByte);
    vsaMapManager->EnableVsaMapInternalAccess(volId);
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeLoaded] VolumeId:{} array:{} state:EXIST_UNLOADED @VolumeLoaded", volId, arrayName);
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

int
Mapper::VolumeUnmounted(int volId, bool flushMapRequired)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VolumeUnmounted] UNMOUNT_VOLUME Volume:{} array:{}", volId, arrayName);
    vsaMapManager->DisableVsaMapAccess(volId);
    VolState state = volState[volId].GetState();
    if (VolState::FOREGROUND_MOUNTED == state)
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeUnmounted] VolumeId:{} array:{} was FG_MOUNTED >> BG_MOUNTED @VolumeUnmounted", volId, arrayName);
        volState[volId].SetState(VolState::BACKGROUND_MOUNTED);
        --numMountedVol;
        POSMetricValue v;
        v.gauge = numMountedVol;
        tp->PublishData(TEL33006_MAP_MOUNTED_VOL_CNT, v, MT_GAUGE);
        v.gauge = volId;
        tp->PublishData(TEL33004_MAP_UNMOUNTED_VOL, v, MT_GAUGE);
        if (flushMapRequired == true)
        {
            int ret = vsaMapManager->FlushTouchedPages(volId, nullptr);
            if (ret < 0)
            {
                POS_TRACE_ERROR(EID(VSAMAP_STORE_FAILURE), "[Mapper VolumeUnmounted] failed to Flush DirtyPages:{} VolumeId:{} array:{} @VolumeUnmounted", ret, volId, arrayName);
                return (int)POS_EVENT_ID::VOL_EVENT_FAIL;
            }
            vsaMapManager->WaitVolumePendingIoDone(volId);
        }
    }
    else
    {
        POS_TRACE_ERROR(EID(MAPPER_SUCCESS), "[Mapper VolumeUnmounted] VolumeId:{} array:{} State:{} is not mounted", volId, arrayName, volState[volId].GetState());
    }
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

int
Mapper::PrepareVolumeDelete(int volId)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VolumeDeleted] DELETE_VOLUME Volume:{} array:{}", volId, arrayName);
    VolState state = volState[volId].GetState();
    if ((VolState::VOLUME_DELETING == state) || (VolState::NOT_EXIST == state))
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VolumeDeleted] failed to Deleted VolumeId:{} array:{} state:{}", volId, arrayName, state);
        return -EID(MAPPER_FAILED);
    }
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeDeleted] VolumeId:{} array:{}", volId, arrayName);
    vsaMapManager->DisableVsaMapInternalAccess(volId);
    vsaMapManager->WaitVolumePendingIoDone(volId);

    // Unloaded case: Load & BG Mount
    if (volState[volId].GetState() == VolState::EXIST_UNLOADED)
    {
        if (_LoadVolumeMeta(volId, true) < 0)
        {
            POS_TRACE_WARN(EID(VSAMAP_LOAD_FAILURE), "[Mapper VolumeDeleted]VSAMap load failed, volumeID:{} array:{} @VolumeDeleted", volId, arrayName);
            return -EID(VSAMAP_LOAD_FAILURE);
        }
        vsaMapManager->WaitVolumePendingIoDone(volId);
    }

    if (vsaMapManager->NeedToDeleteFile(volId) == false)
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeDeleted] VolumeId:{} array:{} Volume VSA File does not exist", volId, arrayName);
    }
    else
    {
        if (_ChangeVolumeStateDeleting(volId) == false)
        {
            POS_TRACE_WARN(EID(VSAMAP_LOAD_FAILURE), "[Mapper VolumeDeleted] Another thread started to delete volumeID:{} array:{} @VolumeDeleted", volId, arrayName);
            return -EID(VSAMAP_LOAD_FAILURE);
        }
    }

    --numMapLoadedVol;
    POSMetricValue v;
    v.gauge = numMapLoadedVol;
    tp->PublishData(TEL33002_MAP_LOADED_VOL_CNT, v, MT_GAUGE);
    return 0;
}

int
Mapper::InvalidateAllBlocksTo(int volId, ISegmentCtx* segmentCtx)
{
    // Mark all blocks in this volume up as Invalidated
    if (0 != vsaMapManager->InvalidateAllBlocks(volId, segmentCtx))
    {
        POS_TRACE_WARN(EID(VSAMAP_INVALIDATE_ALLBLKS_FAILURE),
            "[Mapper VolumeDeleted] VSAMap Invalidate all blocks Failed, volumeID:{} array:{} @VolumeDeleted",
            volId, arrayName);
        return -EID(VSAMAP_INVALIDATE_ALLBLKS_FAILURE);
    }
    return 0;
}

int
Mapper::DeleteVolumeMap(int volId)
{
    if (0 != vsaMapManager->DeleteVSAMap(volId))
    {
        POS_TRACE_WARN(EID(MFS_FILE_DELETE_FAILED), "[Mapper VolumeDeleted] failed to delete VSA Map, volumeID:{} array:{} @VolumeDeleted", volId, arrayName);
        return -EID(MFS_FILE_DELETE_FAILED);
    }
    volState[volId].SetState(VolState::NOT_EXIST);
    return 0;
}

int
Mapper::VolumeDetached(vector<int> volList)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VolumeEvent] DETACH_VOLUME array:{}", arrayName);
    for (int volId : volList)
    {
        VolState state = volState[volId].GetState();
        if ((VolState::VOLUME_DELETING == state) || (VolState::NOT_EXIST == state))
        {
            continue;
        }
        vsaMapManager->DisableVsaMapAccess(volId);
        if (state == VolState::FOREGROUND_MOUNTED)
        {
            volState[volId].SetState(VolState::BACKGROUND_MOUNTED);
            --numMountedVol;
            POSMetricValue v;
            v.gauge = numMountedVol;
            tp->PublishData(TEL33006_MAP_MOUNTED_VOL_CNT, v, MT_GAUGE);
            v.gauge = volId;
            tp->PublishData(TEL33004_MAP_UNMOUNTED_VOL, v, MT_GAUGE);
            POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeDetached] VolumeId:{} array:{} was FG_MOUNTED >> BG_MOUNTED @VolumeDetached", volId, arrayName);
        }
    }
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

void
Mapper::SetVolumeState(int volId, VolState state, uint64_t size)
{
    volState[volId].SetState(state);
    volState[volId].SetSize(size);
}

void
Mapper::_Dispose(void)
{
    if (isInitialized == true)
    {
        vsaMapManager->WaitAllPendingIoDone();
        vsaMapManager->Dispose();
        stripeMapManager->WaitAllPendingIoDone();
        stripeMapManager->Dispose();
        reverseMapManager->WaitAllPendingIoDone();
        reverseMapManager->Dispose();
        _UnregisterFromMapperService();
        _ClearVolumeState();
        if ((addrInfo->IsUT() == false) && (tp != nullptr))
        {
            TelemetryClientSingleton::Instance()->DeregisterPublisher(tp->GetName());
        }
        isInitialized = false;
    }
}

int
Mapper::_LoadVolumeMeta(int volId, bool delVol)
{
    assert(volState[volId].GetSize() != 0);
    VolState state = volState[volId].GetState();
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper _LoadVolumeMeta] volume:{} array:{}, state:{}", volId, arrayName, state);
    if ((VolState::VOLUME_DELETING == state) || (VolState::NOT_EXIST == state))
    {
        POS_TRACE_ERROR(EID(VSAMAP_UNMOUNT_FAILURE), "[Mapper _LoadVolumeMeta] failed to load VolumeId:{}, state:{} arrayName:{} NOT_EXIST", volId, state, arrayName);
        return -1;
    }

    std::unique_lock<std::mutex> lock(volState[volId].GetVolStateLock());
    // Created or BG mounted volume case: FG Mount
    if (volState[volId].GetState() == VolState::EXIST_UNLOADED)
    {
        volState[volId].SetState(VolState::VOLUME_LOADING);
        POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper _LoadVolumeMeta] Set EXIST_UNLOADED >> VOLUME_LOADING volume:{} array:{}", volId, arrayName);
        // Unloaded volume case: Load & Mount
        if (vsaMapManager->CreateVsaMapContent(nullptr, volId, volState[volId].GetSize(), delVol) != 0)
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper _LoadVolumeMeta] failed to create VsaMapFile VolumeId:{}, arrayName:{} @VolumeMounted", volId, arrayName);
            return -1;
        }
        if (vsaMapManager->LoadVSAMapFile(volId) != 0)
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper _LoadVolumeMeta] failed to Load Trigger VolumeId:{}, arrayName:{} @VolumeMounted", volId, arrayName);
            return -1;
        }
        ++numMapLoadedVol;
        POSMetricValue v;
        v.gauge = numMapLoadedVol;
        tp->PublishData(TEL33002_MAP_LOADED_VOL_CNT, v, MT_GAUGE);
    }
    else
    {
        if (volState[volId].GetState() == VolState::VOLUME_LOADING)
        {
            POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper _LoadVolumeMeta] AnotherThread already started VolumeId:{} LOADING, arrayName:{} (NEED_RETRY/WAITPENDING)", volId, arrayName);
        }
        else
        {
            POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper _LoadVolumeMeta] VolumeId:{} was already BG/FG_MOUNTED:{}, arrayName:{} @VolumeMounted", volId, volState[volId].GetState(), arrayName);
        }
    }
    return 0;
}

void
Mapper::_RegisterToMapperService(void)
{
    MapperService* mapperService = MapperServiceSingleton::Instance();
    mapperService->RegisterMapper(arrayName, addrInfo->GetArrayId(),
        GetIVSAMap(), GetIStripeMap(), GetIReverseMap(), GetIMapFlush(), GetIMapperWbt());
}

void
Mapper::_UnregisterFromMapperService(void)
{
    MapperService* mapperService = MapperServiceSingleton::Instance();
    mapperService->UnregisterMapper(arrayName);
}

void
Mapper::_ClearVolumeState(void)
{
    for (int volId = 0; volId < MAX_VOLUME_COUNT; volId++)
    {
        volState[volId].SetState(VolState::NOT_EXIST);
    }
}

bool
Mapper::_ChangeVolumeStateDeleting(int volId)
{
    std::unique_lock<std::mutex> lock(volState[volId].GetVolStateLock());
    if (volState[volId].GetState() == VolState::VOLUME_DELETING)
    {
        return false;
    }
    volState[volId].SetState(VolState::VOLUME_DELETING);
    return true;
}

int
Mapper::_GetMpageSize(void)
{
    int mpageSize = 4096;
    if (metaFs != nullptr)
    {
        MetaFilePropertySet prop;
        prop.ioAccPattern = MetaFileAccessPattern::ByteIntensive;
        prop.ioOpType = MetaFileDominant::WriteDominant;
        prop.integrity = MetaFileIntegrityType::Lvl0_Disable;
        mpageSize = metaFs->EstimateAlignedFileIOSize(prop);
    }
    return mpageSize;
}

} // namespace pos
