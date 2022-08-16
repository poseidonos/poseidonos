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

#include "src/gc/gc_map_update_completion.h"

#include <air/Air.h>

#include <list>
#include <string>

#include "src/allocator/allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_mgmt/array_manager.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/gc/copier_meta.h"
#include "src/gc/gc_stripe_manager.h"
#include "src/include/backend_event.h"
#include "src/include/branch_prediction.h"
#include "src/io/backend_io/flush_completion.h"
#include "src/io/backend_io/stripe_map_update_request.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/io/general_io/rba_state_service.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/mapper_service/mapper_service.h"
#include "src/volume/volume_service.h"

namespace pos
{
GcMapUpdateCompletion::GcMapUpdateCompletion(StripeSmartPtr stripe, std::string arrayName, IStripeMap* iStripeMap,
    EventScheduler* eventScheduler, GcStripeManager* gcStripeManager)
: GcMapUpdateCompletion(stripe, arrayName, iStripeMap, eventScheduler, gcStripeManager,
      ArrayMgr()->GetInfo(arrayName)->arrayInfo,
      RBAStateServiceSingleton::Instance()->GetRBAStateManager(ArrayMgr()->GetInfo(arrayName)->arrayInfo->GetIndex()),
      VolumeServiceSingleton::Instance()->GetVolumeManager(ArrayMgr()->GetInfo(arrayName)->arrayInfo->GetIndex()))
{
}

GcMapUpdateCompletion::GcMapUpdateCompletion(StripeSmartPtr stripe, std::string arrayName, IStripeMap* iStripeMap,
    EventScheduler* eventScheduler, GcStripeManager* gcStripeManager,
    IArrayInfo* inputIArrayInfo,
    RBAStateManager* inputRbaStateManager,
    IVolumeIoManager* inputVolumeManager)
: Callback(false),
  stripe(stripe),
  arrayName(arrayName),
  iStripeMap(iStripeMap),
  eventScheduler(eventScheduler),
  gcStripeManager(gcStripeManager),
  iArrayInfo(inputIArrayInfo),
  rbaStateManager(inputRbaStateManager),
  volumeManager(inputVolumeManager)
{
    const PartitionLogicalSize* udSize =
        iArrayInfo->GetSizeInfo(PartitionType::USER_DATA);
    totalBlksPerUserStripe = udSize->blksPerStripe;
}

GcMapUpdateCompletion::~GcMapUpdateCompletion(void)
{
}

bool
GcMapUpdateCompletion::_DoSpecificJob(void)
{
    BlkAddr rba;
    uint32_t volId;

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

    volumeManager->DecreasePendingIOCount(volId, VolumeIoType::InternalIo);
    airlog("InternalIoPendingCnt", "user", volId, -1);

    gcStripeManager->SetFinished();

    POS_TRACE_DEBUG(EID(GC_MAP_UPDATE_COMPLETION),
        "gc map update completion, arrayName:{}, stripeUserLsid:{}",
        arrayName, stripe->GetUserLsid());

    return true;
}

} // namespace pos
