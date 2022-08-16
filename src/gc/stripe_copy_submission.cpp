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

#include "src/gc/stripe_copy_submission.h"

#include <list>
#include <memory>

#include "src/event_scheduler/event_scheduler.h"
#include "src/gc/stripe_copier.h"
#include "src/include/backend_event.h"
#include "src/logger/logger.h"

namespace pos
{
StripeCopySubmission::StripeCopySubmission(StripeId baseStripeId, CopierMeta* meta, uint32_t copyIndex)
: StripeCopySubmission(baseStripeId, meta, copyIndex,
      nullptr, EventSchedulerSingleton::Instance())
{
}

StripeCopySubmission::StripeCopySubmission(StripeId baseStripeId, CopierMeta* meta, uint32_t copyIndex,
    EventSmartPtr inputEvent, EventScheduler* inputEventScheduler)
: Callback(false, CallbackType_StripeCopySubmission),
  baseStripeId(baseStripeId),
  meta(meta),
  copyIndex(copyIndex),
  isLoaded(false),
  inputEvent(inputEvent),
  eventScheduler(inputEventScheduler)
{
    SetEventType(BackendEvent_GC);
}

StripeCopySubmission::~StripeCopySubmission(void)
{
}

bool
StripeCopySubmission::_DoSpecificJob(void)
{
    if (_GetErrorCount() != 0)
    {
        // TODO(jg121.lim) : reverse map load error handling
    }

    if (isLoaded == false)
    {
        for (uint32_t index = 0; index < meta->GetStripePerSegment(); index++)
        {
            meta->GetVictimStripe(copyIndex, index)->LoadValidBlock();
        }

        POS_TRACE_DEBUG(EID(GC_LOAD_VALID_BLOCKS),
            "valid blocks loaded, startStripeId:{}", baseStripeId);

        isLoaded = true;
    }

    if (meta->IsReadytoCopy(copyIndex) == false)
    {
        return false;
    }

    EventSmartPtr stripeCopier;
    for (uint32_t index = 0; index < CopierMeta::GC_CONCURRENT_COUNT; index++)
    {
        if (nullptr == inputEvent)
        {
            stripeCopier = std::make_shared<StripeCopier>(baseStripeId + index, meta, copyIndex);
        }
        else
        {
            stripeCopier = inputEvent;
        }

        eventScheduler->EnqueueEvent(stripeCopier);
    }

    POS_TRACE_DEBUG(EID(GC_STRIPE_COPIER_SUBMIT),
        "stripe copier submit, startStripeId:{}", baseStripeId);

    return true;
}

} // namespace pos
