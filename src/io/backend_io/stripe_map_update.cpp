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

#include "src/allocator/stripe.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/backend_io/stripe_map_update_completion.h"
#include "src/journal_manager/journal_manager.h"
#include "src/mapper/mapper.h"

namespace ibofos
{
StripeMapUpdate::StripeMapUpdate(Stripe* stripe)
: stripe(stripe),
  mapper(MapperSingleton::Instance()),
  journalManager(JournalManagerSingleton::Instance())
{
}

bool
StripeMapUpdate::Execute(void)
{
    MpageList dirty = mapper->GetDirtyStripeMapPages(stripe->GetVsid());
    StripeAddr oldAddr = mapper->GetLSA(stripe->GetVsid());

    EventSmartPtr event(new StripeMapUpdateCompletion(stripe));
    bool logWriteRequestSuccessful = journalManager->AddStripeMapUpdatedLog(stripe, oldAddr, dirty, event);

    IBOF_EVENT_ID eventId = IBOF_EVENT_ID::NFLSH_STRIPE_DEBUG_UPDATE;
    IBOF_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_FLUSH, eventId, IbofEventId::GetString(eventId),
        stripe->GetVsid(),
        static_cast<uint32_t>(logWriteRequestSuccessful));

    return logWriteRequestSuccessful;
}

} // namespace ibofos
