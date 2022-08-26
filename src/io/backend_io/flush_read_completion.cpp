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

#include "src/io/backend_io/flush_read_completion.h"

#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/include/backend_event.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/io/backend_io/flush_submission.h"
#include "src/logger/logger.h"

namespace pos
{
FlushReadCompletion::FlushReadCompletion(Stripe* stripe, int arrayId, EventScheduler* eventSchedulerArg)
: Callback(false, CallbackType_FlushReadCompletion),
  stripe(stripe),
  arrayId(arrayId),
  eventScheduler(eventSchedulerArg)
{
    SetEventType(BackendEvent_Flush);
}

FlushReadCompletion::~FlushReadCompletion(void)
{
}

bool
FlushReadCompletion::_DoSpecificJob(void)
{
    EventSmartPtr flushEvent(new FlushSubmission(stripe, arrayId));
    if (unlikely(nullptr == flushEvent))
    {
        POS_EVENT_ID eventId =
            EID(FLUSHREAD_FAIL_TO_ALLOCATE_MEMORY);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Fail to allocate memory");
        return false;
    }
    if (nullptr == eventScheduler)
    {
        eventScheduler = EventSchedulerSingleton::Instance();
    }
    eventScheduler->EnqueueEvent(flushEvent);

    return true;
}

} // namespace pos
