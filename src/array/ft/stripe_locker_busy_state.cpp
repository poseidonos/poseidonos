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

#include "stripe_locker_busy_state.h"

#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
{
bool
StripeLockerBusyState::TryLock(StripeId id)
{
    std::unique_lock<std::mutex> lock(mtx);
    if (isStateChanging == true)
    {
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
            "busylocker is now changing state. Using stripe {} is refused", id);
        return false;
    }

    if (workingSet.find(id) == workingSet.end())
    {
        workingSet.insert(id);
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_STRIPE_LOCK,
            "busylocker: stripe {} is locked", id);
        return true;
    }

    return false;
}

void
StripeLockerBusyState::Unlock(StripeId id)
{
    unique_lock<mutex> lock(mtx);
    workingSet.erase(id);
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_STRIPE_UNLOCK,
        "busylocker: stripe {} is unlocked", id);
}

bool
StripeLockerBusyState::StateChange(StripeLockerMode mode)
{
    // ONLY BUSY TO NORMAL ALLOWED
    if (mode != StripeLockerMode::NORMAL)
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
            "busylocker: requested mode {} is invalid", mode);
        return false;
    }

    unique_lock<mutex> lock(mtx);
    if (workingSet.size() == 0)
    {
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
            "locker mode will be changed successfully busy to normal");
        return true;
    }

    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
        "waiting for releasing busy lock (remaining count:{})",
        workingSet.size());
    return false;
}

uint32_t
StripeLockerBusyState::Count()
{
    unique_lock<mutex> lock(mtx);
    return workingSet.size();
}

}; // namespace ibofos
