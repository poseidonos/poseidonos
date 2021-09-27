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

namespace pos
{
MpageList IMapFlush::DEFAULT_DIRTYPAGE_SET;

Mapper::Mapper(IArrayInfo* iarrayInfo, IStateControl* iState)
: VolumeEvent("Mapper", iarrayInfo->GetName(), iarrayInfo->GetIndex()),
  iArrayinfo(iarrayInfo),
  iStateControl(iState),
  isInitialized(false)
{
    VolumeEventPublisherSingleton::Instance()->RegisterSubscriber(this, iarrayInfo->GetName(), iarrayInfo->GetIndex());
    addrInfo = new MapperAddressInfo(iarrayInfo);
    vsaMapManager = new VSAMapManager(addrInfo);
    stripeMapManager = new StripeMapManager(addrInfo, iarrayInfo->GetName(), iarrayInfo->GetIndex());
    reverseMapManager = new ReverseMapManager(vsaMapManager, stripeMapManager, iArrayinfo);
    mapperWbt = new MapperWbt(addrInfo, vsaMapManager, stripeMapManager, reverseMapManager);
    metaFs = nullptr;
    journalService = nullptr;
    _ClearVolumeState();
}

Mapper::~Mapper(void)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper Destructor] in Array:{} was Destroyed", addrInfo->GetArrayName());
    _Dispose();
    delete mapperWbt;
    delete reverseMapManager;
    delete stripeMapManager;
    delete vsaMapManager;
    delete addrInfo;
    VolumeEventPublisherSingleton::Instance()->RemoveSubscriber(this, iArrayinfo->GetName(), iArrayinfo->GetIndex());
}

void
Mapper::Dispose(void)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper Dispose] MAPPER Disposed, init:{} array:{}", isInitialized, addrInfo->GetArrayName());
    if (isInitialized == true)
    {
        StoreAll();
        _Dispose();
    }
}

void
Mapper::Shutdown(void)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper Shutdown] MAPPER Shutdown, init:{} array:{}", isInitialized, addrInfo->GetArrayName());
    _Dispose();
}

int
Mapper::Init(void)
{
    int ret = 0;
    if (isInitialized == false)
    {
        metaFs = MetaFsServiceSingleton::Instance()->GetMetaFs(addrInfo->GetArrayId());
        assert(metaFs != nullptr);
        int mpageSize = _GetMpageSize();
        addrInfo->SetupAddressInfo(mpageSize);
        ret = vsaMapManager->Init();
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(VSAMAP_NOT_LOADED), "[Mapper Init] Failed to Initialize VSAMapManager, array:{}", addrInfo->GetArrayName());
            assert(false);
            return ret;
        }
        ret = stripeMapManager->Init();
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(VSAMAP_NOT_LOADED), "[Mapper Init] Failed to Initialize StripeMap, array:{}", addrInfo->GetArrayName());
            assert(false);
            return ret;
        }
        reverseMapManager->Init(*addrInfo);
        _RegisterToMapperService();
        journalService = JournalServiceSingleton::Instance();
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
Mapper::FlushDirtyMpages(int mapId, EventSmartPtr event, MpageList dirtyPages)
{
    MapContent* map = _GetMapContent(mapId);
    if (nullptr == map)
    {
        return -EID(WRONG_MAP_ID);
    }

    // NVMe FLUSH command: executed by event (FlushCmdHandler)
    if (IMapFlush::DEFAULT_DIRTYPAGE_SET == dirtyPages)
    {
        return map->FlushTouchedPages(event);
    }
    // Journal Checkpoint: executed by event (CheckpointSubmission)
    else
    {
        return map->FlushDirtyPagesGiven(dirtyPages, event);
    }
    bool journalEnabled = journalService->IsEnabled(arrayName);
    if (journalEnabled == false)
    {
        vsaMapManager->WaitVolumePendingIoDone(mapId);
    }
}

int
Mapper::StoreAll(void)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper FlushAll] Flush All Synchronously, array:{}", addrInfo->GetArrayName());
    int ret = stripeMapManager->FlushMap();
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper FlushAll] Failed To Flush All StripeMap, array:{}", addrInfo->GetArrayName());
        return ret;
    }
    ret = vsaMapManager->FlushAllMaps();
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper FlushAll] Failed To Flush All VSAMap, array:{}", addrInfo->GetArrayName());
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
        if (_LoadVolumeMeta(volId) == false)
        {
            POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "[Mapper EnableInternalAccess] failed to load VolumeId:{}, state:{} array:{}", volId, volState[volId].GetState(), addrInfo->GetArrayName());
            return -EID(VSAMAP_LOAD_FAILURE);
        }
        else
        {
            POS_TRACE_INFO(EID(VSAMAP_LOAD_FAILURE), "[Mapper EnableInternalAccess] Load VolumeId:{}, state:{} array:{} Issued (ret:NEED_RETRY)", volId, volState[volId].GetState(), addrInfo->GetArrayName());
            return NEED_RETRY;
        }
    }
    else if (VolState::VOLUME_LOADING == state)
    {
        if (vsaMapManager->IsVolumeLoaded(volId) == true)
        {
            volState[volId].SetState(VolState::BACKGROUND_MOUNTED);
            POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper EnableInternalAccess] VolumeId:{} was loaded LOADING >> BG_MOUNTED, arrayName:{} @VolumeMounted", volId, addrInfo->GetArrayName());
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
    retry = NEED_RETRY;
    if (volState[volId].GetState() == VolState::VOLUME_LOADING)
    {
        return UNMAP_VSA;
    }
    int ret = EnableInternalAccess(volId);
    if ((ret < 0) || (ret == NEED_RETRY))
    {
        return UNMAP_VSA;
    }
    retry = OK_READY;
    return vsaMapManager->GetVSAWoCond(volId, rba);
}

int
Mapper::SetVSAsInternal(int volId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    int ret = EnableInternalAccess(volId);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "[Mapper SetVSAInternal] VolumeId:{} array:{} volume is not accessible", volId, addrInfo->GetArrayName());
        return ret;
    }
    return vsaMapManager->SetVSAsWoCond(volId, startRba, virtualBlks);
}

VirtualBlkAddr
Mapper::GetVSAWithSyncOpen(int volId, BlkAddr rba)
{
    int ret = EnableInternalAccess(volId);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "[Mapper GetVSAWithSyncOpen] Failed to Load VolumeId:{} array:{}", volId, addrInfo->GetArrayName());
        assert(false);
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
        POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "[Mapper SetVSAsWithSyncOpen] Failed to Load VolumeId:{} array:{}", volId, addrInfo->GetArrayName());
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
            POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "[Mapper GetNumUsedBlks] Failed to Load VolumeId:{} array:{}", volId, addrInfo->GetArrayName());
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

bool
Mapper::VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    int volId = volEventBase->volId;
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VolumeCreate] CREATE_VOLUME Volume:{}, size:{} array:{}", volId, volEventBase->volSizeByte, addrInfo->GetArrayName());
    assert (volEventBase->volSizeByte > 0);
    VolState state = volState[volId].GetState();
    if (state != VolState::NOT_EXIST)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VolumeCreate] Volume:{} array:{} already exist, state:{}", volId, addrInfo->GetArrayName(), volState[volId].GetState());
        return false;
    }
    if (vsaMapManager->CreateVsaMapContent(volId, volEventBase->volSizeByte, false) == false)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VolumeCreate] failed to create vsaMap VolumeId:{} array:{}", volId, addrInfo->GetArrayName());
        return false;
    }
    vsaMapManager->WaitVolumePendingIoDone(volId);
    volState[volId].SetState(VolState::BACKGROUND_MOUNTED);
    volState[volId].SetSize(volEventBase->volSizeByte);
    vsaMapManager->EnableVsaMapInternalAccess(volId);
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeCreate] VolumeId:{} array:{} JUST_CREATED >> BG_MOUNTED", volId, addrInfo->GetArrayName());
    return true;
}

bool
Mapper::VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    int volId = volEventBase->volId;
    VolState state = volState[volId].GetState();
    if (VolState::VOLUME_DELETING == state)
    {
        POS_TRACE_ERROR(EID(MAPPER_SUCCESS), "[Mapper VolumeMount] failed to mount VolumeId:{} array:{}, state:VOLUME_DELETING", volId, addrInfo->GetArrayName());
        return false;
    }
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VolumeMount] MOUNT_VOLUME Volume:{}, state:{}, array:{}", volId, state, addrInfo->GetArrayName());
    if (_LoadVolumeMeta(volId) == false)
    {
        POS_TRACE_ERROR(EID(MAPPER_SUCCESS), "[Mapper VolumeMount] failed to load VolumeId:{} array:{}", volId, addrInfo->GetArrayName());
        return false;
    }
    vsaMapManager->WaitVolumePendingIoDone(volId);
    volState[volId].SetState(VolState::FOREGROUND_MOUNTED);
    vsaMapManager->EnableVsaMapAccess(volId);
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeMount] VolumeId:{} array:{} was FG_MOUNTED @VolumeMounted", volId, addrInfo->GetArrayName());
    return true;
}

bool
Mapper::VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    int volId = volEventBase->volId;
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VolumeLoaded] LOAD VOLUME Volume:{} size:{} array:{}", volId, volEventBase->volSizeByte, addrInfo->GetArrayName());
    assert (volEventBase->volSizeByte > 0);
    if (volState[volId].GetState() == VolState::VOLUME_DELETING)
    {
        return false;
    }
    volState[volId].SetState(VolState::EXIST_UNLOADED);
    volState[volId].SetSize(volEventBase->volSizeByte);
    vsaMapManager->EnableVsaMapInternalAccess(volId);
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeLoaded] VolumeId:{} array:{} state:EXIST_UNLOADED @VolumeLoaded", volId, addrInfo->GetArrayName());
    return true;
}

bool
Mapper::VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    return true;
}

bool
Mapper::VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    int volId = volEventBase->volId;
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VolumeUnmounted] UNMOUNT_VOLUME Volume:{} array:{}", volId, addrInfo->GetArrayName());
    vsaMapManager->DisableVsaMapAccess(volId);
    VolState state = volState[volId].GetState();
    if (VolState::FOREGROUND_MOUNTED == state)
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeUnmounted] VolumeId:{} array:{} was FG_MOUNTED >> BG_MOUNTED @VolumeUnmounted", volId, addrInfo->GetArrayName());
        volState[volId].SetState(VolState::BACKGROUND_MOUNTED);
        bool journalEnabled = journalService->IsEnabled(arrayName);
        if (journalEnabled == false)
        {
            int ret = vsaMapManager->FlushMap(volId);
            if (ret < 0)
            {
                POS_TRACE_ERROR(EID(VSAMAP_STORE_FAILURE), "[Mapper VolumeUnmounted] failed to Flush DirtyPages:{} VolumeId:{} array:{} @VolumeUnmounted", ret, volId, addrInfo->GetArrayName());
                return false;
            }
            vsaMapManager->WaitVolumePendingIoDone(volId);
        }
    }
    else
    {
        POS_TRACE_ERROR(EID(MAPPER_SUCCESS), "[Mapper VolumeUnmounted] VolumeId:{} array:{} State:{} is not mounted", volId, addrInfo->GetArrayName(), volState[volId].GetState());
    }
    return true;
}

bool
Mapper::VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    int volId = volEventBase->volId;
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VolumeEvent] DELETE_VOLUME Volume:{} array:{}", volId, addrInfo->GetArrayName());
    VolState state = volState[volId].GetState();
    if ((VolState::VOLUME_DELETING == state) || (VolState::NOT_EXIST == state))
    {
        POS_TRACE_ERROR(EID(MAPPER_SUCCESS), "[Mapper VolumeDeleted] failed to Deleted VolumeId:{} array:{} state:VOLUME_DELETING", volId, addrInfo->GetArrayName());
        return false;
    }
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeDeleted] VolumeId:{} array:{} volSizeByte:{}", volId, addrInfo->GetArrayName(), volEventBase->volSizeByte);
    vsaMapManager->DisableVsaMapInternalAccess(volId);

    // Unloaded case: Load & BG Mount
    if (volState[volId].GetState() == VolState::EXIST_UNLOADED)
    {
        if (_LoadVolumeMeta(volId, true) == false)
        {
            POS_TRACE_WARN(EID(VSAMAP_LOAD_FAILURE), "[Mapper VolumeDeleted]VSAMap load failed, volumeID:{} array:{} @VolumeDeleted", volId, addrInfo->GetArrayName());
            return false;
        }
        vsaMapManager->WaitVolumePendingIoDone(volId);
    }

    if (vsaMapManager->NeedToDeleteFile(volId) == false)
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeDeleted] VolumeId:{} array:{} Volume VSA File does not exist", volId, addrInfo->GetArrayName());
    }
    else
    {
        if (_ChangeVolumeStateDeleting(volId) == false)
        {
            POS_TRACE_WARN(EID(VSAMAP_LOAD_FAILURE), "[Mapper VolumeDeleted] Another thread started to delete volumeID:{} array:{} @VolumeDeleted", volId, addrInfo->GetArrayName());
            return false;
        }

        IVolumeEventHandler* journalVolumeHandler = JournalServiceSingleton::Instance()->GetVolumeEventHandler(arrayName);    // Write log for deleted volume
        if (0 != journalVolumeHandler->WriteVolumeDeletedLog(volId))
        {
            return false;
        }

        // Mark all blocks in this volume up as Invalidated
        if (0 != vsaMapManager->InvalidateAllBlocks(volId))
        {
            POS_TRACE_WARN(EID(VSAMAP_INVALIDATE_ALLBLKS_FAILURE), "[Mapper VolumeDeleted] VSAMap Invalidate all blocks Failed, volumeID:{} array:{} @VolumeDeleted", volId, addrInfo->GetArrayName());
            return false;
        }

        if (0 != journalVolumeHandler->TriggerMetadataFlush())
        {
            return false;
        }
    }

    if (0 != vsaMapManager->DeleteVSAMap(volId))
    {
        POS_TRACE_WARN(EID(MFS_FILE_DELETE_FAILED), "[Mapper VolumeDeleted] failed to delete VSA Map, volumeID:{} array:{} @VolumeDeleted", volId, addrInfo->GetArrayName());
        return false;
    }
    volState[volId].SetState(VolState::NOT_EXIST);
    return true;
}

void
Mapper::VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo)
{
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VolumeEvent] DETACH_VOLUME array:{}", addrInfo->GetArrayName());
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
            POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VolumeDetached] VolumeId:{} array:{} was FG_MOUNTED >> BG_MOUNTED @VolumeDetached", volId, addrInfo->GetArrayName());
        }
    }
}

void
Mapper::_Dispose(void)
{
    if (isInitialized == true)
    {
        vsaMapManager->Dispose();
        stripeMapManager->Dispose();
        reverseMapManager->Dispose();
        _UnregisterFromMapperService();
        _ClearVolumeState();
        isInitialized = false;
    }
}

bool
Mapper::_LoadVolumeMeta(int volId, bool delVol)
{
    assert(volState[volId].GetSize() >= 0);
    VolState state = volState[volId].GetState();
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper _LoadVolumeMeta] volume:{} array:{}, state:{}", volId, addrInfo->GetArrayName(), state);
    if ((VolState::VOLUME_DELETING == state) || (VolState::NOT_EXIST == state))
    {
        POS_TRACE_ERROR(EID(VSAMAP_UNMOUNT_FAILURE), "[Mapper _LoadVolumeMeta] failed to load VolumeId:{}, state:{} arrayName:{} NOT_EXIST", volId, state, addrInfo->GetArrayName());
        return false;
    }

    std::unique_lock<std::mutex> lock(volState[volId].GetVolStateLock());
    // Created or BG mounted volume case: FG Mount
    if (volState[volId].GetState() == VolState::EXIST_UNLOADED)
    {
        volState[volId].SetState(VolState::VOLUME_LOADING);
        POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper _LoadVolumeMeta] Set EXIST_UNLOADED >> VOLUME_LOADING volume:{} array:{}", volId, addrInfo->GetArrayName());
        // Unloaded volume case: Load & Mount
        if (vsaMapManager->CreateVsaMapContent(volId, volState[volId].GetSize(), delVol) == false)
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper _LoadVolumeMeta] failed to create VsaMapFile VolumeId:{}, arrayName:{} @VolumeMounted", volId, addrInfo->GetArrayName());
            return false;
        }

        if (vsaMapManager->LoadVSAMapFile(volId) != 0)
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper _LoadVolumeMeta] failed to Load Trigger VolumeId:{}, arrayName:{} @VolumeMounted", volId, addrInfo->GetArrayName());
            return false;
        }
    }
    else
    {
        if (volState[volId].GetState() == VolState::VOLUME_LOADING)
        {
            POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper _LoadVolumeMeta] AnotherThread already started VolumeId:{} LOADING, arrayName:{} (NEED_RETRY/WAITPENDING)", volId, addrInfo->GetArrayName());
        }
        else
        {
            POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper _LoadVolumeMeta] VolumeId:{} was already BG/FG_MOUNTED:{}, arrayName:{} @VolumeMounted", volId, volState[volId].GetState(), addrInfo->GetArrayName());
        }
    }
    return true;
}

MapContent*
Mapper::_GetMapContent(int mapId)
{
    MapContent* map = nullptr;

    if (mapId == STRIPE_MAP_ID)
    {
        map = stripeMapManager->GetStripeMapContent();
    }
    else if (0 <= mapId && mapId < MAX_VOLUME_COUNT)
    {
        map = vsaMapManager->GetVSAMapContent(mapId);
    }

    return map;
}

void
Mapper::_RegisterToMapperService(void)
{
    MapperService* mapperService = MapperServiceSingleton::Instance();
    mapperService->RegisterMapper(iArrayinfo->GetName(), iArrayinfo->GetIndex(),
        GetIVSAMap(), GetIStripeMap(), GetIReverseMap(), GetIMapFlush(), GetIMapperWbt());
}

void
Mapper::_UnregisterFromMapperService(void)
{
    MapperService* mapperService = MapperServiceSingleton::Instance();
    mapperService->UnregisterMapper(iArrayinfo->GetName());
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
    MetaFilePropertySet prop;
    prop.ioAccPattern = MetaFileAccessPattern::ByteIntensive;
    prop.ioOpType = MetaFileDominant::WriteDominant;
    prop.integrity = MetaFileIntegrityType::Lvl0_Disable;
    mpageSize = metaFs->ctrl->EstimateAlignedFileIOSize(prop);
    return mpageSize;
}

} // namespace pos
