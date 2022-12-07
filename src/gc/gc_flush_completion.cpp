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
#include <unordered_set>

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
        gcStripeManager->FlushCompleted();
        Init();
    }

    if (false == AcquireOwnership())
    {
        return false;
    }

    if (tryCnt >= GC_RBA_TRYLOCK_RETRY_THRESHOLD)
    {
        POS_TRACE_TRACE(EID(GC_RBA_OWNERSHIP_ACQUIRED), "array_name:{}, stripe_id:{}, tried_total:{}",
            arrayName, lsid, tryCnt);
    }
    else
    {
        POS_TRACE_DEBUG(EID(GC_RBA_OWNERSHIP_ACQUIRED), "array_name:{}, stripe_id:{}, tried_total:{}",
            arrayName, lsid, tryCnt);
    }

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
    gcStripeManager->UpdateMapRequested();
    stripe->Flush(event);
    return true;
}

void
GcFlushCompletion::Init(void)
{
    totalBlksPerUserStripe = iArrayInfo->GetSizeInfo(PartitionType::USER_DATA)->blksPerStripe;
    unordered_set<BlkAddr> uniqueRba;
    uint32_t invalidRbaCnt = 0;
    for (uint32_t i = 0; i < totalBlksPerUserStripe; i++)
    {
        BlkAddr rba;
        tie(rba, volId) = stripe->GetReverseMapEntry(i);
        if (likely(rba != INVALID_RBA))
        {
            if (_IsValidRba(rba, i) == true)
            {
                auto result = uniqueRba.insert(rba);
                if (result.second == true)
                {
                    RbaAndSize rbaAndSize = {rba * VolumeIo::UNITS_PER_BLOCK, BLOCK_SIZE};
                    sectorRbaList.push_back(rbaAndSize);
                }
            }
            else
            {
                invalidRbaCnt++;
            }
        }
    }
    {
        BlkAddr rba;
        std::tie(rba, volId) = stripe->GetReverseMapEntry(0);
    }
    sectorRbaList.sort();
    currPos = sectorRbaList.begin();
    lsid = stripe->GetUserLsid();
    ownershipProgress = 0;
    isInit = true;
    POS_TRACE_DEBUG(EID(GC_MAP_UPDATE_INIT),
        "array_name:{}, stripe_id:{}, invalid_rba_count:{}, remaining_rba_count:{}",
        arrayName, lsid, invalidRbaCnt, sectorRbaList.size());
}

bool
GcFlushCompletion::_IsValidRba(BlkAddr rba, uint32_t offset)
{
    return true;
    // todo comment out due to performance issue
    // int shouldRetry = OK_READY;
    // VirtualBlkAddr currentVsa = iVSAMap->GetVSAInternal(volId, rba, shouldRetry);
    // if (NEED_RETRY != shouldRetry)
    // {
    //     VirtualBlkAddr oldVsa = stripe->GetVictimVsa(offset);
    //     bool isValid = currentVsa == oldVsa;
    //     return isValid;
    // }
    // return true;
}

bool
GcFlushCompletion::AcquireOwnership(void)
{
    tryCnt++;
    bool needLogging = tryCnt % 1000 == 0;
    currPos = rbaStateManager->AcquireOwnershipRbaList(volId, sectorRbaList, currPos, ownershipProgress);
    if (currPos != sectorRbaList.end())
    {
        if (needLogging == true)
        {
            RBAOwnerType owner = rbaStateManager->GetOwner(volId, *currPos);
            bool needWarnLogging = tryCnt % GC_RBA_TRYLOCK_RETRY_THRESHOLD == 0;
            if (needWarnLogging)
            {
                POS_TRACE_WARN(EID(GC_RBA_OWNERSHIP_ACQUISITION_FAILED),
                    "want:{}, owned:{}, array_name:{}, vol_id:{}, stripe_id:{}, tried:{}, total:{}, acquired:{}",
                    currPos->sectorRba, owner, arrayName, volId, lsid, tryCnt, sectorRbaList.size(), ownershipProgress);
            }
            else
            {
                POS_TRACE_DEBUG(EID(GC_RBA_OWNERSHIP_ACQUISITION_FAILED),
                    "want:{}, owned:{}, array_name:{}, vol_id:{}, stripe_id:{}, tried:{}, total:{}, acquired:{}",
                    currPos->sectorRba, owner, arrayName, volId, lsid, tryCnt, sectorRbaList.size(), ownershipProgress);
            }
        }
        return false;
    }
    return true;
}

} // namespace pos
