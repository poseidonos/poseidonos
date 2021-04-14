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

#include "get_gc_threshold_wbt_command.h"

#include "src/allocator_service/allocator_service.h"
#include "src/allocator/i_allocator_wbt.h"
#include "src/gc/garbage_collector.h"

namespace pos
{
GetGcThresholdWbtCommand::GetGcThresholdWbtCommand(void)
:   GcWbtCommand(GET_GC_THRESHOLD, "get_gc_threshold")
{
}

GetGcThresholdWbtCommand::~GetGcThresholdWbtCommand(void)
{
}

int
GetGcThresholdWbtCommand::Execute(Args &argv, JsonElement &elem)
{
    GarbageCollector* gc = _GetGC("");
    if (gc == nullptr)
    {
        return -1;
    }
    int isEnabled = gc->IsEnabled();
    if (0 != isEnabled)
    {
        return -1;
    }

    ISegmentCtx* iSegmentCtx = AllocatorServiceSingleton::Instance()->GetISegmentCtx("");
    uint32_t numGcThreshold = iSegmentCtx->GetGcThreshold();
    uint32_t numUrgentThreshold = iSegmentCtx->GetUrgentThreshold();

    JsonElement thresholdElem("gc_threshold");
    thresholdElem.SetAttribute(JsonAttribute("normal", numGcThreshold));
    thresholdElem.SetAttribute(JsonAttribute("urgent", numUrgentThreshold));

    elem.SetElement(thresholdElem);

    return 0;
}

} // namespace pos
