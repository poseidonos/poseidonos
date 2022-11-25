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

#include "src/gc/gc_flush_completion.h"

#include <air/Air.h>

#include <memory>

#include "src/allocator/allocator.h"
#include "src/allocator/stripe/stripe.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_mgmt/array_manager.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/gc/copier_meta.h"
#include "src/gc/gc_map_update_request.h"
#include "src/gc/gc_stripe_manager.h"
#include "src/include/backend_event.h"
#include "src/include/branch_prediction.h"
#include "src/io/backend_io/flush_completion.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/io/general_io/rba_state_service.h"
#include "src/logger/logger.h"

namespace pos
{
GcFlushCompletion::GcFlushCompletion(StripeSmartPtr stripe, string arrayName, GcStripeManager* gcStripeManager, GcWriteBuffer* dataBuffer)
: GcFlushCompletion(stripe, arrayName, gcStripeManager, dataBuffer,
      nullptr,
      RBAStateServiceSingleton::Instance()->GetRBAStateManager(ArrayMgr()->GetInfo(arrayName)->arrayInfo->GetIndex()),
      ArrayMgr()->GetInfo(arrayName)->arrayInfo,
      MapperServiceSingleton::Instance()->GetIVSAMap(arrayName))
{
}

GcFlushCompletion::GcFlushCompletion(StripeSmartPtr stripe, string arrayName, GcStripeManager* gcStripeManager, GcWriteBuffer* dataBuffer,
    EventSmartPtr inputEvent,
    RBAStateManager* inputRbaStateManager,
    IArrayInfo* inputIArrayInfo,
    IVSAMap* iVSAMap)
: Callback(false, CallbackType_GcFlushCompletion),
  stripe(stripe),
  arrayName(arrayName),
  gcStripeManager(gcStripeManager),
  dataBuffer(dataBuffer),
  inputEvent(inputEvent),
  rbaStateManager(inputRbaStateManager),
  iArrayInfo(inputIArrayInfo),
  iVSAMap(iVSAMap)
{
}

GcFlushCompletion::~GcFlushCompletion(void)
{
}

bool
GcFlushCompletion::_DoSpecificJob(void)
{
    if (nullptr != dataBuffer)
    {
        gcStripeManager->ReturnBuffer(dataBuffer);
        dataBuffer = nullptr;
        POS_TRACE_DEBUG(EID(GC_STRIPE_FLUSH_COMPLETION),
            "array_name:{}, stripe_id:{}",
            arrayName, stripe->GetUserLsid());
    }

    if (isInit == false)
    {
        _Init();
    }

    if (false == _AcquireOwnership())
    {
        return false;
    }

    POS_TRACE_DEBUG(EID(GC_RBA_OWNERSHIP_ACQUIRED), "array_name:{}, stripe_id:{}, retried:{}",
        arrayName, lsid, retryCnt);
    retryCnt = 0;

    airlog("PERF_GcFlush", "write", 0, totalBlksPerUserStripe * BLOCK_SIZE);
    EventSmartPtr event;
    if (nullptr == inputEvent)
    {
        event = make_shared<GcMapUpdateRequest>(stripe, arrayName, gcStripeManager, sectorRbaList);
    }
    else
    {
        event = inputEvent;
    }
    stripe->Flush(event);
    return true;
}

void
GcFlushCompletion::_Init(void)
{
    totalBlksPerUserStripe = iArrayInfo->GetSizeInfo(PartitionType::USER_DATA)->blksPerStripe;
    for (uint32_t i = 0; i < totalBlksPerUserStripe; i++)
    {
        BlkAddr rba;
        tie(rba, volId) = stripe->GetReverseMapEntry(i);
        if (likely(rba != INVALID_RBA))
        {
            if (_IsValidRba(rba, i) == true)
            {
                RbaAndSize rbaAndSize = {rba * VolumeIo::UNITS_PER_BLOCK, BLOCK_SIZE};
                auto it = sectorRbaList.insert(sectorRbaList.end(), rbaAndSize);
                VictimRba victimRba = {rba, i, it};
                victimBlockList.push_back(victimRba);
            }
        }
    }
    lsid = stripe->GetUserLsid();
    isInit = true;
}

bool
GcFlushCompletion::_IsValidRba(BlkAddr rba, uint32_t offset)
{
    int shouldRetry = OK_READY;
    VirtualBlkAddr currentVsa = iVSAMap->GetVSAInternal(volId, rba, shouldRetry);
    if (NEED_RETRY != shouldRetry)
    {
        VirtualBlkAddr oldVsa = stripe->GetVictimVsa(offset);
        bool isValid = currentVsa == oldVsa;
        return isValid;
    }
    return true;
}

void
GcFlushCompletion::_RemoveInvalidRba(void)
{
    uint32_t removedCnt = 0;
    for (auto i = victimBlockList.begin(); i != victimBlockList.end(); )
    {
        BlkAddr rba = i->rba;
        uint32_t offset = i->offset;
        if (_IsValidRba(rba, offset) == false)
        {
            sectorRbaList.erase(i->iter);
            i = victimBlockList.erase(i);
            removedCnt++;
        }
        else
        {
            i++;
        }
    }
    if (removedCnt > 0)
    {
        POS_TRACE_DEBUG(EID(GC_INVALID_RBA_REMOVED),
            "array_name:{}, stripe_id:{}, removed:{}, remaining_rba_count:{}",
            arrayName, lsid, removedCnt, victimBlockList.size());
    }
}

bool
GcFlushCompletion::_AcquireOwnership(void)
{
    bool ownershipAcquired = rbaStateManager->AcquireOwnershipRbaList(volId, sectorRbaList);
    if (false == ownershipAcquired)
    {
        retryCnt++;
        if (retryCnt % 1000 == 0)
        {
            POS_TRACE_DEBUG(EID(GC_RBA_OWNERSHIP_ACQUISITION_FAILED),
                "array_name:{}, stripe_id:{}, retried:{}, rba_count:{}",
                arrayName, lsid, retryCnt, victimBlockList.size());
            _RemoveInvalidRba();
        }
        return false;
    }
    return true;
}

} // namespace pos
