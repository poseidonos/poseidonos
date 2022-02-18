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

#include "src/io/frontend_io/write_completion.h"

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_mgmt/array_manager.h"
#include "src/bio/volume_io.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/backend_io/stripe_map_update_request.h"
#include "src/io/backend_io/flush_submission.h"
#include "src/io/general_io/rba_state_service.h"
#include "src/io/frontend_io/write_for_parity.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/allocator/event/stripe_put_event.h"
#include "src/io/backend_io/flush_completion.h"

namespace pos
{
WriteCompletion::WriteCompletion(VolumeIoSmartPtr input)
: WriteCompletion(input,
      AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(input.get()->GetArrayId()),
      EventFrameworkApiSingleton::Instance()->IsReactorNow(),
      EventSchedulerSingleton::Instance())
{
}

WriteCompletion::WriteCompletion(VolumeIoSmartPtr input,
    IWBStripeAllocator* iWBStripeAllocator, bool isReactorNow,
    EventScheduler* inputEventScheduler)
: Callback(isReactorNow, CallbackType_WriteCompletion),
  volumeIo(input),
  iWBStripeAllocator(iWBStripeAllocator),
  eventScheduler(inputEventScheduler)
{
    arrayInfo = ArrayMgr()->GetInfo(volumeIo->GetArrayId())->arrayInfo;
}

WriteCompletion::~WriteCompletion()
{
}

bool
WriteCompletion::_DoSpecificJob()
{
    bool executionSuccessful = false;

    uint32_t volumeId = volumeIo->GetVolumeId();
    BlkAddr startRba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);

    RBAStateManager& rbaStateManager =
        *RBAStateServiceSingleton::Instance()->GetRBAStateManager(volumeIo->GetArrayId());
    rbaStateManager.BulkReleaseOwnership(volumeId, startRba, blockCount);

    Stripe* stripeToFlush = nullptr;
    executionSuccessful = _UpdateStripe(stripeToFlush);
    if (unlikely(nullptr != stripeToFlush))
    {
        executionSuccessful = _RequestFlush(stripeToFlush);
    }

    if (unlikely(false == executionSuccessful))
    {
        // We inform the error to the Callee of this callback,
        // and do not retry this callback.
        InformError(IOErrorType::GENERIC_ERROR);
        executionSuccessful = true;
    }

    volumeIo = nullptr;

    return executionSuccessful;
}

bool
WriteCompletion::_UpdateStripe(Stripe*& stripeToFlush)
{
    bool stripeUpdateSuccessful = true;
    StripeAddr lsidEntry = volumeIo->GetLsidEntry();
    Stripe* stripe = iWBStripeAllocator->GetStripe(lsidEntry.stripeId);
    if (likely(nullptr != stripe))
    {
        uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
        uint32_t remainingBlksAfterDecrease =
            stripe->DecreseBlksRemaining(blockCount);
        if (0 == remainingBlksAfterDecrease)
        {
            stripeToFlush = stripe;
        }
    }
    else
    {
        VirtualBlkAddr startVsa = volumeIo->GetVsa();
        StripeId vsid = startVsa.stripeId;
        POS_EVENT_ID eventId = POS_EVENT_ID::WRWRAPUP_STRIPE_NOT_FOUND;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Stripe #{} not found at WriteWrapup state", vsid);

        stripeUpdateSuccessful = false;
    }

    return stripeUpdateSuccessful;
}

bool
WriteCompletion::_RequestFlush(Stripe* stripe)
{
    bool requestFlushSuccessful = true; 
    EventSmartPtr event(new FlushSubmission(stripe, volumeIo->GetArrayId(),
        arrayInfo->IsWriteThroughEnabled()));

    if (unlikely(stripe->Flush(event) < 0))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::WRWRAPUP_EVENT_ALLOC_FAILED;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Flush Event allocation failed at WriteWrapup state");

        requestFlushSuccessful = false;
    }

    return requestFlushSuccessful;
}

} // namespace pos
