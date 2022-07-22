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

#include "stripe_locker_busy_state.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

#include <sstream>

namespace pos
{
bool
StripeLockerBusyState::TryLock(StripeLockInfo lockInfo)
{
    std::unique_lock<std::mutex> lock(mtx);
    if (busySet.find(lockInfo) == busySet.end())
    {
        busySet.insert(lockInfo);
        return true;
    }
    return false;
}

void
StripeLockerBusyState::Unlock(StripeId id)
{
    unique_lock<mutex> lock(mtx);
    busySet.erase(StripeLockInfo(id));
}

bool
StripeLockerBusyState::Exists(StripeId id)
{
    return busySet.find(StripeLockInfo(id)) != busySet.end();
}

uint32_t
StripeLockerBusyState::Count(void)
{
    unique_lock<mutex> lock(mtx);
    return busySet.size();
}

void
StripeLockerBusyState::Clear(void)
{
    unique_lock<mutex> lock(mtx);
    busySet.clear();
}

void
StripeLockerBusyState::WriteLog(void)
{
    stringstream ss;
    for (auto item : busySet)
    {
        ss << "(id:" << item.id << ", owner:" << item.owner << "), ";
    }
    POS_TRACE_WARN(EID(BUSY_LOCKER_WARN), "BusyLocker, cnt:{}, items:{}",
        busySet.size(), ss.str());
}

}; // namespace pos
