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

#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
GcCtx::GcCtx(void)
{
    normalGcthreshold = DEFAULT_GC_THRESHOLD;
    urgentGcthreshold = DEFAULT_URGENT_THRESHOLD;
    curGcMode = MODE_NO_GC;
}

int
GcCtx::GetNormalGcThreshold(void)
{
    return normalGcthreshold;
}

int
GcCtx::GetUrgentThreshold(void)
{
    return urgentGcthreshold;
}

void
GcCtx::SetNormalGcThreshold(int inputThreshold)
{
    normalGcthreshold = inputThreshold;
}

void
GcCtx::SetUrgentThreshold(int inputThreshold)
{
    urgentGcthreshold = inputThreshold;
}

GcMode
GcCtx::GetCurrentGcMode(int numFreeSegments)
{
    if (urgentGcthreshold >= numFreeSegments)
    {
        if (curGcMode != MODE_URGENT_GC)
        {
            POS_TRACE_INFO(EID(ALLOCATOR_CURRENT_GC_MODE), "Change GC STATE from GCState:{} to URGENT GC MODE, free segment count:{}", (int)curGcMode, numFreeSegments);
        }
        curGcMode = MODE_URGENT_GC;
    }
    else if (normalGcthreshold >= numFreeSegments)
    {
        if (curGcMode != MODE_NORMAL_GC)
        {
            POS_TRACE_INFO(EID(ALLOCATOR_CURRENT_GC_MODE), "Change GC STATE from GCState:{} to NORMAL GC MODE, free segment count:{}", (int)curGcMode, numFreeSegments);
        }
        curGcMode = MODE_NORMAL_GC;
    }
    else
    {
        if (curGcMode != MODE_NO_GC)
        {
            POS_TRACE_INFO(EID(ALLOCATOR_CURRENT_GC_MODE), "Change GC STATE from GCState:{} to NO GC MODE, free segment count:{}", (int)curGcMode, numFreeSegments);
        }
        curGcMode = MODE_NO_GC;
    }
    return curGcMode;
}

} // namespace pos
