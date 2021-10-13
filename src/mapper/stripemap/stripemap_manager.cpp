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

#include "src/allocator_service/allocator_service.h"
#include "src/allocator/stripe/stripe.h"
#include "src/mapper/map_flushed_event.h"
#include "src/mapper/address/mapper_address_info.h"
#include "src/mapper/stripemap/stripemap_manager.h"
#include "src/allocator/i_wbstripe_allocator.h"

#include <string>
#include <tuple>

namespace pos
{
StripeMapManager::StripeMapManager(MapperAddressInfo* info, std::string arrayName, int arrayId)
: stripeMap(nullptr),
  addrInfo(info),
  numLoadIssuedCount(0),
  numWriteIssuedCount(0),
  arrayName(arrayName),
  arrayId(arrayId)
{
    pthread_rwlock_init(&stripeMapLock, nullptr);
}

StripeMapManager::~StripeMapManager(void)
{
    Dispose();
}

int
StripeMapManager::Init(void)
{
    numWriteIssuedCount = 0;
    numLoadIssuedCount = 0;
    stripeMap = new StripeMapContent(STRIPE_MAP_ID, arrayId);
    stripeMap->InMemoryInit(addrInfo->maxVsid, addrInfo->GetMpageSize());
    int ret = stripeMap->OpenMapFile();
    if (ret == EID(NEED_TO_INITIAL_STORE))
    {
        EventSmartPtr eventStripeMap = std::make_shared<MapFlushedEvent>(STRIPE_MAP_ID, this);
        numWriteIssuedCount++;
        POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper StripeMap] Issue Flush Header, array:{}", addrInfo->GetArrayName());
        ret = stripeMap->FlushHeader(eventStripeMap);
        if (ret < 0)
        {
            numWriteIssuedCount--;
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper StripeMap] Failed to Initial Store StripeMap File, array:{}", addrInfo->GetArrayName());
        }
        else
        {
            WaitAllPendingIoDone();
        }
    }
    else if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MAPPER_FAILED), "[Mapper StripeMap] Failed to create StripeMap File, array:{}", addrInfo->GetArrayName());
    }
    else
    {
        ret = LoadStripeMapFile();
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MAPPER_START), "[Mapper StripeMap] Failed to load StripeMap File");
        }
    }
    return ret;
}

void
StripeMapManager::Dispose(void)
{
    WaitAllPendingIoDone();
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
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper StripeMap] Issue Load StripeMap, array:{}", addrInfo->GetArrayName());
    int ret = stripeMap->Load(cb);
    if (ret == -EID(MAP_LOAD_COMPLETED))
    {
        numLoadIssuedCount--;
        POS_TRACE_ERROR(EID(MAPPER_START), "[Mapper StripeMap] failed To Load StripeMap");
        return ret;
    }

    _WaitLoadIoDone();
    POS_TRACE_INFO(EID(MAPPER_START), "[Mapper StripeMap] StripeMap Loaded, arrayName:{}", addrInfo->GetArrayName());
    return ret;
}

int
StripeMapManager::FlushMap(void)
{
    EventSmartPtr eventStripeMap = std::make_shared<MapFlushedEvent>(STRIPE_MAP_ID, this);
    numWriteIssuedCount++;
    POS_TRACE_INFO(EID(MAPPER_FAILED), "[Mapper StripeMap] Issue Flush StripeMap, array:{}", addrInfo->GetArrayName());
    int ret = stripeMap->FlushTouchedPages(eventStripeMap);
    if (ret < 0)
    {
        numWriteIssuedCount--;
        POS_TRACE_ERROR(EID(STRIPEMAP_STORE_FAILURE), "[Mapper StripeMap] failed to FlushMap for stripeMap Failed");
    }
    return ret;
}

void
StripeMapManager::MapFlushDone(int mapId)
{
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "[Mapper StripeMap] StripeMap Flush Done, WritePendingCnt:{} @MapAsyncFlushDone", numWriteIssuedCount);
    assert(numWriteIssuedCount > 0);
    numWriteIssuedCount--;
}

void
StripeMapManager::WaitAllPendingIoDone(void)
{
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "[Mapper StripeMap] PendingWriteCnt:{}, PendingReadCnt:{}", numWriteIssuedCount, numLoadIssuedCount);
    while ((numWriteIssuedCount + numLoadIssuedCount) != 0);
}

void
StripeMapManager::WaitWritePendingIoDone(void)
{
    while (numWriteIssuedCount != 0);
}

StripeMapContent*
StripeMapManager::GetStripeMapContent(void)
{
    return stripeMap;
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
    IWBStripeAllocator* iWBStripeAllocator = AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(arrayId);
    bool referenced = iWBStripeAllocator->ReferLsidCnt(stripeAddr);
    pthread_rwlock_unlock(&stripeMapLock);

    return std::make_tuple(stripeAddr, referenced);
}

StripeId
StripeMapManager::GetRandomLsid(StripeId vsid)
{
    return vsid + addrInfo->numWbStripes;
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
        POS_TRACE_ERROR((int)POS_EVENT_ID::STRIPEMAP_SET_FAILURE,
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

void
StripeMapManager::_MapLoadDone(int param)
{
    POS_TRACE_INFO(EID(MAP_LOAD_COMPLETED), "[Mapper StripeMap] volId:{} arrayName:{} load done, so wake up! @_MapLoadDone", addrInfo->GetArrayName());
    assert(numLoadIssuedCount > 0);
    numLoadIssuedCount--;
}

void
StripeMapManager::_WaitLoadIoDone()
{
    while (numLoadIssuedCount != 0);
}

} // namespace pos
