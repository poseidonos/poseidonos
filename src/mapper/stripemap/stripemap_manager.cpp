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

#include "src/allocator_service/allocator_service.h"
#include "src/allocator/wb_stripe_manager/stripe.h"
#include "src/mapper/map_flushed_event.h"
#include "src/mapper/address/mapper_address_info.h"
#include "src/mapper/stripemap/stripemap_manager.h"
#include "src/allocator/i_wbstripe_allocator.h"

#include <string>
#include <tuple>

namespace pos
{

StripeMapManager::StripeMapManager(MapperAddressInfo* info, std::string arrayName)
: stripeMap(nullptr),
  addrInfo(info),
  arrayName(arrayName)
{
    pthread_rwlock_init(&stripeMapLock, nullptr);
}

StripeMapManager::~StripeMapManager(void)
{
    if (stripeMap != nullptr)
    {
        delete stripeMap;
    }
}

void
StripeMapManager::Init(MapperAddressInfo& info)
{
    stripeMap = new StripeMapContent(STRIPE_MAP_ID, arrayName);
    stripeMap->Prepare(info.maxVsid);
    stripeMap->LoadSync(CREATE_FILE_IF_NOT_EXIST);
}

int
StripeMapManager::StoreMap(void)
{
    int ret = stripeMap->StoreMap();
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(STRIPEMAP_STORE_FAILURE), "StripeMap Store failed");
    }

    return ret;
}

int
StripeMapManager::FlushMap(void)
{
    EventSmartPtr callBackStripeMap = std::make_shared<MapFlushedEvent>(STRIPE_MAP_ID, this);
    mapFlushStatus[STRIPE_MAP_ID] = MapFlushState::FLUSHING;
    int ret = stripeMap->FlushTouchedPages(callBackStripeMap);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(STRIPEMAP_STORE_FAILURE), "Flush() for stripeMap Failed");
    }

     return ret;
}

void
StripeMapManager::Close(void)
{
    stripeMap->FileClose();
    delete stripeMap;
    stripeMap = nullptr;
}

StripeMapContent*
StripeMapManager::GetStripeMapContent(void)
{
    return stripeMap;
}

bool
StripeMapManager::AllMapsAsyncFlushed(void)
{
    return mapFlushStatus[STRIPE_MAP_ID] == MapFlushState::FLUSH_DONE;
}
//----------------------------------------------------------------------------//
void
StripeMapManager::MapAsyncFlushDone(int mapId)
{
    POS_TRACE_INFO(EID(MAP_FLUSH_COMPLETED), "mapId:{} Flushed @MapAsyncFlushDone", mapId);
    mapFlushStatus[STRIPE_MAP_ID] = MapFlushState::FLUSH_DONE;
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
    IWBStripeAllocator* iWBStripeAllocator = AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(arrayName);
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
            "StripeMap set failure, vsid:{}  lsid:{}  loc:{}", vsid, lsid, loc);
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

} // namespace pos
