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

#include "update_data_complete_handler.h"

#include "rebuilder.h"
#include "src/allocator/allocator.h"
#include "src/array/ft/stripe_locker.h"
#include "src/logger/logger.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
UpdateDataCompleteHandler::UpdateDataCompleteHandler(
    uint32_t _t, UbioSmartPtr _u, RebuildBehavior* _b)
: Callback(false),
  targetId(_t),
  ubio(_u),
  behavior(_b)
{
}

bool
UpdateDataCompleteHandler::_DoSpecificJob()
{
    IBOF_TRACE_DEBUG(2831, "UpdateDataComplete, target:{}", targetId);
    RebuildContext* rebuildCtx = behavior->GetContext();

    if (rebuildCtx->id == "META_SSD")
    {
        rebuildCtx->locker->Unlock(targetId);
    }

    uint32_t currentTaskCnt = rebuildCtx->taskCnt -= 1;

    if (currentTaskCnt == 0)
    {
        if (rebuildCtx->id == "USER_DATA")
        {
            AllocatorSingleton::Instance()->ReleaseRebuildSegment(targetId);
        }
        EventSmartPtr nextEvent(new Rebuilder(behavior));
#if defined QOS_ENABLED_BE
        if (GetEventType() == BackendEvent_MetadataRebuild)
        {
            nextEvent->SetEventType(BackendEvent_MetadataRebuild);
        }
        if (GetEventType() == BackendEvent_UserdataRebuild)
        {
            nextEvent->SetEventType(BackendEvent_UserdataRebuild);
        }
#endif
        EventArgument::GetEventScheduler()->EnqueueEvent(nextEvent);
    }

    ubio->FreeDataBuffer();
    ubio = nullptr;

    return true;
}
} // namespace ibofos
