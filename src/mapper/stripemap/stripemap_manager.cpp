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

#include "src/mapper/stripemap/stripemap_manager.h"

#include <memory>
#include <string>
#include <tuple>

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator/stripe/stripe.h"
#include "src/allocator_service/allocator_service.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/mapper/address/mapper_address_info.h"
#include "src/mapper/map_flushed_event.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
StripeMapManager::StripeMapManager(TelemetryPublisher* tp_, StripeMapContent* cont, EventScheduler* eventSched, MapperAddressInfo* info)
: stripeMap(cont),
  addrInfo(info),
  numLoadIssuedCount(0),
  numWriteIssuedCount(0),
  callback(nullptr),
  tp(tp_)
{
    eventScheduler = eventSched;
    if (eventScheduler == nullptr)
    {
        eventScheduler = EventSchedulerSingleton::Instance();
    }
    pthread_rwlock_init(&stripeMapLock, nullptr);
}

StripeMapManager::StripeMapManager(TelemetryPublisher* tp_, EventScheduler* eventSched, MapperAddressInfo* info)
: StripeMapManager(tp_, nullptr, eventSched, info)
{
    eventScheduler = eventSched;
    if (eventScheduler == nullptr)
    {
        eventScheduler = EventSchedulerSingleton::Instance();
    }
    pthread_rwlock_init(&stripeMapLock, nullptr);
}
// LCOV_EXCL_START
StripeMapManager::~StripeMapManager(void)
{
    Dispose();
}
// LCOV_EXCL_STOP
int
StripeMapManager::Init(void)
{
    numWriteIssuedCount = 0;
    numLoadIssuedCount = 0;
    if (stripeMap == nullptr)
    {
        stripeMap = new StripeMapContent(STRIPE_MAP_ID, addrInfo);
    }
    stripeMap->InMemoryInit(addrInfo->GetMaxVSID(), addrInfo->GetMpageSize());
    int ret = stripeMap->OpenMapFile();
    if (ret == EID(NEED_TO_INITIAL_STORE))
    {
        EventSmartPtr eventStripeMap = std::make_shared<MapFlushedEvent>(STRIPE_MAP_ID, this);
        numWriteIssuedCount++;
        POSMetricValue v;
        v.gauge = numWriteIssuedCount;
        tp->PublishData(TEL33009_MAP_STRIPE_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
        POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper StripeMap] Issue Flush Header, array:{}, arrayId:{}",
            addrInfo->GetArrayName(), addrInfo->GetArrayId());
        ret = stripeMap->FlushHeader(eventStripeMap);
        if (ret < 0)
        {
            numWriteIssuedCount--;
            v.gauge = numWriteIssuedCount;
            tp->PublishData(TEL33009_MAP_STRIPE_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper StripeMap] Failed to Initial Store StripeMap File, array:{}, arrayId:{}",
                addrInfo->GetArrayName(), addrInfo->GetArrayId());
        }
        else
        {
            WaitAllPendingIoDone();
        }
    }
    else if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper StripeMap] Failed to create StripeMap File, array:{}, arrayId:{}",
            addrInfo->GetArrayName(), addrInfo->GetArrayId());
    }
    else
    {
        ret = LoadStripeMapFile();
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MAPPER_START), "[Mapper StripeMap] Failed to load StripeMap File, array{}, arrayId:{}",
                addrInfo->GetArrayName(), addrInfo->GetArrayId());
        }
    }
    return ret;
}

void
StripeMapManager::Dispose(void)
{
    if (stripeMap != nullptr)
    {
        delete stripeMap;
        stripeMap = nullptr;
    }
}

int
StripeMapManager::LoadStripeMapFile(void)
{
    AsyncLoadCallBack cb = std::bind(&StripeMapManager::_MapLoadDone, this, std::placeholders::_1);
    numLoadIssuedCount++;
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper StripeMap] Issue Load StripeMap, array:{}, arrayId:{}",
        addrInfo->GetArrayName(), addrInfo->GetArrayId());
    int ret = stripeMap->Load(cb);
    if (ret == ERRID(MAP_LOAD_COMPLETED))
    {
        numLoadIssuedCount--;
        POS_TRACE_ERROR(EID(MAPPER_START), "[Mapper StripeMap] failed To Load StripeMap");
        return ret;
    }

    _WaitLoadIoDone();
    POS_TRACE_INFO(EID(MAPPER_START), "[Mapper StripeMap] StripeMap Loaded, array:{}, arrayId:{}",
        addrInfo->GetArrayName(), addrInfo->GetArrayId());
    return ret;
}

int
StripeMapManager::FlushDirtyPagesGiven(MpageList dirtyPages, EventSmartPtr cb)
{
    if (numWriteIssuedCount != 0)
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_COMPLETED), "[MAPPER StripeMap FlushDirtyPagesGiven] Failed to Issue Flush, Another Flush is still progressing, issuedCount:{}", numWriteIssuedCount);
        return ERRID(MAP_FLUSH_IN_PROGRESS);
    }

    assert(callback == nullptr);
    callback = cb;
    EventSmartPtr eventStripeMap = std::make_shared<MapFlushedEvent>(STRIPE_MAP_ID, this);
    numWriteIssuedCount++;
    POSMetricValue v;
    v.gauge = numWriteIssuedCount;
    tp->PublishData(TEL33009_MAP_STRIPE_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper StripeMap FlushDirtyPagesGiven] Issue Flush StripeMap, array:{}, arrayId:{}",
        addrInfo->GetArrayName(), addrInfo->GetArrayId());
    int ret = stripeMap->FlushDirtyPagesGiven(dirtyPages, eventStripeMap);
    if (ret < 0)
    {
        numWriteIssuedCount--;
        v.gauge = numWriteIssuedCount;
        tp->PublishData(TEL33009_MAP_STRIPE_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
        POS_TRACE_ERROR(EID(STRIPEMAP_STORE_FAILURE), "[Mapper StripeMap FlushDirtyPagesGiven] failed to FlushMap for stripeMap Failed");
    }
    return ret;
}

int
StripeMapManager::FlushTouchedPages(EventSmartPtr cb)
{
    if (numWriteIssuedCount != 0)
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_COMPLETED), "[MAPPER StripeMap FlushTouchedPages] Failed to Issue Flush, Another Flush is still progressing, issuedCount:{}", numWriteIssuedCount);
        return ERRID(MAP_FLUSH_IN_PROGRESS);
    }

    assert(callback == nullptr);
    callback = cb;
    EventSmartPtr eventStripeMap = std::make_shared<MapFlushedEvent>(STRIPE_MAP_ID, this);
    numWriteIssuedCount++;
    POSMetricValue v;
    v.gauge = numWriteIssuedCount;
    tp->PublishData(TEL33009_MAP_STRIPE_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper StripeMap FlushTouchedPages] Issue Flush StripeMap, array:{}, arrayId:{}",
        addrInfo->GetArrayName(), addrInfo->GetArrayId());
    int ret = stripeMap->FlushTouchedPages(eventStripeMap);
    if (ret < 0)
    {
        numWriteIssuedCount--;
        v.gauge = numWriteIssuedCount;
        tp->PublishData(TEL33009_MAP_STRIPE_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
        POS_TRACE_ERROR(EID(STRIPEMAP_STORE_FAILURE), "[Mapper StripeMap FlushTouchedPages] failed to FlushMap for stripeMap Failed");
    }
    return ret;
}

void
StripeMapManager::MapFlushDone(int mapId)
{
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "[Mapper StripeMap] StripeMap Flush Done, WritePendingCnt:{} @MapAsyncFlushDone, array:{}, arrayId:{}",
        numWriteIssuedCount, addrInfo->GetArrayName(), addrInfo->GetArrayId());
    if (callback != nullptr)
    {
        eventScheduler->EnqueueEvent(callback);
        callback = nullptr;
    }
    assert(numWriteIssuedCount > 0);
    numWriteIssuedCount--;
    POSMetricValue v;
    v.gauge = numWriteIssuedCount;
    tp->PublishData(TEL33009_MAP_STRIPE_FLUSH_PENDINGIO_CNT, v, MT_GAUGE);
}

void
StripeMapManager::WaitAllPendingIoDone(void)
{
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "[Mapper StripeMap] WaitAllPendingIoDone PendingWriteCnt:{}, PendingReadCnt:{}, array:{}, arrayId:{}",
        numWriteIssuedCount, numLoadIssuedCount, addrInfo->GetArrayName(), addrInfo->GetArrayId());
    while ((numWriteIssuedCount + numLoadIssuedCount) != 0)
        ;
}

void
StripeMapManager::WaitWritePendingIoDone(void)
{
    while ((addrInfo->IsUT() == false) && (numWriteIssuedCount != 0))
        ;
}

StripeMapContent*
StripeMapManager::GetStripeMapContent(void)
{
    return stripeMap;
}

void
StripeMapManager::SetStripeMapContent(StripeMapContent* content)
{
    // only for UT
    if (stripeMap != nullptr)
    {
        delete stripeMap;
    }
    stripeMap = content;
}

StripeAddr
StripeMapManager::GetLSA(StripeId vsid)
{
    pthread_rwlock_rdlock(&stripeMapLock);
    StripeAddr stripeAddr = stripeMap->GetEntry(vsid);
    pthread_rwlock_unlock(&stripeMapLock);
    return stripeAddr;
}

std::tuple<StripeAddr, bool>
StripeMapManager::GetLSAandReferLsid(StripeId vsid)
{
    pthread_rwlock_rdlock(&stripeMapLock);
    StripeAddr stripeAddr = stripeMap->GetEntry(vsid);
    IWBStripeAllocator* iWBStripeAllocator = AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(addrInfo->GetArrayId());
    bool referenced = iWBStripeAllocator->ReferLsidCnt(stripeAddr);
    pthread_rwlock_unlock(&stripeMapLock);

    return std::make_tuple(stripeAddr, referenced);
}

StripeId
StripeMapManager::GetRandomLsid(StripeId vsid)
{
    // To read only in user ssd area.
    return (vsid + addrInfo->GetNumWbStripes()) % addrInfo->GetMaxVSID();
}

int
StripeMapManager::SetLSA(StripeId vsid, StripeId lsid, StripeLoc loc)
{
    StripeAddr entry = {.stripeLoc = loc, .stripeId = lsid};
    pthread_rwlock_wrlock(&stripeMapLock);
    int ret = stripeMap->SetEntry(vsid, entry);
    pthread_rwlock_unlock(&stripeMapLock);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(STRIPEMAP_SET_FAILURE),
            "[Mapper StripeMap] StripeMap set failure, vsid:{}  lsid:{}  loc:{}", vsid, lsid, loc);
    }
    else
    {
        ret = 0;
    }
    return ret;
}

MpageList
StripeMapManager::GetDirtyStripeMapPages(int vsid)
{
    return stripeMap->GetDirtyPages(vsid, 1);
}

int
StripeMapManager::Dump(std::string fileName)
{
    return stripeMap->Dump(fileName);
}

int
StripeMapManager::DumpLoad(std::string fileName)
{
    return stripeMap->DumpLoad(fileName);
}

void
StripeMapManager::_MapLoadDone(int param)
{
    POS_TRACE_INFO(EID(MAP_LOAD_COMPLETED), "[Mapper StripeMap] arrayName:{} load done, so wake up! @_MapLoadDone, arrayId:{}",
        addrInfo->GetArrayName(), addrInfo->GetArrayId());
    assert(numLoadIssuedCount > 0);
    numLoadIssuedCount--;
}

void
StripeMapManager::_WaitLoadIoDone()
{
    while ((addrInfo->IsUT() == false) && (numLoadIssuedCount != 0))
        ;
}

} // namespace pos
