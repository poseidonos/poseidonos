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

#include "src/gc/copier_write_completion.h"
#include "src/spdk_wrapper/free_buffer_pool.h"
#include "src/logger/logger.h"
#include "src/include/backend_event.h"
#include "src/allocator/allocator.h"
#include "src/allocator/wb_stripe_manager/stripe.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_mgmt/array_manager.h"
#include "src/mapper/mapper.h"
#include "src/mapper_service/mapper_service.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/io/general_io/rba_state_service.h"
#include "src/io/backend_io/stripe_map_update_request.h"
#include "src/io/backend_io/flush_completion.h"
#include "src/gc/copier_meta.h"
#include "src/gc/gc_map_update.h"
#include "src/event_scheduler/event_scheduler.h"
#include "Air.h"

#include <list>
#include <string>

namespace pos
{
GcFlushCompletion::GcFlushCompletion(Stripe* stripe, std::string arrayName)
: GcFlushCompletion(stripe, MapperServiceSingleton::Instance()->GetIStripeMap(arrayName),
      EventSchedulerSingleton::Instance(), arrayName)
{
}

GcFlushCompletion::GcFlushCompletion(Stripe* stripe, IStripeMap* stripeMap, EventScheduler* eventScheduler, std::string arrayName)
: Event(true),
  stripe(stripe),
  iStripeMap(stripeMap),
  eventScheduler(eventScheduler),
  arrayName(arrayName)
{
    SetFrontEnd(false);
    SetEventType(BackendEvent_GC);
    AIRLOG(LAT_BDEV_READ, 0, 0, stripe->GetVsid());

    IArrayInfo* info = ArrayMgr::Instance()->GetArrayInfo(arrayName);
    const PartitionLogicalSize* udSize =
        info->GetSizeInfo(PartitionType::USER_DATA);
    totalBlksPerUserStripe = udSize->blksPerStripe;
}

GcFlushCompletion::~GcFlushCompletion(void)
{
}

bool
GcFlushCompletion::Execute(void)
{
    RBAStateManager* rbaStateManager =
        RBAStateServiceSingleton::Instance()->GetRBAStateManager(arrayName);

    BlkAddr rba;
    uint32_t volId;
    std::list<RbaAndSize> rbaList;
    bool wrapupSuccessful = true;

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
    bool ownershipAcquired = rbaStateManager->AcquireOwnershipRbaList(volId,
            rbaList);
    if (false == ownershipAcquired)
    {
        return false;
    }

    EventSmartPtr event(new GcMapUpdate(stripe, arrayName));
    if (likely(event != nullptr))
    {
        EventSchedulerSingleton::Instance()->EnqueueEvent(event);
    }
    else
    {
        POS_TRACE_ERROR(static_cast<int>(POS_EVENT_ID::MAP_UPDATE_HANDLER_EVENT_ALLOCATE_FAIL),
            "Failed to allocate flush wrapup event");
        wrapupSuccessful = false;
        rbaStateManager->ReleaseOwnershipRbaList(volId, rbaList);
    }

    return wrapupSuccessful;
}

} // namespace pos
