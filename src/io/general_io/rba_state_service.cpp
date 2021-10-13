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


#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/io/general_io/rba_state_service.h"

namespace pos
{
RBAStateService::RBAStateService(void)
{
    for (int arrayId = 0 ; arrayId < ArrayMgmtPolicy::MAX_ARRAY_CNT; arrayId++)
    {
        items[arrayId] = nullptr;
    }
}

RBAStateService::~RBAStateService(void)
{
}

void
RBAStateService::Register(int arrayId, RBAStateManager* mgr)
{
    if (items[arrayId] != nullptr)
    {
        POS_TRACE_ERROR(9999,
            "RBA State manager for array {} already exists", arrayId);
    }

    items[arrayId] = mgr;
    POS_TRACE_DEBUG(9999, "RBA State manager for array {} is registered", arrayId);
}

void
RBAStateService::Unregister(int arrayId)
{
    if (items[arrayId] == nullptr)
    {
        POS_TRACE_ERROR(9999,
            "RBA State manager for array {} does not exist", arrayId);
    }

    items[arrayId] = nullptr;

    POS_TRACE_DEBUG(9999, "RBA State manager for array {} is unregistered", arrayId);
}

RBAStateManager*
RBAStateService::GetRBAStateManager(int arrayId)
{
    return items[arrayId];
}

} // namespace pos
