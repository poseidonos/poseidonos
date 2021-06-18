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

#include "stripe_map_update.h"

#include "src/allocator/stripe/stripe.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/backend_io/stripe_map_update_completion.h"
#include "src/journal_service/journal_service.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"

namespace pos
{
StripeMapUpdate::StripeMapUpdate(Stripe* stripe, std::string& arrayName)
: StripeMapUpdate(stripe,
    MapperServiceSingleton::Instance()->GetIStripeMap(arrayName),
    JournalServiceSingleton::Instance(), EventSchedulerSingleton::Instance(),
    arrayName)
{
}

StripeMapUpdate::StripeMapUpdate(Stripe* stripe, IStripeMap* iStripeMap,
    JournalService* journalService, EventScheduler* scheduler, std::string& arrayName)
: stripe(stripe),
  iStripeMap(iStripeMap),
  journalService(journalService),
  eventScheduler(scheduler),
  arrayName(arrayName)
{
}

StripeMapUpdate::~StripeMapUpdate(void)
{
}

bool
StripeMapUpdate::Execute(void)
{
    bool executionSuccessful = false;

    if (journalService->IsEnabled(arrayName))
    {
        IJournalWriter* journal = journalService->GetWriter(arrayName);

        MpageList dirty = iStripeMap->GetDirtyStripeMapPages(stripe->GetVsid());
        StripeAddr oldAddr = iStripeMap->GetLSA(stripe->GetVsid());
        EventSmartPtr callbackEvent(new StripeMapUpdateCompletion(stripe, arrayName));

        int result = journal->AddStripeMapUpdatedLog(stripe, oldAddr,
            dirty, callbackEvent);

        executionSuccessful = (result == 0);
    }
    else
    {
        StripeMapUpdateCompletion event(stripe, arrayName);
        executionSuccessful = event.Execute();
        if (unlikely(false == executionSuccessful))
        {
            POS_EVENT_ID eventId =
                POS_EVENT_ID::NFLSH_STRIPE_DEBUG_UPDATE;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                PosEventId::GetString(eventId));

            EventSmartPtr event(new StripeMapUpdateCompletion(stripe, arrayName));
            eventScheduler->EnqueueEvent(event);

            executionSuccessful = true;
        }
    }

    POS_EVENT_ID eventId = POS_EVENT_ID::NFLSH_STRIPE_DEBUG_UPDATE;
    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_FLUSH, eventId, PosEventId::GetString(eventId),
        stripe->GetVsid(),
        static_cast<uint32_t>(executionSuccessful));

    return executionSuccessful;
}

} // namespace pos
