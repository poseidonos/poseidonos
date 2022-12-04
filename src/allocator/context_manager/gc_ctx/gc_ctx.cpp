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

#include "src/allocator/context_manager/block_allocation_status.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
GcCtx::GcCtx(BlockAllocationStatus* allocStatus, uint32_t arrayId)
: blockAllocStatus(allocStatus),
  arrayId(arrayId)
{
    curGcMode = MODE_NO_GC;
    prevGcMode = MODE_NO_GC;
}

uint32_t
GcCtx::GetNormalGcThreshold(void)
{
    return normalGcThreshold;
}

uint32_t
GcCtx::GetUrgentThreshold(void)
{
    return urgentGcThreshold;
}

void
GcCtx::SetNormalGcThreshold(uint32_t inputThreshold)
{
    normalGcThreshold = inputThreshold;
    POS_TRACE_TRACE(EID(GC_THRESHOLD_IS_SET), "normal_threshold:{}, array_id:{}",
        normalGcThreshold, arrayId);
}

void
GcCtx::SetUrgentThreshold(uint32_t inputThreshold)
{
    urgentGcThreshold = inputThreshold;
    POS_TRACE_TRACE(EID(GC_THRESHOLD_IS_SET), "urgent_threshold:{}, array_id:{}",
        urgentGcThreshold, arrayId);
}

GcMode
GcCtx::GetCurrentGcMode(void)
{
    return curGcMode;
}

GcMode
GcCtx::UpdateCurrentGcMode(uint32_t numFreeSegments)
{
    pos::GcMode newGcMode = curGcMode;

    if (urgentGcThreshold >= numFreeSegments)
    {
        newGcMode = MODE_URGENT_GC;
    }
    else if (normalGcThreshold >= numFreeSegments)
    {
        newGcMode = MODE_NORMAL_GC;
    }
    else
    {
        newGcMode = MODE_NO_GC;
    }

    if (curGcMode != newGcMode)
    {
        _PrintInfo(newGcMode, numFreeSegments);
        _UpdateGcMode(newGcMode);

        if (newGcMode == MODE_URGENT_GC)
        {
            blockAllocStatus->ProhibitUserBlockAllocation();
        }
        else
        {
            blockAllocStatus->PermitUserBlockAllocation();
        }
    }

    return curGcMode;
}

void
GcCtx::_UpdateGcMode(pos::GcMode newGcMode)
{
    prevGcMode = curGcMode;
    curGcMode = newGcMode;
}

void
GcCtx::_PrintInfo(pos::GcMode newGcMode, uint32_t numFreeSegments)
{
    if (curGcMode != newGcMode)
    {
        POS_TRACE_INFO(EID(ALLOCATOR_CURRENT_GC_MODE),
            "Change GC STATE from GCState:{} to {}}",
            (uint32_t)curGcMode, (uint32_t)newGcMode);

        // TODO (dh.ihm) want to print out this here...
        /*
        POSMetricValue v;
        v.gauge = curGcMode;
        telPublisher->PublishData(TEL30003_ALCT_GCMODE, v, MT_GAUGE);
        */
    }
}

} // namespace pos
