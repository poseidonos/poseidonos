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
#include "src/event_scheduler/event_scheduler.h"
#include "src/mapper/include/mapper_const.h"
#include "src/mapper/map_flushed_event.h"
#include "src/mapper/vsamap/vsamap_manager.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

#include <string>
#include <vector>

namespace pos
{
VSAMapManager::VSAMapManager(TelemetryPublisher* tp_, EventScheduler* eventSched, VSAMapContent* vsaMap, MapperAddressInfo* info)
: addrInfo(info),
  vsaMaps(),
  mapFlushState(),
  mapLoadState(),
  isVsaMapAccessable(),
  isVsaMapInternalAccessable(),
  numWriteIssuedCount(0),
  numLoadIssuedCount(0),
  tp(tp_)
{
    // only for UT
    eventScheduler = eventSched;
    for (int volId = 0; volId < MAX_VOLUME_COUNT; ++volId)
    {
        vsaMaps[volId] = nullptr;
        mapFlushState[volId] = MapFlushState::FLUSH_DONE;
        mapLoadState[volId] = MapLoadState::LOAD_DONE;
        isVsaMapInternalAccessable[volId] = false;
        isVsaMapAccessable[volId] = false;
    }
    vsaMaps[0] = vsaMap;
}

VSAMapManager::VSAMapManager(TelemetryPublisher* tp_, MapperAddressInfo* info)
: addrInfo(info),
  vsaMaps(),
  mapFlushState(),
  mapLoadState(),
  isVsaMapAccessable(),
  isVsaMapInternalAccessable(),
  numWriteIssuedCount(0),
  numLoadIssuedCount(0),
  tp(tp_)
{
    eventScheduler = EventSchedulerSingleton::Instance();
    for (int volId = 0; volId < MAX_VOLUME_COUNT; ++volId)
    {
        vsaMaps[volId] = nullptr;
        mapFlushState[volId] = MapFlushState::FLUSH_DONE;
        mapLoadState[volId] = MapLoadState::LOAD_DONE;
        isVsaMapInternalAccessable[volId] = false;
        isVsaMapAccessable[volId] = false;
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
    for (int volId = 0; volId < MAX_VOLUME_COUNT; ++volId)
    {
        if (vsaMaps[volId] != nullptr)
        {
            delete vsaMaps[volId];
            vsaMaps[volId] = nullptr;
        }
    }
}

int
VSAMapManager::CreateVsaMapContent(VSAMapContent* vm, int volId, uint64_t volSizeByte, bool delVol)
{
    assert(vsaMaps[volId] == nullptr);
    if (vm != nullptr)
    {
        // for UT
        vsaMaps[volId] = vm;
    }
    else
    {
        vsaMaps[volId] = new VSAMapContent(volId, addrInfo);
    }
    uint64_t blkCnt = DivideUp(volSizeByte, (uint64_t)pos::BLOCK_SIZE);
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
            POSMetricValue v;
            v.gauge = numWriteIssuedCount;
            tp->PublishData(TEL33008_MAP_VSA_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
            POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VSAMap] Issue Flush Header, volume:{} array:{}", volId, addrInfo->GetArrayName());
            ret = vsaMaps[volId]->FlushHeader(callBackVSAMap);
            if (ret < 0)
            {
                numWriteIssuedCount--;
                v.gauge = numWriteIssuedCount;
                tp->PublishData(TEL33008_MAP_VSA_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
                mapFlushState[volId] = MapFlushState::FLUSH_DONE;
                POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VSAMap] Failed to Initial Store VSAMap File, array:{}", addrInfo->GetArrayName());
                break;
            }
            else
            {
                WaitVolumePendingIoDone(volId);
                return 0;
            }
        }
        else if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VSAMap] failed to create Vsa map File, volume:{}, array:{}", volId, addrInfo->GetArrayName());
            break;
        }
        else
        {
            return 0;
        }
    } while (false);

    delete vsaMaps[volId];
    vsaMaps[volId] = nullptr;
    return -1;
}

int
VSAMapManager::LoadVSAMapFile(int volId)
{
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VSAMap] Issue Load VSAMap, volId:{}, array:{}", volId, addrInfo->GetArrayName());
    assert(vsaMaps[volId] != nullptr);
    AsyncLoadCallBack cbLoadDone = std::bind(&VSAMapManager::_MapLoadDone, this, std::placeholders::_1);
    mapLoadState[volId] = MapLoadState::LOADING;
    numLoadIssuedCount++;
    POSMetricValue v;
    v.gauge = numLoadIssuedCount;
    tp->PublishData(TEL33007_MAP_VSA_LOAD_PENDINGIO_CNT, v, MT_GAUGE);
    int ret = vsaMaps[volId]->Load(cbLoadDone);
    if (ret < 0)
    {
        mapLoadState[volId] = MapLoadState::LOAD_DONE;
        numLoadIssuedCount--;
        v.gauge = numLoadIssuedCount;
        tp->PublishData(TEL33007_MAP_VSA_LOAD_PENDINGIO_CNT, v, MT_GAUGE);
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
VSAMapManager::FlushDirtyPagesGiven(int volId, MpageList dirtyPages, EventSmartPtr cb)
{
    if (mapFlushState[volId] != MapFlushState::FLUSH_DONE)
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_COMPLETED), "[MAPPER VSAMap FlushDirtyPagesGiven] Failed to Issue Flush, Another Flush is still progressing in volume:{}, issuedCount:{}", volId, numWriteIssuedCount);
        return -EID(MAP_FLUSH_IN_PROGRESS);
    }
    int cnt = dirtyPages.size();
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VSAMap FlushDirtyPagesGiven] Issue Flush VSAMap, volume :{}, array:{}, numdirtyPages:{}", volId, addrInfo->GetArrayName(), cnt);
    POSMetricValue v;
    v.gauge = cnt;
    tp->PublishData(TEL33010_MAP_VSA_FLUSHED_DIRTYPAGE_CNT, v, MT_GAUGE);
    assert(vsaMaps[volId] != nullptr);
    assert(vsaMaps[volId]->GetCallback() == nullptr);
    vsaMaps[volId]->SetCallback(cb);

    EventSmartPtr callBackVSAMap = std::make_shared<MapFlushedEvent>(volId, this);
    mapFlushState[volId] = MapFlushState::FLUSHING;
    numWriteIssuedCount++;
    v.gauge = numWriteIssuedCount;
    tp->PublishData(TEL33008_MAP_VSA_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
    int ret = vsaMaps[volId]->FlushDirtyPagesGiven(dirtyPages, callBackVSAMap);
    if (ret < 0)
    {
        mapFlushState[volId] = MapFlushState::FLUSH_DONE;
        numWriteIssuedCount--;
        v.gauge = numWriteIssuedCount;
        tp->PublishData(TEL33008_MAP_VSA_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VSAMap FlushDirtyPagesGiven] failed to flush vsamap, volumeId:{}, array:{}", volId, addrInfo->GetArrayName());
    }
    else
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VSAMap FlushDirtyPagesGiven] flush vsamp started volumeId:{}, array:{}", volId, addrInfo->GetArrayName());
    }
    return ret;
}

int
VSAMapManager::FlushTouchedPages(int volId, EventSmartPtr cb)
{
    if (mapFlushState[volId] != MapFlushState::FLUSH_DONE)
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_COMPLETED), "[MAPPER VSAMap FlushTouchedPages] Failed to Issue Flush, Another Flush is still progressing in volume:{}, issuedCount:{}", volId, numWriteIssuedCount);
        return -EID(MAP_FLUSH_IN_PROGRESS);
    }
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VSAMap FlushTouchedPages] Issue Flush VSAMap, volume :{}, array:{}", volId, addrInfo->GetArrayName());
    assert(vsaMaps[volId] != nullptr);
    assert(vsaMaps[volId]->GetCallback() == nullptr);
    vsaMaps[volId]->SetCallback(cb);

    EventSmartPtr callBackVSAMap = std::make_shared<MapFlushedEvent>(volId, this);
    mapFlushState[volId] = MapFlushState::FLUSHING;
    numWriteIssuedCount++;
    POSMetricValue v;
    v.gauge = numWriteIssuedCount;
    tp->PublishData(TEL33008_MAP_VSA_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
    int ret = vsaMaps[volId]->FlushTouchedPages(callBackVSAMap);
    if (ret < 0)
    {
        mapFlushState[volId] = MapFlushState::FLUSH_DONE;
        numWriteIssuedCount--;
        v.gauge = numWriteIssuedCount;
        tp->PublishData(TEL33008_MAP_VSA_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VSAMap FlushTouchedPages] failed to flush vsamap, volumeId:{}, array:{}", volId, addrInfo->GetArrayName());
    }
    else
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VSAMap FlushTouchedPages] flush vsamp started volumeId:{}, array:{}", volId, addrInfo->GetArrayName());
    }
    return ret;
}

int
VSAMapManager::FlushAllMaps(void)
{
    int ret = 0;
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper VSAMap FlushAllMaps] Issue Flush All VSAMaps, array:{}", addrInfo->GetArrayName());
    for (int volId = 0; volId < MAX_VOLUME_COUNT; ++volId)
    {
        if ((isVsaMapInternalAccessable[volId] == true) && (vsaMaps[volId] != nullptr))
        {
            EventSmartPtr callBackVSAMap = std::make_shared<MapFlushedEvent>(volId, this);
            mapFlushState[volId] = MapFlushState::FLUSHING;
            numWriteIssuedCount++;
            POSMetricValue v;
            v.gauge = numWriteIssuedCount;
            tp->PublishData(TEL33008_MAP_VSA_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
            ret = vsaMaps[volId]->FlushTouchedPages(callBackVSAMap);
            if (ret < 0)
            {
                mapFlushState[volId] = MapFlushState::FLUSH_DONE;
                numWriteIssuedCount--;
                POSMetricValue v;
                v.gauge = numWriteIssuedCount;
                tp->PublishData(TEL33008_MAP_VSA_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
                POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper VSAMap FlushAllMaps] failed to flush vsamap, volumeId:{}, array:{}", volId, addrInfo->GetArrayName());
            }
            else
            {
                POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper VSAMap FlushAllMaps] flush vsamp started volumeId:{}, array:{}", volId, addrInfo->GetArrayName());
            }
        }
    }
    return ret;
}

int
VSAMapManager::InvalidateAllBlocks(int volId, ISegmentCtx* segmentCtx)
{
    assert(vsaMaps[volId] != nullptr);
    return vsaMaps[volId]->InvalidateAllBlocks(segmentCtx);
}

bool
VSAMapManager::NeedToDeleteFile(int volId)
{
    assert(vsaMaps[volId] != nullptr);
    if (vsaMaps[volId]->DoesFileExist() == false)
    {
        return false;
    }
    return true;
}

int
VSAMapManager::DeleteVSAMap(int volId)
{
    assert(vsaMaps[volId] != nullptr);
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
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "[Mapper VSAMap] WaitAllPendingIoDone PendingWriteCnt:{}, PendingReadCnt:{}", numWriteIssuedCount, numLoadIssuedCount);
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
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "[Mapper VSAMap] WaitPendingIoDone Vol:{}, PendingWriteCnt:{}, PendingReadCnt:{}", volId, numWriteIssuedCount, numLoadIssuedCount);
    while ((addrInfo->IsUT() == false) && ((mapLoadState[volId] != MapLoadState::LOAD_DONE) || (mapFlushState[volId] != MapFlushState::FLUSH_DONE)));
}

bool
VSAMapManager::IsVolumeLoaded(int volId)
{
    return (mapLoadState[volId] == MapLoadState::LOAD_DONE);
}

void
VSAMapManager::MapFlushDone(int mapId)
{
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "[Mapper VSAMap] mapId:{} WritePendingCnt:{} Flushed Done", mapId, numWriteIssuedCount);
    EventSmartPtr callback = vsaMaps[mapId]->GetCallback();
    if (callback != nullptr)
    {
        eventScheduler->EnqueueEvent(callback);
        vsaMaps[mapId]->SetCallback(nullptr);
    }
    assert(numWriteIssuedCount > 0);
    numWriteIssuedCount--;
    POSMetricValue v;
    v.gauge = numWriteIssuedCount;
    tp->PublishData(TEL33008_MAP_VSA_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
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
    int blksPerStripe = addrInfo->GetBlksPerStripe();
    vsa.stripeId = rba / blksPerStripe;
    vsa.offset = rba % blksPerStripe;
    return vsa;
}

MpageList
VSAMapManager::GetDirtyVsaMapPages(int volId, BlkAddr startRba, uint64_t numBlks)
{
    assert(vsaMaps[volId] != nullptr);
    return vsaMaps[volId]->GetDirtyPages(startRba, numBlks);
}

int64_t
VSAMapManager::GetNumUsedBlks(int volId)
{
    assert(vsaMaps[volId] != nullptr);
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

bool
VSAMapManager::IsVsaMapInternalAccesible(int volId)
{
    return isVsaMapInternalAccessable[volId];
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

int
VSAMapManager::Dump(int volId, std::string fileName)
{
    assert(vsaMaps[volId] != nullptr);
    return vsaMaps[volId]->Dump(fileName);
}

int
VSAMapManager::DumpLoad(int volId, std::string fileName)
{
    assert(vsaMaps[volId] != nullptr);
    return vsaMaps[volId]->DumpLoad(fileName);
}

VSAMapContent*
VSAMapManager::GetVSAMapContent(int volId)
{
    assert(vsaMaps[volId] != nullptr);
    return vsaMaps[volId];
}

void
VSAMapManager::SetVSAMapContent(int volId, VSAMapContent* content)
{
    // only for UT
    if (vsaMaps[volId] != nullptr)
    {
        delete vsaMaps[volId];
    }
    vsaMaps[volId] = content;
}

void
VSAMapManager::_MapLoadDone(int volId)
{
    POS_TRACE_INFO(EID(MAP_LOAD_COMPLETED), "[Mapper VSAMap] Load Done volId:{} array:{}, ReadPendingCnt:{}", volId, addrInfo->GetArrayName(), numLoadIssuedCount);
    assert(numLoadIssuedCount > 0);
    numLoadIssuedCount--;
    mapLoadState[volId] = MapLoadState::LOAD_DONE;
    POSMetricValue v;
    v.gauge = numLoadIssuedCount;
    tp->PublishData(TEL33007_MAP_VSA_LOAD_PENDINGIO_CNT, v, MT_GAUGE);
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
