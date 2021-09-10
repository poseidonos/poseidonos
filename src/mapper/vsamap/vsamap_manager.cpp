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

#include <string>
#include <vector>

namespace pos
{
VSAMapManager::VSAMapManager(MapperAddressInfo* info)
: addrInfo(info),
  numWriteIssuedCount(0),
  numLoadIssuedCount(0)
{
    for (int volId = 0; volId < MAX_VOLUME_COUNT; ++volId)
    {
        vsaMaps[volId] = nullptr;
        mapFlushState[volId] = MapFlushState::FLUSH_DONE;
        mapLoadState[volId] = MapLoadState::LOAD_DONE;
        isVsaMapInternalAccessable[volId] = false;
    }
}

VSAMapManager::~VSAMapManager(void)
{
    Dispose();
}

int
VSAMapManager::Init(void)
{
    for (int volId = 0; volId < MAX_VOLUME_COUNT; ++volId)
    {
        isVsaMapAccessable[volId] = false;
        mapFlushState[volId] = MapFlushState::FLUSH_DONE;
        mapLoadState[volId] = MapLoadState::LOAD_DONE;
    }
    numWriteIssuedCount = 0;
    numLoadIssuedCount = 0;
    return 0;
}

void
VSAMapManager::Dispose(void)
{
    WaitAllPendingIoDone();
    for (int volId = 0; volId < MAX_VOLUME_COUNT; ++volId)
    {
        if (vsaMaps[volId] != nullptr)
        {
            delete vsaMaps[volId];
            vsaMaps[volId] = nullptr;
        }
    }
}

bool
VSAMapManager::CreateVsaMapContent(int volId, uint64_t volSizeByte, bool delVol)
{
    assert(vsaMaps[volId] == nullptr);
    vsaMaps[volId] = new VSAMapContent(volId, addrInfo->GetArrayId());
    uint64_t blkCnt = DivideUp(volSizeByte, pos::BLOCK_SIZE);
    do
    {
        if (vsaMaps[volId]->InMemoryInit(volId, blkCnt, addrInfo->GetMpageSize()) != 0)
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VSAMap] Vsa map In-memory Data Prepare Failed, volume:{} array:{}", volId, addrInfo->GetArrayName());
            break;
        }
        int ret = vsaMaps[volId]->OpenMapFile();
        if ((delVol == false) && (ret == EID(NEED_TO_INITIAL_STORE)))
        {
            EventSmartPtr callBackVSAMap = std::make_shared<MapFlushedEvent>(volId, this);
            mapFlushState[volId] = MapFlushState::FLUSHING;
            numWriteIssuedCount++;
            POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VSAMap] Issue Flush Header, volume:{} array:{}", volId, addrInfo->GetArrayName());
            ret = vsaMaps[volId]->FlushHeader(callBackVSAMap);
            if (ret < 0)
            {
                numWriteIssuedCount--;
                mapFlushState[volId] = MapFlushState::FLUSH_DONE;
                POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VSAMap] Failed to Initial Store VSAMap File, array:{}", addrInfo->GetArrayName());
                break;
            }
            else
            {
                WaitVolumePendingIoDone(volId);
                return true;
            }
        }
        else if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VSAMap] failed to create Vsa map File, volume:{}, array:{}", volId, addrInfo->GetArrayName());
            break;
        }
        else
        {
            return true;
        }
    } while (false);

    delete vsaMaps[volId];
    vsaMaps[volId] = nullptr;
    return false;
}

int
VSAMapManager::LoadVSAMapFile(int volId)
{
    assert(vsaMaps[volId] != nullptr);
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VSAMap] Issue Load VSAMap, volId:{}, array:{}", volId, addrInfo->GetArrayName());
    AsyncLoadCallBack cbLoadDone = std::bind(&VSAMapManager::_MapLoadDone, this, std::placeholders::_1);
    mapLoadState[volId] = MapLoadState::LOADING;
    numLoadIssuedCount++;
    int ret = vsaMaps[volId]->Load(cbLoadDone);
    if (ret < 0)
    {
        mapLoadState[volId] = MapLoadState::LOAD_DONE;
        numLoadIssuedCount--;
        if (-EID(MAP_LOAD_COMPLETED) == ret)
        {
            ret = 0; // This is a normal case
            POS_TRACE_INFO(EID(MAPPER_START), "[Mapper VSAMap] No mpage to Load, so VSAMap Load Finished, volId:{}, array:{}", volId, addrInfo->GetArrayName());
        }
        else
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VSAMap] Error on Load trigger, volId:{}, array:{}, ret:{}", volId, addrInfo->GetArrayName(), ret);
        }
    }
    return ret;
}

int
VSAMapManager::FlushMap(int volId)
{
    EventSmartPtr callBackVSAMap = std::make_shared<MapFlushedEvent>(volId, this);
    mapFlushState[volId] = MapFlushState::FLUSHING;
    numWriteIssuedCount++;
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VSAMap] Issue Flush VSAMap, volume :{}, array:{}", volId, addrInfo->GetArrayName());
    int ret = vsaMaps[volId]->FlushTouchedPages(callBackVSAMap);
    if (ret < 0)
    {
        mapFlushState[volId] = MapFlushState::FLUSH_DONE;
        numWriteIssuedCount--;
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VSAMap] failed to flush vsamap, volumeId:{}, array:{}", volId, addrInfo->GetArrayName());
    }
    else
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VSAMap] flush vsamp started volumeId:{}, array:{}", volId, addrInfo->GetArrayName());
    }
    return ret;
}

int
VSAMapManager::FlushAllMaps(void)
{
    int ret = 0;
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VSAMap] Issue Flush All VSAMaps, array:{}", addrInfo->GetArrayName());
    for (int volId = 0; volId < MAX_VOLUME_COUNT; ++volId)
    {
        if (isVsaMapInternalAccessable[volId] == true)
        {
            EventSmartPtr callBackVSAMap = std::make_shared<MapFlushedEvent>(volId, this);
            mapFlushState[volId] = MapFlushState::FLUSHING;
            numWriteIssuedCount++;
            ret = vsaMaps[volId]->FlushTouchedPages(callBackVSAMap);
            if (ret < 0)
            {
                mapFlushState[volId] = MapFlushState::FLUSH_DONE;
                numWriteIssuedCount--;
                POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VSAMap] failed to flush vsamap, volumeId:{}, array:{}", volId, addrInfo->GetArrayName());
            }
            else
            {
                POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VSAMap] flush vsamp started volumeId:{}, array:{}", volId, addrInfo->GetArrayName());
            }
        }
    }
    return ret;
}

int
VSAMapManager::InvalidateAllBlocks(int volId)
{
    return vsaMaps[volId]->InvalidateAllBlocks();
}

bool
VSAMapManager::NeedToDeleteFile(int volId)
{
    if (vsaMaps[volId]->DoesFileExist() == false)
    {
        return false;
    }
    return true;
}

int
VSAMapManager::DeleteVSAMap(int volId)
{
    bool ret = vsaMaps[volId]->DeleteMapFile();
    if (ret == 0)
    {
        delete vsaMaps[volId];
        vsaMaps[volId] = nullptr;
    }
    return ret;
}

void
VSAMapManager::WaitAllPendingIoDone(void)
{
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "[Mapper VSAMap] PendingWriteCnt:{}, PendingReadCnt:{}", numWriteIssuedCount, numLoadIssuedCount);
    while ((numWriteIssuedCount + numLoadIssuedCount) != 0);
}

void
VSAMapManager::WaitLoadPendingIoDone(void)
{
    while (numLoadIssuedCount != 0);
}

void
VSAMapManager::WaitWritePendingIoDone(void)
{
    while (numWriteIssuedCount != 0);
}

void
VSAMapManager::WaitVolumePendingIoDone(int volId)
{
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "[Mapper VSAMap] PendingWriteCnt:{}, PendingReadCnt:{}", numWriteIssuedCount, numLoadIssuedCount);
    while ((mapLoadState[volId] != MapLoadState::LOAD_DONE) || (mapFlushState[volId] != MapFlushState::FLUSH_DONE));
}

void
VSAMapManager::MapFlushDone(int mapId)
{
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "[Mapper VSAMap] mapId:{} WritePendingCnt:{} Flushed Done", mapId, numWriteIssuedCount);
    assert(numWriteIssuedCount > 0);
    numWriteIssuedCount--;
    mapFlushState[mapId] = MapFlushState::FLUSH_DONE;
}

int
VSAMapManager::GetVSAs(int volId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray)
{
    if (false == isVsaMapAccessable[volId])
    {
        POS_TRACE_WARN(EID(VSAMAP_NOT_ACCESSIBLE), "[Mapper VSAMap] VolumeId:{}, array:{} is not accessible, maybe unmounted", volId, addrInfo->GetArrayName());
        for (uint32_t blkIdx = 0; blkIdx < numBlks; ++blkIdx)
        {
            vsaArray[blkIdx] = UNMAP_VSA;
        }
        return -EID(VSAMAP_NOT_ACCESSIBLE);
    }

    VSAMapContent* vsaMap = vsaMaps[volId];
    for (uint32_t blkIdx = 0; blkIdx < numBlks; ++blkIdx)
    {
        BlkAddr targetRba = startRba + blkIdx;
        vsaArray[blkIdx] = vsaMap->GetEntry(targetRba);
    }
    return 0;
}

int
VSAMapManager::SetVSAs(int volId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    if (false == isVsaMapAccessable[volId])
    {
        POS_TRACE_WARN(EID(VSAMAP_NOT_ACCESSIBLE), "[Mapper VSAMap] VolumeId:{} is not accessible, maybe unmounted", volId);
        return -EID(VSAMAP_NOT_ACCESSIBLE);
    }
    return _UpdateVsaMap(volId, startRba, virtualBlks);
}

VirtualBlkAddr
VSAMapManager::GetVSAWoCond(int volId, BlkAddr rba)
{
    assert(mapLoadState[volId] == MapLoadState::LOAD_DONE);
    return vsaMaps[volId]->GetEntry(rba);
}

int
VSAMapManager::SetVSAsWoCond(int volId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    assert(mapLoadState[volId] == MapLoadState::LOAD_DONE);
    return _UpdateVsaMap(volId, startRba, virtualBlks);
}

VirtualBlkAddr
VSAMapManager::GetRandomVSA(BlkAddr rba)
{
    VirtualBlkAddr vsa;
    vsa.stripeId = rba / addrInfo->blksPerStripe;
    vsa.offset = rba % addrInfo->blksPerStripe;
    return vsa;
}

MpageList
VSAMapManager::GetDirtyVsaMapPages(int volId, BlkAddr startRba, uint64_t numBlks)
{
    return vsaMaps[volId]->GetDirtyPages(startRba, numBlks);
}

int64_t
VSAMapManager::GetNumUsedBlks(int volId)
{
    return vsaMaps[volId]->GetNumUsedBlks();
}

bool
VSAMapManager::IsVsaMapAccessible(int volId)
{
    return isVsaMapAccessable[volId];
}

void
VSAMapManager::EnableVsaMapAccess(int volId)
{
    isVsaMapAccessable[volId] = true;
}

void
VSAMapManager::DisableVsaMapAccess(int volId)
{
    isVsaMapAccessable[volId] = false;
}

void
VSAMapManager::EnableVsaMapInternalAccess(int volId)
{
    // POS_TRACE_INFO(EID(DELETE_VOLUME), "[Mapper VSAMap] Enable Internal VsaMap Access volumeId:{}, array:{}", volId, addrInfo->GetArrayName());
    isVsaMapInternalAccessable[volId] = true;
}

void
VSAMapManager::DisableVsaMapInternalAccess(int volId)
{
    // POS_TRACE_INFO(EID(DELETE_VOLUME), "[Mapper VSAMap] Disable Internal VsaMap Access volumeId:{}, array:{}", volId, addrInfo->GetArrayName());
    isVsaMapInternalAccessable[volId] = false;
}

void
VSAMapManager::_MapLoadDone(int volId)
{
    POS_TRACE_INFO(EID(MAP_LOAD_COMPLETED), "[Mapper VSAMap] Load Done volId:{} array:{}, ReadPendingCnt:{}", volId, addrInfo->GetArrayName(), numLoadIssuedCount);
    assert(numLoadIssuedCount > 0);
    numLoadIssuedCount--;
    mapLoadState[volId] = MapLoadState::LOAD_DONE;
}

int
VSAMapManager::_UpdateVsaMap(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    int ret = 0;
    VSAMapContent* vsaMap = vsaMaps[volumeId];

    for (uint32_t blkIdx = 0; blkIdx < virtualBlks.numBlks; blkIdx++)
    {
        VirtualBlkAddr targetVsa = {.stripeId = virtualBlks.startVsa.stripeId,
            .offset = virtualBlks.startVsa.offset + blkIdx};
        BlkAddr targetRba = startRba + blkIdx;
        ret = vsaMap->SetEntry(targetRba, targetVsa);
        if (ret < 0)
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::VSAMAP_SET_FAILURE, "[Mapper VSAMap] failed to update VSAMap Info, volumeId:{}  targetRba:{}  targetVsa.sid:{}  targetVsa.offset:{}",
                            volumeId, targetRba, targetVsa.stripeId, targetVsa.offset);
            break;
        }
    }
    return ret;
}

} // namespace pos
