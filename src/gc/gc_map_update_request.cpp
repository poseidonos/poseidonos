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

#include "src/gc/gc_map_update_request.h"

#include <list>
#include <memory>
#include <string>

#include "src/allocator/allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_mgmt/array_manager.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/gc/copier_meta.h"
#include "src/gc/gc_map_update_completion.h"
#include "src/gc/gc_stripe_manager.h"
#include "src/include/backend_event.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/backend_io/flush_completion.h"
#include "src/io/backend_io/stripe_map_update_request.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/io/general_io/rba_state_service.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/mapper_service/mapper_service.h"
#include "src/meta_service/meta_service.h"

namespace pos
{
GcMapUpdateRequest::GcMapUpdateRequest(StripeSmartPtr stripe, std::string arrayName, GcStripeManager* gcStripeManager)
: GcMapUpdateRequest(stripe,
      std::make_shared<GcMapUpdateCompletion>(stripe, arrayName, MapperServiceSingleton::Instance()->GetIStripeMap(arrayName), EventSchedulerSingleton::Instance(), gcStripeManager),
      MapperServiceSingleton::Instance()->GetIVSAMap(arrayName),
      ArrayMgr()->GetInfo(arrayName)->arrayInfo,
      MetaServiceSingleton::Instance()->GetMetaUpdater(arrayName))
{
}

GcMapUpdateRequest::GcMapUpdateRequest(StripeSmartPtr stripe,
    CallbackSmartPtr completionEvent,
    IVSAMap* inputIVSAMap,
    IArrayInfo* inputIArrayInfo,
    IMetaUpdater* inputMetaUpdater)
: Event(false),
  stripe(stripe),
  completionEvent(completionEvent),
  iVSAMap(inputIVSAMap),
  iArrayInfo(inputIArrayInfo),
  metaUpdater(inputMetaUpdater)
{
    const PartitionLogicalSize* udSize =
        iArrayInfo->GetSizeInfo(PartitionType::USER_DATA);
    totalBlksPerUserStripe = udSize->blksPerStripe;
    stripesPerSegment = udSize->stripesPerSegment;

    currentStripeOffset = 0;
}

bool
GcMapUpdateRequest::Execute(void)
{
    bool result = true;
    result = _BuildMeta();
    if (unlikely(result != true))
    {
        return result;
    }

    result = _UpdateMeta();

    return result;
}

bool
GcMapUpdateRequest::_BuildMeta(void)
{
    StripeId stripeId = stripe->GetVsid();
    BlkAddr rba;
    uint32_t volId;
    VirtualBlkAddr currentVsa;
    bool isValidData = false;

    for (; currentStripeOffset < totalBlksPerUserStripe; currentStripeOffset++)
    {
        std::tie(rba, volId) = stripe->GetReverseMapEntry(currentStripeOffset);
        if (likely(INVALID_RBA != rba))
        {
            int shouldRetry = OK_READY;
            currentVsa = iVSAMap->GetVSAInternal(volId, rba, shouldRetry);

            if (NEED_RETRY == shouldRetry)
            {
                return false;
            }
            VirtualBlkAddr oldVsa = stripe->GetVictimVsa(currentStripeOffset);
            isValidData = (currentVsa == oldVsa);

            VirtualBlkAddr writeVsa = {stripeId, currentStripeOffset};

            if (isValidData == true)
            {
                _AddBlockMapUpdateLog(rba, writeVsa);
                _RegisterInvalidateSegments(currentVsa);
            }
        }
    }

    std::tie(rba, volId) = stripe->GetReverseMapEntry(0);
    mapUpdates.volumeId = volId;
    mapUpdates.vsid = stripeId;
    mapUpdates.wbLsid = stripe->GetWbLsid();
    mapUpdates.userLsid = stripe->GetUserLsid();

    return true;
}

bool
GcMapUpdateRequest::_UpdateMeta(void)
{
    int result = metaUpdater->UpdateGcMap(stripe, mapUpdates, invalidSegCnt, completionEvent);
    if (unlikely(0 != result))
    {
        POS_EVENT_ID eventId = EID(GC_MAP_UPDATE_FAILED);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "gc map update failed, arrayName:{}, stripeUserLsid:{}",
            iArrayInfo->GetName(), mapUpdates.userLsid);
        return false;
    }

    POS_TRACE_DEBUG(EID(GC_MAP_UPDATE_REQUEST),
        "gc map update request, arrayName:{}, stripeUserLsid:{}",
        iArrayInfo->GetName(), mapUpdates.userLsid);
    return true;
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
