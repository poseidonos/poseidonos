/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "meta_update_call_back.h"

namespace pos
{
MetaUpdateCallback::MetaUpdateCallback(bool isFrontEnd, ISegmentCtx* segmentCtx_,
    CallbackType type, uint32_t weight, SystemTimeoutChecker* timeoutCheckerArg,
    EventScheduler* eventSchedulerArg)
: Callback(isFrontEnd, type, weight, timeoutCheckerArg, eventSchedulerArg),
  segmentCtx(segmentCtx_),
  logGroupId(UINT32_MAX)
{
}

// LCOV_EXCL_START
MetaUpdateCallback::~MetaUpdateCallback(void)
{
}
// LCOV_EXCL_STOP

void
MetaUpdateCallback::SetLogGroupId(int groupId)
{
    logGroupId = groupId;
}

int
MetaUpdateCallback::GetLogGroupId(void)
{
    return logGroupId;
}

void
MetaUpdateCallback::ValidateBlks(VirtualBlks blks)
{
    segmentCtx->ValidateBlocksWithGroupId(blks, logGroupId);
}

bool
MetaUpdateCallback::InvalidateBlks(VirtualBlks blks, bool isForced)
{
    return segmentCtx->InvalidateBlocksWithGroupId(blks, isForced, logGroupId);
}

bool
MetaUpdateCallback::UpdateOccupiedStripeCount(StripeId lsid)
{
    return segmentCtx->UpdateStripeCount(lsid, logGroupId);
}
} // namespace pos
