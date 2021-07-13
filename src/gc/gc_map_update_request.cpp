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

#include "src/gc/gc_map_update_request.h"
#include "src/gc/gc_map_update.h"

#include "src/logger/logger.h"
#include "src/spdk_wrapper/free_buffer_pool.h"
#include "src/include/backend_event.h"
#include "Air.h"
#include "src/allocator/allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_mgmt/array_manager.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/gc/copier_meta.h"
#include "src/gc/gc_stripe_manager.h"
#include "src/io/backend_io/flush_completion.h"
#include "src/io/backend_io/stripe_map_update_request.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/io/general_io/rba_state_service.h"
#include "src/mapper/mapper.h"
#include "src/mapper_service/mapper_service.h"

#include <list>
#include <string>

namespace pos
{
GcMapUpdateRequest::GcMapUpdateRequest(Stripe* stripe, std::string arrayName, GcStripeManager* gcStripeManager, IStripeMap* iStripeMap,
                IVSAMap *iVSAMap, JournalService *journalService, EventScheduler *eventScheduler)
: Event(false),
  stripe(stripe),
  arrayName(arrayName),
  gcStripeManager(gcStripeManager),
  iStripeMap(iStripeMap),
  iVSAMap(iVSAMap),
  journalService(journalService),
  eventScheduler(eventScheduler)
  
{
    IArrayInfo* info = ArrayMgr::Instance()->GetArrayInfo(arrayName);
    const PartitionLogicalSize* udSize =
        info->GetSizeInfo(PartitionType::USER_DATA);
    totalBlksPerUserStripe = udSize->blksPerStripe;
    stripesPerSegment = udSize->stripesPerSegment;;

    stripeOffset = 0;
}


GcMapUpdateRequest::GcMapUpdateRequest(Stripe* stripe, std::string arrayName, GcStripeManager* gcStripeManager)
: GcMapUpdateRequest(
    stripe,
    arrayName,
    gcStripeManager,
    MapperServiceSingleton::Instance()->GetIStripeMap(arrayName),
    MapperServiceSingleton::Instance()->GetIVSAMap(arrayName),
    JournalServiceSingleton::Instance(),
    EventSchedulerSingleton::Instance())
{
}

bool
GcMapUpdateRequest::Execute(void)
{
    StripeId stripeId = stripe->GetVsid();
    BlkAddr rba;
    uint32_t volId;
    VirtualBlkAddr currentVsa;
    bool isValidData = false;
    bool executionSuccessful = false;

    for (; stripeOffset < totalBlksPerUserStripe; stripeOffset++)
    {
        std::tie(rba, volId) = stripe->GetReverseMapEntry(stripeOffset);
        if (likely(INVALID_RBA != rba))
        {
            int shouldRetry = CALLER_EVENT;
            currentVsa = iVSAMap->GetVSAInternal(volId, rba, shouldRetry);

            if (NEED_RETRY == shouldRetry)
            {
                return false;
            }
            VirtualBlkAddr oldVsa = stripe->GetVictimVsa(stripeOffset);
            isValidData = (currentVsa == oldVsa);

            VirtualBlkAddr writeVsa = {stripeId, stripeOffset};

            if (isValidData == true)
            {
                _AddBlockMapUpdateLog(rba, writeVsa);
                _GetDirtyPages(volId, rba);
                _RegisterInvalidateSegments(currentVsa);
            }
        }
    }

    std::tie(rba, volId) = stripe->GetReverseMapEntry(0);

    mapUpdates.volumeId = volId;
    mapUpdates.vsid = stripeId;
    mapUpdates.wbLsid = stripe->GetWbLsid();
    mapUpdates.userLsid = stripe->GetUserLsid();

    EventSmartPtr event(new GcMapUpdate(stripe, arrayName, mapUpdates, invalidSegCnt, iStripeMap, gcStripeManager));
    if (journalService->IsEnabled(arrayName))
    {
        MapPageList dirtyMap;
        dirtyMap[volId] = volumeDirtyList;
        dirtyMap[STRIPE_MAP_ID] = iStripeMap->GetDirtyStripeMapPages(stripeId);

        IJournalWriter* journal = journalService->GetWriter(arrayName);
        journal->AddGcStripeFlushedLog(mapUpdates, dirtyMap, event);

        executionSuccessful = true;
    }
    else
    {
        eventScheduler->EnqueueEvent(event);
        executionSuccessful = true;
    }

    POS_TRACE_DEBUG((int)POS_EVENT_ID::GC_MAP_UPDATE_REQUEST,
        "gc map update request, arrayName:{}, stripeUserLsid:{}, result:{}",
        arrayName, stripe->GetUserLsid(), executionSuccessful);

    return executionSuccessful;
}

void
GcMapUpdateRequest::_AddBlockMapUpdateLog(BlkAddr rba, VirtualBlkAddr writeVsa)
{
    GcBlockMapUpdate mapUpdate;
    mapUpdate.rba = rba;
    mapUpdate.vsa = writeVsa;
    mapUpdates.blockMapUpdateList.push_back(mapUpdate);
}

void
GcMapUpdateRequest::_GetDirtyPages(uint32_t volId, BlkAddr rba)
{
    uint32_t blockCount = 1;
    auto dirty = iVSAMap->GetDirtyVsaMapPages(volId, rba, blockCount);
    volumeDirtyList.insert(dirty.begin(), dirty.end());
}

void
GcMapUpdateRequest::_RegisterInvalidateSegments(VirtualBlkAddr vsa)
{
    SegmentId segId = vsa.stripeId / stripesPerSegment;
    if (invalidSegCnt.find(segId) == invalidSegCnt.end())
    {
        uint32_t oneBlockCount = 1;
        invalidSegCnt.emplace(segId, oneBlockCount);
    }
    else
    {
        invalidSegCnt[segId]++;
    }
}

} // namespace pos
