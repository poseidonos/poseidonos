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

#include "src/gc/stripe_copy_submission.h"

#include <list>

#include "src/gc/stripe_copier.h"
#include "src/logger/logger.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
StripeCopySubmission::StripeCopySubmission(StripeId baseStripeId,
    VictimStripe* victimStripe,
    CopierMeta* meta)
: Callback(false),
  baseStripeId(baseStripeId),
  meta(meta),
  victimStripe(victimStripe)
{
    loadedReverseMapCount = 0;
#if defined QOS_ENABLED_BE
    SetEventType(BackendEvent_GC);
#endif
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

    for (uint32_t index = 0; index < STRIPES_PER_SEGMENT; index++)
    {
        EventSmartPtr stripeCopier(new StripeCopier(baseStripeId + index,
            &victimStripe[index],
            meta));
        EventArgument::GetEventScheduler()->EnqueueEvent(stripeCopier);
    }

    return true;
}

} // namespace ibofos
