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

#include "src/metadata/meta_updater.h"

#include "src/array_models/interface/i_array_info.h"
#include "src/bio/volume_io.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/gc/gc_stripe_manager.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/metadata/block_map_update.h"
#include "src/metadata/gc_map_update.h"
#include "src/metadata/meta_event_factory.h"
#include "src/metadata/stripe_map_update.h"

namespace pos
{
MetaUpdater::MetaUpdater(IStripeMap* stripeMap,
    IJournalManager* journal, IJournalWriter* journalWriter,
    EventScheduler* eventScheduler, MetaEventFactory* eventFactory, IArrayInfo* arrayInfo)
: stripeMap(stripeMap),
  journal(journal),
  journalWriter(journalWriter),
  eventScheduler(eventScheduler),
  metaEventFactory(eventFactory),
  arrayInfo(arrayInfo)
{
}

MetaUpdater::~MetaUpdater(void)
{
}

int
MetaUpdater::UpdateBlockMap(VolumeIoSmartPtr volumeIo, CallbackSmartPtr callback)
{
    int result = 0;

    CallbackSmartPtr blockMapUpdate =
        metaEventFactory->CreateBlockMapUpdateEvent(volumeIo);
    blockMapUpdate->SetCallee(callback);

    if (journal->IsEnabled() == true)
    {
        result = journalWriter->AddBlockMapUpdatedLog(volumeIo, blockMapUpdate);
    }
    else
    {
        // TODO (huijeong.kim) change to use ExecuteOrScheduleEvent
        bool executedSuccessfully = blockMapUpdate->Execute();
        if (unlikely(false == executedSuccessfully))
        {
            POS_EVENT_ID eventId =
                EID(WRCMP_MAP_UPDATE_FAILED);
            POS_TRACE_ERROR(static_cast<int>(eventId),
                "Write wraup failed at map update");
            eventScheduler->EnqueueEvent(blockMapUpdate);
        }
    }
    return result;
}

int
MetaUpdater::UpdateStripeMap(Stripe* stripe, CallbackSmartPtr callback)
{
    int result = 0;

    CallbackSmartPtr stripeMapUpdate =
        metaEventFactory->CreateStripeMapUpdateEvent(stripe);
    stripeMapUpdate->SetCallee(callback);

    if (journal->IsEnabled() == true)
    {
        StripeAddr oldAddr = stripeMap->GetLSA(stripe->GetVsid());

        result = journalWriter->AddStripeMapUpdatedLog(stripe, oldAddr, stripeMapUpdate);
    }
    else
    {
        bool executionSuccessful = stripeMapUpdate->Execute();
        if (unlikely(false == executionSuccessful))
        {
            POS_EVENT_ID eventId =
                EID(NFLSH_STRIPE_DEBUG_UPDATE);
            POS_TRACE_ERROR(static_cast<int>(eventId),
                "Stripe Map Update Request : stripe.vsid : {}", stripe->GetVsid());

            eventScheduler->EnqueueEvent(stripeMapUpdate);
        }
    }
    return result;
}

int
MetaUpdater::UpdateGcMap(StripeSmartPtr stripe, GcStripeMapUpdateList mapUpdateInfoList, std::map<SegmentId, uint32_t> invalidSegCnt, CallbackSmartPtr callback)
{
    int result = 0;

    CallbackSmartPtr gcMapUpdate =
        metaEventFactory->CreateGcMapUpdateEvent(stripe, mapUpdateInfoList, invalidSegCnt);
    gcMapUpdate->SetCallee(callback);

    if (journal->IsEnabled() == true)
    {
        result = journalWriter->AddGcStripeFlushedLog(mapUpdateInfoList, gcMapUpdate);
    }
    else
    {
        bool executionSuccessful = gcMapUpdate->Execute();
        if (unlikely(false == executionSuccessful))
        {
            POS_EVENT_ID eventId =
                EID(NFLSH_STRIPE_DEBUG_UPDATE);
            POS_TRACE_ERROR(static_cast<int>(eventId),
                "Stripe Map Update Request : stripe.vsid : {}", stripe->GetVsid());

            eventScheduler->EnqueueEvent(gcMapUpdate);
        }
    }
    return result;
}

} // namespace pos
