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

#include "src/gc/gc_map_update.h"

#include "src/logger/logger.h"
#include "src/spdk_wrapper/free_buffer_pool.h"
#if defined QOS_ENABLED_BE
#include "src/include/backend_event.h"
#endif
#include "Air.h"
#include "src/allocator/allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_mgmt/array_manager.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/gc/copier_meta.h"
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
GcMapUpdate::GcMapUpdate(Stripe* stripe, std::string& arrayName)
: GcMapUpdate(stripe, MapperServiceSingleton::Instance()->GetIStripeMap(arrayName),
      EventSchedulerSingleton::Instance(), arrayName)
{
}

GcMapUpdate::GcMapUpdate(Stripe* stripe, IStripeMap* stripeMap, EventScheduler* eventScheduler, std::string& arrayName)
: stripe(stripe),
  iStripeMap(stripeMap),
  eventScheduler(eventScheduler),
  arrayName(arrayName)
{
#if defined QOS_ENABLED_BE
    SetFrontEnd(false);
    SetEventType(BackendEvent_GC);
#endif
    AIRLOG(LAT_BDEV_READ, 0, 0, stripe->GetVsid());

    IArrayInfo* info = ArrayMgr::Instance()->GetArrayInfo(arrayName);
    const PartitionLogicalSize* udSize =
        info->GetSizeInfo(PartitionType::USER_DATA);
    totalBlksPerUserStripe = udSize->blksPerStripe;
    stripesPerSegment = udSize->stripesPerSegment;
}

GcMapUpdate::~GcMapUpdate(void)
{
}

bool
GcMapUpdate::Execute(void)
{
    IVSAMap* iVSAMap = MapperServiceSingleton::Instance()->GetIVSAMap(arrayName);
    StripeId stripeId = stripe->GetVsid();
    BlkAddr rba;
    uint32_t volId;
    VirtualBlkAddr currentVsa;
    bool isValidData = false;
    uint32_t numValidate = 0;

    if (stripeOffset != totalBlksPerUserStripe)
    {
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

                uint32_t oneBlockCount = 1;
                VirtualBlkAddr writeVsa = {stripeId, stripeOffset};
                VirtualBlks writeVsaRange = {writeVsa, oneBlockCount};

                                if (isValidData == true)
                {
                    iVSAMap->SetVSAsInternal(volId, rba, writeVsaRange);
                    _RegisterInvalidateSegments(currentVsa);
                    numValidate++;
                }
            }
        }
        _InvalidateBlock();
        _ValidateBlock(stripeId, numValidate);

        RBAStateManager* rbaStateManager =
            RBAStateServiceSingleton::Instance()->GetRBAStateManager(arrayName);
        std::list<RbaAndSize> rbaList;

        for (uint32_t i = 0; i < totalBlksPerUserStripe; i++)
        {
            std::tie(rba, volId) = stripe->GetReverseMapEntry(i);
            if (likely(rba != INVALID_RBA))
            {
                RbaAndSize rbaAndSize = {rba * VolumeIo::UNITS_PER_BLOCK,
                    BLOCK_SIZE};
                rbaList.push_back(rbaAndSize);
            }
        }
        std::tie(rba, volId) = stripe->GetReverseMapEntry(0);
        rbaStateManager->ReleaseOwnershipRbaList(volId, rbaList);
    }

    FlushCompletion event(stripe, iStripeMap, eventScheduler, arrayName);
    bool done = event.Execute();
    bool wrapupSuccessful = true;

    if (unlikely(false == done))
    {
        EventSmartPtr event(new FlushCompletion(stripe, arrayName));
        if (likely(event != nullptr))
        {
            eventScheduler->EnqueueEvent(event);
        }
        else
        {
            POS_TRACE_ERROR(static_cast<int>(POS_EVENT_ID::MAP_UPDATE_HANDLER_EVENT_ALLOCATE_FAIL),
                "Failed to allocate flush wrapup event");
            wrapupSuccessful = false;
        }
    }

    return wrapupSuccessful;
}

void
GcMapUpdate::_RegisterInvalidateSegments(VirtualBlkAddr vsa)
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

void
GcMapUpdate::_InvalidateBlock(void)
{
    IBlockAllocator* iBlockAllocator = AllocatorServiceSingleton::Instance()->GetIBlockAllocator(arrayName);
    for (auto it : invalidSegCnt)
    {
        SegmentId segId = it.first;
        uint32_t invalidCnt = it.second;

        VirtualBlkAddr invalidVsa;
        invalidVsa.stripeId = segId * stripesPerSegment;
        invalidVsa.offset = 0;
        VirtualBlks invalidVsaRange = {invalidVsa, invalidCnt};

        iBlockAllocator->InvalidateBlks(invalidVsaRange);
    }
}

void
GcMapUpdate::_ValidateBlock(StripeId stripeId, uint32_t cnt)
{
    IBlockAllocator* iBlockAllocator = AllocatorServiceSingleton::Instance()->GetIBlockAllocator(arrayName);
    VirtualBlkAddr writeVsa = {stripeId, 0};
    VirtualBlks writeVsaRange = {writeVsa, cnt};

    iBlockAllocator->ValidateBlks(writeVsaRange);
}

} // namespace pos
