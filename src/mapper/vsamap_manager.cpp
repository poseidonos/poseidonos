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

#include "vsamap_manager.h"

#include <string>
#include <vector>

#include "map_flushed_event.h"

namespace ibofos
{
VSAMapManager::VSAMapManager(void)
: VolumeEvent("Mapper")
{
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        vsaMaps[volumeId] = nullptr;
        isVsaMapAccessable[volumeId] = false;
    loadDoneFlag[volumeId] = NOT_LOADED;
    }

    volumeMountState.clear();
    loadDoneWakeUp = std::bind(&VSAMapManager::_WakeUpAfterLoadDone,
        this, std::placeholders::_1);
    loadDone = std::bind(&VSAMapManager::_AfterLoadDone, this,
        std::placeholders::_1);
    volumeManager = VolumeManagerSingleton::Instance();
}

VSAMapManager:: ~VSAMapManager(void)
{
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        if (vsaMaps[volumeId] != nullptr)
        {
            delete vsaMaps[volumeId];
        }
    }
}

int
VSAMapManager::SyncStore(void)
{
    int ret = 0;

    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        // Not loaded volumes
        if (vsaMaps[volumeId] == nullptr)
            continue;

        ret = vsaMaps[volumeId]->StoreSync();
        if (ret < 0)
        {
            IBOF_TRACE_ERROR(EID(VSAMAP_STORE_FAILURE), "VSAMap Store failed, volumeId:{}",
                volumeId);
        }
    }

    return ret;
}

void
VSAMapManager::_WaitForMapAsyncFlushed(int volId)
{
    while (mapFlushStatus[volId] != VSAMapFlushState::FLUSH_DONE)
    {
        usleep(1);
    }
}

int
VSAMapManager::AsyncStore(void)
{
    int ret = 0;

    for (auto& volState : volumeMountState)
    {
        if (VolState::EXIST_UNLOADED != volState.second)
        {
            int volumeId = volState.first;
            EventSmartPtr callBackVSAMap = std::make_shared<MapFlushedEvent>(volumeId, this);
            mapFlushStatus[volumeId] = VSAMapFlushState::FLUSHING;
            ret = vsaMaps[volumeId]->Flush(callBackVSAMap);
            if (ret < 0)
            {
                IBOF_TRACE_ERROR(EID(MAPPER_FAILED), "Flush() for volumeId:{} Failed",
                    volumeId);
            }
            else
            {
                IBOF_TRACE_INFO(EID(MAPPER_SUCCESS), "Flush() for volumeId:{} Started",
                    volumeId);
            }
        }
    }
    mapFlushStatus[STRIPE_MAP_ID] = VSAMapFlushState::FLUSHING;

    return ret;
}

void
VSAMapManager::Close(void)
{
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        if ((vsaMaps[volumeId] != nullptr) && (vsaMaps[volumeId]->IsFileOpened()))
        {
            IBOF_TRACE_INFO(EID(MAPPER_SUCCESS), "Mapper closes MFS file for volumeId:{}",
                volumeId);
            vsaMaps[volumeId]->FileClose();
        }
    }
}

VSAMapContent*
VSAMapManager::GetVSAMapContent(int volID)
{
    return vsaMaps[volID];
}

int
VSAMapManager::_GetNumLoadedVolume(void)
{
    int loadedCnt = 0;

    for (auto& volState : volumeMountState)
    {
        if (VolState::EXIST_UNLOADED != volState.second)
        {
            ++loadedCnt;
        }
    }

    return loadedCnt;
}

void
VSAMapManager::MapAsyncFlushed(int mapId)
{
    IBOF_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "mapId:{} Flushed @MapAsyncFlushed",
                    mapId);
    mapFlushStatus[mapId] = VSAMapFlushState::FLUSH_DONE;
}

bool
VSAMapManager::AllMapsAsyncFlushed(void)
{
    bool allFlushed = true;

    for (auto& mapStatus : mapFlushStatus)
    {
        if (mapStatus.second != VSAMapFlushState::FLUSH_DONE)
        {
            allFlushed = false;
            break;
        }
    }

    return allFlushed;
}

void
VSAMapManager::SetVolumeManagerObject(VolumeManager* volumeManagerToUse)
{
    volumeManager = volumeManagerToUse;
}

int
VSAMapManager::EnableInternalAccess(int volID, int& caller)
{
    while (volMountStateLock[volID].try_lock() == false)
    {
        if (CALLER_EVENT == caller)
        {
            return -EID(MAP_LOAD_ONGOING);
        }
    }

    int ret = 0;
    VolMountStateIter iter;
    if (_IsVolumeExist(volID, iter) == false)
    {
        volMountStateLock[volID].unlock();
        return -EID(VSAMAP_LOAD_FAILURE);
    }

    // Unloaded volume case: Load & BG Mount
    if (iter->second == VolState::EXIST_UNLOADED)
    {
        std::string volName = "";
        if (CALLER_EVENT == caller)
        {
            if (LOADING == loadDoneFlag[volID])
            {
                volMountStateLock[volID].unlock();
                return -EID(MAP_LOAD_ONGOING);
            }
            volName = "CALLER_EVENT";
        }
        // In case of internal-loading, we don't know the volume size
        if (VolumeMounted(volName, "", volID, UNKNOWN_SIZE_BECAUSEOF_INTERNAL_LOAD, 0,
                0))
        {
            IBOF_TRACE_INFO(EID(MAPPER_SUCCESS),
                "VolumeId:{} Internal Mount Request Succeeded", volID);
        }
        else
        {
            IBOF_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE),
                "VolumeId:{} Internal Mount Request Failed", volID);
            ret = -EID(VSAMAP_LOAD_FAILURE);
        }
    }
    // Just Created volume case: BG Mount
    else if (iter->second == VolState::JUST_CREATED)
    {
        volumeMountState.erase(iter);
        volumeMountState.emplace(volID, VolState::BACKGROUND_MOUNTED);
        IBOF_TRACE_INFO(EID(MAPPER_SUCCESS), "VolumeId:{} was set as BG mounted",
            volID);
    }
    else // VolState::BACKGROUND_MOUNTED or VolState::FOREGROUND_MOUNTED
    {
        // Do nothing
    }

    volMountStateLock[volID].unlock();
    return ret;
}

bool
VSAMapManager::VolumeCreated(std::string volName, int volID,
    uint64_t volSizeByte,
    uint64_t maxiops, uint64_t maxbw)
{
    do
    {
        if (false == _PrepareVsaMapAndHeader(volID, volSizeByte, false))
        {
            IBOF_TRACE_ERROR(EID(MAPPER_FAILED), "Vsa map & Header Prepare Failed");
            break;
        }
        if (false == _PrepareInMemoryData(volID, volSizeByte))
        {
            IBOF_TRACE_ERROR(EID(MAPPER_FAILED), "In-memory Data Prepare Failed");
            break;
        }
        if (false == _VSAMapFileCreate(volID))
        {
            IBOF_TRACE_ERROR(EID(MAPPER_FAILED), "Vsa map File Creation Failed");
            break;
        }
        if (false == _VSAMapFileStore(volID))
        {
            IBOF_TRACE_ERROR(EID(MAPPER_FAILED), "Vsa map File Store Failed");
            break;
        }

        std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volID]);
        volumeMountState.emplace(volID, VolState::JUST_CREATED);
        IBOF_TRACE_INFO(EID(MAPPER_SUCCESS), "VolumeId:{} JUST_CREATED", volID);
        return true;
    } while (false);

    return false;
}

bool
VSAMapManager::VolumeLoaded(std::string name, int id, uint64_t totalSize,
    uint64_t maxiops, uint64_t maxbw)
{
    std::unique_lock<std::recursive_mutex> lock(volMountStateLock[id]);
    volumeMountState.emplace(id, VolState::EXIST_UNLOADED);
    IBOF_TRACE_INFO(EID(MAPPER_SUCCESS),
        "VolumeId:{} is inserted to volumeMountState as EXIST_UNLOADED @VolumeLoaded",
        id);
    return true;
}

bool
VSAMapManager::VolumeMounted(std::string volName, std::string subnqn,
    int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw)
{
    bool isUnknownSize = (volSizeByte == UNKNOWN_SIZE_BECAUSEOF_INTERNAL_LOAD);
    return _LoadVolumeMeta(volName, volID, volSizeByte, isUnknownSize);
}

bool
VSAMapManager::VolumeUnmounted(std::string volName, int volID)
{
    _DisableVsaMapAccess(volID);

    do
    {
        VSAMapContent*& vsaMap = vsaMaps[volID];
        EventSmartPtr callBackVSAMap = std::make_shared<MapFlushedEvent>(volID, this);
        mapFlushStatus[volID] = VSAMapFlushState::FLUSHING;

        int ret = vsaMap->Flush(callBackVSAMap);
        if (ret < 0)
        {
            IBOF_TRACE_ERROR(EID(VSAMAP_STORE_FAILURE),
                "ret:{} of vsaMap->Unload(), VolumeId:{} @VolumeUnmounted", ret, volID);
        }
        _WaitForMapAsyncFlushed(volID);

        std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volID]);
        VolMountStateIter iter;
        if (_IsVolumeExist(volID, iter))
        {
            if (iter->second == VolState::FOREGROUND_MOUNTED)
            {
                volumeMountState.erase(iter);
                volumeMountState.emplace(volID, VolState::BACKGROUND_MOUNTED);
                IBOF_TRACE_INFO(EID(MAPPER_SUCCESS),
                    "VolumeId:{} was set as BG_MOUNTED @VolumeUnmounted", volID);
            }
            else
            {
                IBOF_TRACE_WARN(EID(VSAMAP_UNMOUNT_FAILURE),
                    "volumeID:{} is Not FG_MOUNTED @VolumeUnmounted", volID);
            }
        }

        return true;
    } while (false);

    return false;
}

void
VSAMapManager::VolumeDetached(vector<int> volList)
{
    for (int volumeId : volList)
    {
        _DisableVsaMapAccess(volumeId);

        std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volumeId]);

        VolMountStateIter iter;
        if (_IsVolumeExist(volumeId, iter))
        {
            if (iter->second == VolState::FOREGROUND_MOUNTED)
            {
                volumeMountState.erase(iter);
                volumeMountState.emplace(volumeId, VolState::BACKGROUND_MOUNTED);
                IBOF_TRACE_INFO(EID(MAPPER_SUCCESS),
                    "VolumeId:{} was set as BG_MOUNTED @VolumeDetached", volumeId);
            }
            else
            {
                IBOF_TRACE_WARN(EID(VSAMAP_UNMOUNT_FAILURE),
                    "volumeId:{} is Not FG_MOUNTED @VolumeDetached", volumeId);
            }
        }
    }
}

bool
VSAMapManager::VolumeDeleted(std::string volName, int volID,
    uint64_t volSizeByte)
{
    std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volID]);
    IBOF_TRACE_INFO(EID(MAPPER_SUCCESS),
        "Starting VolumeDelete: volID:{}  volSizeByte:{}",
        volID, volSizeByte);

    VSAMapContent*& vsaMap = vsaMaps[volID];
    // Unloaded case: Load & BG Mount
    if (nullptr == vsaMap)
    {
        // We know the volume size but deal with internal loading
        if (false == _LoadVolumeMeta(volName, volID, volSizeByte, true))
        {
            IBOF_TRACE_WARN(EID(VSAMAP_LOAD_FAILURE),
                "VSAMap load failed, volumeID:{} @VolumeDeleted", volID);
            return false;
        }
    }

    if (vsaMap->DoesFileExist() == false)
    {
        IBOF_TRACE_ERROR(EID(NO_BLOCKMAP_MFS_FILE),
            "No MFS filename:{} for volName:{} @VolumeDeleted", vsaMap->GetFileName(),
            volName);
        return false;
    }

    // Mark all blocks in this volume up as Invalidated
    if (0 != vsaMap->InvalidateAllBlocks())
    {
        IBOF_TRACE_WARN(EID(VSAMAP_INVALIDATE_ALLBLKS_FAILURE),
            "VSAMap Invalidate all blocks Failed, volumeID:{} @VolumeDeleted", volID);
        return false;
    }
    if (0 != vsaMap->Unload())
    {
        IBOF_TRACE_WARN(EID(VSAMAP_UNLOAD_FAILURE),
            "VSAMap unload(store) failed, volumeID:{} @VolumeDeleted", volID);
        return false;
    }

    // file close and delete
    if (0 != vsaMap->FileClose())
    {
        IBOF_TRACE_WARN(EID(MFS_FILE_CLOSE_FAILED),
            "VSAMap File close failed, volumeID:{} @VolumeDeleted", volID);
        return false;
    }
    if (0 != vsaMap->DeleteMapFile())
    {
        IBOF_TRACE_WARN(EID(MFS_FILE_DELETE_FAILED),
            "VSAMap File delete failed, volumeID:{} @VolumeDeleted", volID);
        return false;
    }

    // clean up contents in dram
    delete vsaMap;
    vsaMap = nullptr;
    volumeMountState.erase(volID);

    return true;
}

bool
VSAMapManager::VolumeUpdated(std::string volName, int volID, uint64_t maxiops,
    uint64_t maxbw)
{
    return true;
}

bool
VSAMapManager::_PrepareVsaMapAndHeader(int volID, uint64_t& volSizeByte, bool isUnknownVolSize)
{
    int ret = 0;
    VSAMapContent*& vsaMapRef = vsaMaps[volID];
    assert(vsaMapRef == nullptr);

    vsaMapRef = new VSAMapContent(volID);
    if (isUnknownVolSize && (volSizeByte == 0)) // Internal loading case
    {
        uint64_t volSizebyVolMgr = 0;
        ret = volumeManager->GetVolumeSize(volID, volSizebyVolMgr);
        if (ret != 0)
        {
            IBOF_TRACE_WARN(EID(VSAMAP_HEADER_LOAD_FAILURE),
                "Getting volume Size failed, volumeID:{}  volSizebyVolMgr:{}", volID, volSizebyVolMgr);
            return false;
        }

        volSizeByte = volSizebyVolMgr;
        IBOF_TRACE_INFO(9999, "volID:{}  volSizebyVolMgr:{}", volID, volSizebyVolMgr);
    }
    uint64_t blkCnt = DivideUp(volSizeByte, ibofos::BLOCK_SIZE);

    return (0 == vsaMapRef->Prepare(blkCnt, volID));
}

bool
VSAMapManager::_PrepareInMemoryData(int volID, uint64_t volSizeByte)
{
    VSAMapContent*& vsaMapRef = vsaMaps[volID];
    assert(vsaMapRef != nullptr);

    uint64_t blkCnt = DivideUp(volSizeByte, ibofos::BLOCK_SIZE);

    return (0 == vsaMapRef->InMemoryInit(blkCnt, volID));
}

bool
VSAMapManager::_VSAMapFileCreate(int volID)
{
    VSAMapContent*& vsaMapRef = vsaMaps[volID];
    assert(vsaMapRef != nullptr);

    return (0 == vsaMapRef->CreateMapFile());
}

int
VSAMapManager::_VSAMapFileSyncLoad(int volID)
{
    VSAMapContent*& vsaMapRef = vsaMaps[volID];
    assert(vsaMapRef != nullptr);

    return vsaMapRef->LoadSync();
}

int
VSAMapManager::_VSAMapFileAsyncLoad(int volID)
{
    VSAMapContent*& vsaMapRef = vsaMaps[volID];
    assert(vsaMapRef != nullptr);

    unique_lock<std::mutex> lock(volAsyncLoadLock);
    loadDoneFlag[volID] = NOT_LOADED;

    IBOF_TRACE_INFO(EID(MAPPER_SUCCESS),
        "VSAMap Async Load Started, volID:{} @_VSAMapFileAsyncLoad", volID);
    int ret = vsaMapRef->LoadAsync(loadDoneWakeUp);
    if (ret < 0)
    {
        if (-EID(MAP_LOAD_COMPLETED) == ret)
        {
            loadDoneFlag[volID] = LOAD_DONE;
            ret = 0; // This is a normal case
            IBOF_TRACE_INFO(EID(MAPPER_START),
                "No mpage to Load, so VSAMap Async Load Finished, volID:{} @_VSAMapFileAsyncLoad",
                volID);
        }
        else
        {
            IBOF_TRACE_ERROR(EID(MAPPER_FAILED),
                "Error on AsyncLoad trigger, volID:{}  ret:{} @_VSAMapFileAsyncLoad", volID,
                ret);
        }
    }
    else
    {
        cvLoadDone.wait(lock, [&] { return LOAD_DONE == loadDoneFlag[volID]; });
        IBOF_TRACE_INFO(EID(MAPPER_START),
            "after waking up, VSAMap Async Load Finished, volID:{} @_VSAMapFileAsyncLoad",
            volID);
    }

    return ret;
}

int
VSAMapManager::_VSAMapFileAsyncLoadNoWait(int volID)
{
    VSAMapContent*& vsaMapRef = vsaMaps[volID];
    assert(vsaMapRef != nullptr);

    loadDoneFlag[volID] = LOADING;

    IBOF_TRACE_INFO(EID(MAPPER_SUCCESS),
        "VSAMap Async Load Started, volID:{} @VSAMapFileAsyncLoadNoWait",
        volID);
    int ret = vsaMapRef->LoadAsyncEvent(loadDone);
    if (ret < 0)
    {
        IBOF_TRACE_ERROR(EID(MAPPER_FAILED),
            "Error on AsyncLoad trigger, volID:{}  ret:{} @VSAMapFileAsyncLoadNoWait",
            volID, ret);
    }

    return ret;
}

bool
VSAMapManager::_VSAMapFileStore(int volID)
{
    VSAMapContent*& vsaMapRef = vsaMaps[volID];
    assert(vsaMapRef != nullptr);

    return (0 == vsaMapRef->StoreSync());
}

void
VSAMapManager::_EnableVsaMapAccess(int volID)
{
    isVsaMapAccessable[volID] = true;
}

void
VSAMapManager::_DisableVsaMapAccess(int volID)
{
    isVsaMapAccessable[volID] = false;
}

bool
VSAMapManager::IsVsaMapAccessible(int volID)
{
    return isVsaMapAccessable[volID];
}

bool
VSAMapManager::IsVSAMapLoaded(int volID)
{
    if (vsaMaps[volID] == nullptr)
    {
        return false;
    }

    if (vsaMaps[volID]->IsLoaded())
    {
        return true;
    }
    else
    {
        return (LOAD_DONE == loadDoneFlag[volID]);
    }
}

void
VSAMapManager::_WakeUpAfterLoadDone(int volID)
{
    std::unique_lock<std::mutex> lock(volAsyncLoadLock);
    IBOF_TRACE_INFO(EID(MAP_LOAD_COMPLETED),
        "volID:{} async load done, so wake up! @_WakeUpAfterLoadDone", volID);
    loadDoneFlag[volID] = LOAD_DONE;
    cvLoadDone.notify_all();
}

void
VSAMapManager::_AfterLoadDone(int volID)
{
    std::unique_lock<std::recursive_mutex> lock(volMountStateLock[volID]);
    loadDoneFlag[volID] = LOAD_DONE;
    IBOF_TRACE_INFO(EID(MAP_LOAD_COMPLETED),
        "volID:{} async load done @_AfterLoadDone", volID);

    volumeMountState.erase(volID);
    volumeMountState.emplace(volID, VolState::BACKGROUND_MOUNTED);
    IBOF_TRACE_INFO(EID(MAPPER_SUCCESS),
        "VolumeId:{} was loaded and set as BG_MOUNTED @_AfterLoadDone", volID);
}

bool
VSAMapManager::_IsVolumeExist(int volID, VolMountStateIter& iter)
{
    VolMountStateIter it = volumeMountState.find(volID);
    iter = it;

    if (it == volumeMountState.end())
    {
        IBOF_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE),
            "VolumeId:{} is not in volumeMountState", volID);
        int cnt = 0;
        for (auto& entry : volumeMountState)
        {
            ++cnt;
            IBOF_TRACE_ERROR(EID(VSAMAP_LOAD_FAILURE), "INFO - volumeMountState {}/{}  K:{}  V:{}",
                cnt, volumeMountState.size(), entry.first, entry.second);
        }
        return false;
    }

    return true;
}

bool
VSAMapManager::_LoadVolumeMeta(std::string volName, int volID, uint64_t volSizeByte,
    bool isUnknownVolSize)
{
    // If GC(EventWorker thread) is already loading the VSAMap of volID,
    // CLI thread has to wait until loading done
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
        volumeMountState.erase(iter);
        volumeMountState.emplace(volID, VolState::FOREGROUND_MOUNTED);
        _EnableVsaMapAccess(volID);
        IBOF_TRACE_INFO(EID(MAPPER_SUCCESS),
            "VolumeId:{} was set as FG_MOUNTED @VolumeMounted", volID);
        return true;
    }

    // Unloaded volume case: Load & Mount
    do
    {
        if (iter->second == VolState::EXIST_UNLOADED)
        {
            if (_PrepareVsaMapAndHeader(volID, volSizeByte, isUnknownVolSize) == false)
            {
                IBOF_TRACE_ERROR(EID(MAPPER_FAILED),
                    "Vsa map & Header Prepare Failed @VolumeMounted");
                break;
            }
            if (_PrepareInMemoryData(volID, volSizeByte) == false)
            {
                IBOF_TRACE_ERROR(EID(MAPPER_FAILED),
                    "In-memory Data Prepare Failed @VolumeMounted");
                break;
            }
            // GC: EventWorker thread
            if (isUnknownVolSize && ("CALLER_EVENT" == volName))
            {
                if (_VSAMapFileAsyncLoadNoWait(volID) != 0)
                {
                    IBOF_TRACE_ERROR(EID(MAPPER_FAILED),
                        "Vsa map File Async Load Trigger Failed @VolumeMounted");
                    break;
                }
            }
            else
            {
                if (_VSAMapFileAsyncLoad(volID) != 0)
                {
                    IBOF_TRACE_ERROR(EID(MAPPER_FAILED),
                        "Vsa map File Async Load Failed @VolumeMounted");
                    break;
                }
                volumeMountState.erase(iter);
                // Replay: CLI thread
                if (isUnknownVolSize)
                {
                    volumeMountState.emplace(volID, VolState::BACKGROUND_MOUNTED);
                    IBOF_TRACE_INFO(EID(MAPPER_SUCCESS),
                        "VolumeId:{} was loaded and set as BG_MOUNTED @VolumeMounted", volID);
                }
                // User Action: CLI thread
                else
                {
                    volumeMountState.emplace(volID, VolState::FOREGROUND_MOUNTED);
                    _EnableVsaMapAccess(volID);
                    IBOF_TRACE_INFO(EID(MAPPER_SUCCESS),
                        "VolumeId:{} was loaded and set as FG_MOUNTED @VolumeMounted", volID);
                }
            }

            return true;
        }
    } while (false);

    return false;
}

} // namespace ibofos
