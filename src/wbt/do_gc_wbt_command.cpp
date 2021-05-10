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

#include "do_gc_wbt_command.h"

#include "src/allocator_service/allocator_service.h"
#include "src/allocator/i_segment_ctx.h"
#include "src/gc/garbage_collector.h"

namespace pos
{
DoGcWbtCommand::DoGcWbtCommand(void)
:   GcWbtCommand(DO_GC, "do_gc")
{
}

DoGcWbtCommand::~DoGcWbtCommand(void)
{
}

int
DoGcWbtCommand::Execute(Args &argv, JsonElement &elem)
{
    int returnValue = -1;
    if (!argv.contains("array"))
    {
        return returnValue;
    }
    std::string arrayName = argv["array"].get<std::string>();
    GarbageCollector* gc = _GetGC(arrayName);

    if (gc == nullptr)
    {
        return returnValue;
    }

    int isEnabled = gc->IsEnabled();

    if (0 != isEnabled)
    {
        return returnValue;
    }

    int gcStarted = gc->Start();

    if (0 != gcStarted)
    {
        return returnValue;
    }

    ISegmentCtx* iSegmentCtx = AllocatorServiceSingleton::Instance()->GetISegmentCtx("");
    SegmentId victimId = iSegmentCtx->GetGCVictimSegment();

    if (UNMAP_SEGMENT == victimId)
    {
        return returnValue;
    }

    returnValue = gc->DisableThresholdCheck();

    return returnValue;
}

} // namespace pos
