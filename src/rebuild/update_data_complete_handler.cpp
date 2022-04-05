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

#include "update_data_complete_handler.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/backend_event.h"

namespace pos
{
UpdateDataCompleteHandler::UpdateDataCompleteHandler(
    uint32_t _t, UbioSmartPtr _u, RebuildBehavior* _b)
: Callback(false, CallbackType_UpdateDataCompleteHandler),
  targetId(_t),
  ubio(_u),
  behavior(_b)
{
}

bool
UpdateDataCompleteHandler::_DoSpecificJob()
{
    if (_GetErrorCount() > 0)
    {
        RebuildContext* rebuildCtx = behavior->GetContext();
        POS_TRACE_ERROR((int)POS_EVENT_ID::REBUILD_FAILED,
                "Failed to update data during rebuild - Array:{}, Partition:{}, ID:{}",
                rebuildCtx->array , PARTITION_TYPE_STR[rebuildCtx->part], targetId);
        rebuildCtx->SetResult(RebuildState::FAIL);
    }

    return behavior->Complete(targetId, ubio);
}
} // namespace pos
