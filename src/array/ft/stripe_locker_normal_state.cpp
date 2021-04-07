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

#include "stripe_locker_normal_state.h"

#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
{
bool
StripeLockerNormalState::TryLock(StripeId id)
{
    std::unique_lock<std::mutex> lock(mtx);
    if (isStateChanging == true)
    {
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
            "normallocker is now changing state. Using stripe {} is refused", id);
        return false;
    }

    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_STRIPE_LOCK,
        "normallocker: stripe {} is using", id);
    workingSet.insert(id);
    return true;
}

void
StripeLockerNormalState::Unlock(StripeId id)
{
    unique_lock<mutex> lock(mtx);
    auto iter = workingSet.find(id);
    if (iter != workingSet.end())
    {
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_STRIPE_UNLOCK,
            "normallocker: stripe {} is released", id);
        workingSet.erase(iter);
    }
}

bool
StripeLockerNormalState::StateChange(StripeLockerMode mode)
{
    // ONLY NORMAL TO BUSY ALLOWED
    if (mode != StripeLockerMode::BUSY)
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
            "normallocker: requested mode {} is invalid", mode);
        return false;
    }

    unique_lock<mutex> lock(mtx);
    if (workingSet.size() == 0)
    {
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
            "locker mode will be changed successfully normal to busy");
        return true;
    }

    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
        "waiting for remaining mfs io (remaining count:{})",
        workingSet.size());

    return false;
}

uint32_t
StripeLockerNormalState::Count()
{
    unique_lock<mutex> lock(mtx);
    return workingSet.size();
}

}; // namespace ibofos
