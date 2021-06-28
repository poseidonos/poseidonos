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

#include "src/mapper/include/mapper_const.h"
#include "src/mapper/map_flushed_event.h"
#include "src/mapper/vsamap/vsamap_manager.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/journal_service/journal_service.h"

#include <string>
#include <vector>

namespace pos
{
VSAMapManager::VSAMapManager(MapperAddressInfo* info, std::string arrayName, int arrayId)
: VolumeEvent("Mapper", arrayName),
  volumeManager(nullptr)
{
    VolumeEventPublisherSingleton::Instance()->RegisterSubscriber(this, arrayName, 0);

    vsaMapAPI = new VSAMapAPI(this, info);

    volumeMountState.clear();
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        loadDoneFlag[volumeId] = NOT_LOADED;
    }
    loadDoneWakeUp = std::bind(&VSAMapManager::_WakeUpAfterLoadDone, this, std::placeholders::_1);
    loadDone = std::bind(&VSAMapManager::_AfterLoadDone, this, std::placeholders::_1);
    volumeManager = nullptr;
}

VSAMapManager::~VSAMapManager(void)
{
    VolumeEventPublisherSingleton::Instance()->RemoveSubscriber(this, arrayName, 0);

    delete vsaMapAPI;
}

void
VSAMapManager::Init(void)
{
}

int
VSAMapManager::StoreMaps(void)
{
    int ret = 0;
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        if (GetVSAMapContent(volumeId) == nullptr)   // Not loaded volumes
            continue;

        ret = GetVSAMapContent(volumeId)->StoreMap();
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(VSAMAP_STORE_FAILURE), "VSAMap SyncStore failed, volumeId:{}", volumeId);
        }
    }
    return ret;
}

int
VSAMapManager::FlushMaps(void)
{
    int ret = 0;
    for (auto& volState : volumeMountState)
    {
        if (VolState::EXIST_UNLOADED != volState.second)
        {
            int volumeId = volState.first;
            EventSmartPtr callBackVSAMap = std::make_shared<MapFlushedEvent>(volumeId, this);
            mapFlushStatus[volumeId] = MapFlushState::FLUSHING;
            ret = GetVSAMapContent(volumeId)->FlushTouchedPages(callBackVSAMap);
            if (ret < 0)
            {
                POS_TRACE_ERROR(EID(MAPPER_FAILED), "AsyncStore() for volumeId:{} Failed", volumeId);
            }
            else
            {
                POS_TRACE_INFO(EID(MAPPER_SUCCESS), "AsyncStore() for volumeId:{} Started", volumeId);
            }
        }
    }
    return ret;
}

void
VSAMapManager::Close(void)
{
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        if ((GetVSAMapContent(volumeId) != nullptr) && (GetVSAMapContent(volumeId)->IsFileOpened()))
        {
            POS_TRACE_INFO(EID(MAPPER_SUCCESS), "Mapper closes MFS file for volumeId:{}", volumeId);
            GetVSAMapContent(volumeId)->FileClose();
        }
    }
}

int
VSAMapManager::EnableInternalAccess(int volID, int caller)
{
    int ret = 0;
    VolMountStateIter iter;
    if (_IsVolumeExist(volID, iter) == false)
    {
        return -EID(VSAMAP_LOAD_FAILURE);
    }

    if (_GetVolumeState(volID) == VolState::VOLUME_DELETING)
    {
        return -EID(VSAMAP_LOAD_FAILURE);
    }

    // Unloaded volume case: Load & BG Mount
    if (iter->second == VolState::EXIST_UNLOADED)
    {
        // If another thread has already started loading
        if (volMountStateLock[volID].try_lock() == false)
        {
            if (CALLER_EVENT == caller)
            {
                ret = -EID(MAP_LOAD_ONGOING);
            }
            else
            {
                while (loadDoneFlag[volID] != LOAD_DONE)
                {
                    usleep(1);
                }
                ret = 0;
            }
            return ret;
        }

        std::string volName = "";
        if (NOT_LOADED == loadDoneFlag[volID])
        {
            if (CALLER_EVENT == caller)
            {
                volName = "CALLER_EVENT";
            }
        }
        else    // LOADING, LOAD_DONE
        {
            volMountStateLock[volID].unlock();
            if (CALLER_EVENT == caller)
            {
                return -EID(MAP_LOAD_ONGOING);
            }
            else
            {
                while (loadDoneFlag[volID] != LOAD_DONE)
                {
                    usleep(1);
                }
                return 0;
            }
        }

        // In case of internal-loading, we don't know the volume size
        if (VolumeMounted(volName, "", volID, UNKNOWN_SIZE_BECAUSEOF_INTERNAL_LOAD, 0, 0, "", 0))
        {
            POS_TRACE_INFO(EID(MAPPER_SUCCESS), "VolumeId:{} Internal Mount Request Succeeded", volID);
        }
        else
        {
            POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "VolumeId:{} Internal Mount Request Failed", volID);
            ret = -EID(VSAMAP_LOAD_FAILURE);
        }

        volMountStateLock[volID].unlock();
    }
    // Just Created volume case: BG Mount
    else if (iter->second == VolState::JUST_CREATED)
    {
        std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volID]);
        volumeMountState[iter->first] = VolState::BACKGROUND_MOUNTED;
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "VolumeId:{} was set as BG mounted", volID);
    }
    else // VolState::BACKGROUND_MOUNTED or VolState::FOREGROUND_MOUNTED
    {
        // Do nothing
    }

    return ret;
}

std::atomic<int>&
VSAMapManager::GetLoadDoneFlag(int volumeId)
{
    return loadDoneFlag[volumeId];
}

VSAMapContent*&
VSAMapManager::GetVSAMapContent(int volID)
{
    return vsaMapAPI->GetVSAMapContent(volID);
}

bool
VSAMapManager::AllMapsAsyncFlushed(void)
{
    bool allFlushed = true;

    for (auto& mapStatus : mapFlushStatus)
    {
        if (mapStatus.second != MapFlushState::FLUSH_DONE)
        {
            allFlushed = false;
            break;
        }
    }

    return allFlushed;
}

void
VSAMapManager::SetVolumeManagerObject(IVolumeManager* volumeManagerToUse)
{
    volumeManager = volumeManagerToUse;
}

IVSAMap*
VSAMapManager::GetIVSAMap(void)
{
    return vsaMapAPI;
}

VSAMapAPI*
VSAMapManager::GetVSAMapAPI(void)
{
    return vsaMapAPI;
}

//------------------------------------------------------------------------------

void
VSAMapManager::MapAsyncFlushDone(int mapId)
{
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "mapId:{} Flushed @MapAsyncFlushed", mapId);
    mapFlushStatus[mapId] = MapFlushState::FLUSH_DONE;
}

bool
VSAMapManager::VolumeCreated(std::string volName, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID)
{
    do
    {
        if (false == _PrepareVsaMapAndHeader(volID, volSizeByte, false))
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "Vsa map & Header Prepare Failed");
            break;
        }
        if (false == _PrepareInMemoryData(volID, volSizeByte))
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "In-memory Data Prepare Failed");
            break;
        }
        if (false == _VSAMapFileCreate(volID))
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "Vsa map File Creation Failed");
            break;
        }
        if (false == _VSAMapFileStore(volID))
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "Vsa map File Store Failed");
            break;
        }

        std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volID]);
        volumeMountState.emplace(volID, VolState::JUST_CREATED);
        vsaMapAPI->EnableVsaMapInternalAccess(volID);
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "VolumeId:{} JUST_CREATED", volID);
        return true;
    }
    while (false);
    return false;
}

bool
VSAMapManager::VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayNAme, int arrayID)
{
    if (_GetVolumeState(volID) == VolState::VOLUME_DELETING)
    {
        return false;
    }

    bool isUnknownSize = (volSizeByte == UNKNOWN_SIZE_BECAUSEOF_INTERNAL_LOAD);
    return _LoadVolumeMeta(volName, volID, volSizeByte, isUnknownSize);
}

bool
VSAMapManager::VolumeLoaded(std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID)
{
    if (_GetVolumeState(id) == VolState::VOLUME_DELETING)
    {
        return false;
    }

    std::unique_lock<std::recursive_mutex> lock(volMountStateLock[id]);
    volumeMountState.emplace(id, VolState::EXIST_UNLOADED);
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "VolumeId:{} is inserted to volumeMountState as EXIST_UNLOADED @VolumeLoaded", id);
    return true;
}

bool
VSAMapManager::VolumeUpdated(std::string volName, int volID, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID)
{
    return true;
}

bool
VSAMapManager::VolumeUnmounted(std::string volName, int volID, std::string arrayName, int arrayID)
{
    if (_GetVolumeState(volID) == VolState::VOLUME_DELETING)
    {
        return false;
    }

    vsaMapAPI->DisableVsaMapAccess(volID);

    do
    {
        EventSmartPtr callBackVSAMap = std::make_shared<MapFlushedEvent>(volID, this);
        mapFlushStatus[volID] = MapFlushState::FLUSHING;

        int ret = GetVSAMapContent(volID)->FlushTouchedPages(callBackVSAMap);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(VSAMAP_STORE_FAILURE), "ret:{} of vsaMap->Unload(), VolumeId:{} @VolumeUnmounted", ret, volID);
        }
        _WaitForMapAsyncFlushed(volID);

        std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volID]);
        VolMountStateIter iter;
        if (_IsVolumeExist(volID, iter))
        {
            if (iter->second == VolState::FOREGROUND_MOUNTED)
            {
                volumeMountState[iter->first] = VolState::BACKGROUND_MOUNTED;
                POS_TRACE_INFO(EID(MAPPER_SUCCESS), "VolumeId:{} was set as BG_MOUNTED @VolumeUnmounted", volID);
            }
            else
            {
                POS_TRACE_WARN(EID(VSAMAP_UNMOUNT_FAILURE), "volumeID:{} is Not FG_MOUNTED @VolumeUnmounted", volID);
            }
        }

        return true;
    } while (false);

    return false;
}

bool
VSAMapManager::VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte, std::string arrayName, int arrayID)
{
    // std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volID]);
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "Starting VolumeDelete: volID:{}  volSizeByte:{}", volID, volSizeByte);

    VSAMapContent*& vsaMap = GetVSAMapContent(volID);

    // Unloaded case: Load & BG Mount
    if (nullptr == vsaMap)
    {
        // We know the volume size but deal with internal loading
        if (false == _LoadVolumeMeta(volName, volID, volSizeByte, true))
        {
            POS_TRACE_WARN(EID(VSAMAP_LOAD_FAILURE), "VSAMap load failed, volumeID:{} @VolumeDeleted", volID);
            return false;
        }
    }

    if (_ChangeVolumeStateDeleting(volID) == false)
    {
        POS_TRACE_WARN(EID(VSAMAP_LOAD_FAILURE), "Another thread started to delete volumeID:{} @VolumeDeleted", volID);
        return true;
    }

    if (vsaMap->DoesFileExist() == false)
    {
        // VolumeDeleted can be notified again when pos crashes during volume deletion
        POS_TRACE_DEBUG(EID(NO_BLOCKMAP_MFS_FILE), "No MFS filename:{} for volName:{} @VolumeDeleted", vsaMap->GetFileName(), volName);
        return true;
    }

    // Mark all blocks in this volume up as Invalidated
    if (0 != vsaMap->InvalidateAllBlocks())
    {
        POS_TRACE_WARN(EID(VSAMAP_INVALIDATE_ALLBLKS_FAILURE), "VSAMap Invalidate all blocks Failed, volumeID:{} @VolumeDeleted", volID);
        return false;
    }

    // Write log for deleted volume
    if (0 != JournalServiceSingleton::Instance()->GetVolumeEventHandler(arrayName)->VolumeDeleted(volID))
    {
        return false;
    }

    // file close and delete
    if (0 != vsaMap->FileClose())
    {
        POS_TRACE_WARN(EID(MFS_FILE_CLOSE_FAILED), "VSAMap File close failed, volumeID:{} @VolumeDeleted", volID);
        return false;
    }
    if (0 != vsaMap->DeleteMapFile())
    {
        POS_TRACE_WARN(EID(MFS_FILE_DELETE_FAILED), "VSAMap File delete failed, volumeID:{} @VolumeDeleted", volID);
        return false;
    }

    // clean up contents in dram
    delete vsaMap;
    vsaMap = nullptr;
    volumeMountState.erase(volID);

    return true;
}

void
VSAMapManager::VolumeDetached(vector<int> volList, std::string arrayName, int arrayID)
{
    for (int volumeId : volList)
    {
        vsaMapAPI->DisableVsaMapAccess(volumeId);
        if (_GetVolumeState(volumeId) == VolState::VOLUME_DELETING)
        {
            continue;
        }

        std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volumeId]);

        VolMountStateIter iter;
        if (_IsVolumeExist(volumeId, iter))
        {
            if (iter->second == VolState::FOREGROUND_MOUNTED)
            {
                volumeMountState[iter->first] = VolState::BACKGROUND_MOUNTED;
                POS_TRACE_INFO(EID(MAPPER_SUCCESS), "VolumeId:{} was set as BG_MOUNTED @VolumeDetached", volumeId);
            }
            else
            {
                POS_TRACE_WARN(EID(VSAMAP_UNMOUNT_FAILURE), "volumeId:{} is Not FG_MOUNTED @VolumeDetached", volumeId);
            }
        }
    }
}

bool
VSAMapManager::_ChangeVolumeStateDeleting(uint32_t volID)
{
    std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volID]);
    VolMountStateIter it = volumeMountState.find(volID);
    if (volumeMountState[it->first] == VolState::VOLUME_DELETING)
    {
        return false;
    }
    volumeMountState[it->first] = VolState::VOLUME_DELETING;
    vsaMapAPI->DisableVsaMapInternalAccess(volID);
    return true;
}

VolState
VSAMapManager::_GetVolumeState(uint32_t volID)
{
    std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volID]);
    VolMountStateIter it = volumeMountState.find(volID);
    if (it == volumeMountState.end())
    {
        return VolState::NOT_EXIST;
    }
    else
    {
        return volumeMountState[it->first];
    }
}

bool
VSAMapManager::_PrepareVsaMapAndHeader(int volID, uint64_t& volSizeByte, bool isUnknownVolSize)
{
    int ret = 0;
    VSAMapContent*& vsaMapRef = vsaMapAPI->GetVSAMapContent(volID);
    assert(vsaMapRef == nullptr);

    vsaMapRef = new VSAMapContent(volID, arrayName);
    if (isUnknownVolSize && (volSizeByte == 0)) // Internal loading case
    {
        uint64_t volSizebyVolMgr = 0;

        if (volumeManager == nullptr)
        {
            IVolumeManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
            ret = volMgr->GetVolumeSize(volID, volSizebyVolMgr);
        }
        else
        {
            ret = volumeManager->GetVolumeSize(volID, volSizebyVolMgr);
        }
        if (ret != 0)
        {
            POS_TRACE_WARN(EID(VSAMAP_HEADER_LOAD_FAILURE), "Getting volume Size failed, volumeID:{}  volSizebyVolMgr:{}", volID, volSizebyVolMgr);
            return false;
        }

        volSizeByte = volSizebyVolMgr;
        POS_TRACE_INFO(9999, "volID:{}  volSizebyVolMgr:{}", volID, volSizebyVolMgr);
    }
    uint64_t blkCnt = DivideUp(volSizeByte, pos::BLOCK_SIZE);

    return (0 == vsaMapRef->Prepare(blkCnt, volID));
}

bool
VSAMapManager::_PrepareInMemoryData(int volID, uint64_t volSizeByte)
{
    VSAMapContent*& vsaMapRef = vsaMapAPI->GetVSAMapContent(volID);
    assert(vsaMapRef != nullptr);

    uint64_t blkCnt = DivideUp(volSizeByte, pos::BLOCK_SIZE);

    return (0 == vsaMapRef->InMemoryInit(blkCnt, volID));
}

bool
VSAMapManager::_VSAMapFileCreate(int volID)
{
    VSAMapContent*& vsaMapRef = vsaMapAPI->GetVSAMapContent(volID);
    assert(vsaMapRef != nullptr);

    return (0 == vsaMapRef->CreateMapFile());
}

int
VSAMapManager::_VSAMapFileAsyncLoad(int volID)
{
    VSAMapContent*& vsaMapRef = vsaMapAPI->GetVSAMapContent(volID);
    assert(vsaMapRef != nullptr);

    unique_lock<std::mutex> lock(volAsyncLoadLock);
    loadDoneFlag[volID] = NOT_LOADED;

    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "VSAMap Async Load Started, volID:{} @_VSAMapFileAsyncLoad", volID);
    int ret = vsaMapRef->LoadAsync(loadDoneWakeUp);
    if (ret < 0)
    {
        if (-EID(MAP_LOAD_COMPLETED) == ret)
        {
            loadDoneFlag[volID] = LOAD_DONE;
            ret = 0; // This is a normal case
            POS_TRACE_INFO(EID(MAPPER_START), "No mpage to Load, so VSAMap Async Load Finished, volID:{} @_VSAMapFileAsyncLoad", volID);
        }
        else
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "Error on AsyncLoad trigger, volID:{}  ret:{} @_VSAMapFileAsyncLoad", volID, ret);
        }
    }
    else
    {
        cvLoadDone.wait(lock, [&] { return LOAD_DONE == loadDoneFlag[volID]; });
        POS_TRACE_INFO(EID(MAPPER_START), "after waking up, VSAMap Async Load Finished, volID:{} @_VSAMapFileAsyncLoad", volID);
    }

    return ret;
}

int
VSAMapManager::_VSAMapFileAsyncLoadNoWait(int volID)
{
    VSAMapContent*& vsaMapRef = vsaMapAPI->GetVSAMapContent(volID);
    assert(vsaMapRef != nullptr);

    loadDoneFlag[volID] = LOADING;

    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "VSAMap Async Load Started, volID:{} @VSAMapFileAsyncLoadNoWait", volID);
    int ret = vsaMapRef->LoadAsyncEvent(loadDone);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "Error on AsyncLoad trigger, volID:{}  ret:{} @VSAMapFileAsyncLoadNoWait", volID, ret);
    }

    return ret;
}

bool
VSAMapManager::_VSAMapFileStore(int volID)
{
    VSAMapContent*& vsaMapRef = vsaMapAPI->GetVSAMapContent(volID);
    assert(vsaMapRef != nullptr);

    return (0 == vsaMapRef->StoreMap());
}

void
VSAMapManager::_WakeUpAfterLoadDone(int volID)
{
    std::unique_lock<std::mutex> lock(volAsyncLoadLock);
    POS_TRACE_INFO(EID(MAP_LOAD_COMPLETED), "volID:{} async load done, so wake up! @_WakeUpAfterLoadDone", volID);
    loadDoneFlag[volID] = LOAD_DONE;
    cvLoadDone.notify_all();
}

void
VSAMapManager::_AfterLoadDone(int volID)
{
    std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volID]);

    loadDoneFlag[volID] = LOAD_DONE;
    POS_TRACE_INFO(EID(MAP_LOAD_COMPLETED), "volID:{} async load done @_AfterLoadDone", volID);

    volumeMountState[volID] = VolState::BACKGROUND_MOUNTED;
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "VolumeId:{} was loaded and set as BG_MOUNTED @_AfterLoadDone", volID);
}

bool
VSAMapManager::_IsVolumeExist(int volID, VolMountStateIter& iter)
{
    VolMountStateIter it = volumeMountState.find(volID);
    iter = it;

    if (it == volumeMountState.end())
    {
        POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "VolumeId:{} is not in volumeMountState", volID);
        int cnt = 0;
        for (auto& entry : volumeMountState)
        {
            ++cnt;
            POS_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "INFO - volumeMountState {}/{}  K:{}  V:{}",
                            cnt, volumeMountState.size(), entry.first, entry.second);
        }
        return false;
    }

    return true;
}

bool
VSAMapManager::_LoadVolumeMeta(std::string volName, int volID, uint64_t volSizeByte, bool isUnknownVolSize)
{
    // If GC(EventWorker thread) is already loading the VSAMap of volID, CLI thread has to wait until loading done
    while (false == isUnknownVolSize && LOADING == loadDoneFlag[volID])
    {
        usleep(1);
    }

    std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volID]);
    VolMountStateIter iter;
    if (_IsVolumeExist(volID, iter) == false)
    {
        return false;
    }

    // Created or BG mounted volume case: FG Mount
    if ((iter->second == VolState::JUST_CREATED || iter->second == VolState::BACKGROUND_MOUNTED) && (false == isUnknownVolSize))
    {
        volumeMountState[iter->first] = VolState::FOREGROUND_MOUNTED;
        vsaMapAPI->EnableVsaMapAccess(volID);
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "VolumeId:{} was set as FG_MOUNTED @VolumeMounted", volID);
        return true;
    }

    // Unloaded volume case: Load & Mount
    do
    {
        if (iter->second == VolState::EXIST_UNLOADED)
        {
            if (_PrepareVsaMapAndHeader(volID, volSizeByte, isUnknownVolSize) == false)
            {
                POS_TRACE_ERROR(EID(MAPPER_FAILED), "Vsa map & Header Prepare Failed @VolumeMounted");
                break;
            }
            if (_PrepareInMemoryData(volID, volSizeByte) == false)
            {
                POS_TRACE_ERROR(EID(MAPPER_FAILED), "In-memory Data Prepare Failed @VolumeMounted");
                break;
            }
            // GC: EventWorker thread
            if (isUnknownVolSize && ("CALLER_EVENT" == volName))
            {
                if (_VSAMapFileAsyncLoadNoWait(volID) != 0)
                {
                    POS_TRACE_ERROR(EID(MAPPER_FAILED), "Vsa map File Async Load Trigger Failed @VolumeMounted");
                    break;
                }
            }
            else
            {
                if (_VSAMapFileAsyncLoad(volID) != 0)
                {
                    POS_TRACE_ERROR(EID(MAPPER_FAILED), "Vsa map File Async Load Failed @VolumeMounted");
                    break;
                }
                // Replay: CLI thread
                if (isUnknownVolSize)
                {
                    volumeMountState[iter->first] = VolState::BACKGROUND_MOUNTED;
                    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "VolumeId:{} was loaded and set as BG_MOUNTED @VolumeMounted", volID);
                }
                // User Action: CLI thread
                else
                {
                    volumeMountState[iter->first] = VolState::FOREGROUND_MOUNTED;
                    vsaMapAPI->EnableVsaMapAccess(volID);
                    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "VolumeId:{} was loaded and set as FG_MOUNTED @VolumeMounted", volID);
                }
            }

            return true;
        }
    } while (false);

    return false;
}

void
VSAMapManager::_WaitForMapAsyncFlushed(int volId)
{
    while (mapFlushStatus[volId] != MapFlushState::FLUSH_DONE)
    {
        usleep(1);
    }
}

} // namespace pos
