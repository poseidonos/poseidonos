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

#include "stripe_locker_normal_state.h"
#include "src/event/event_manager.h"
#include "src/logger/logger.h"

#include <sstream>

namespace pos
{
bool
StripeLockerNormalState::TryLock(StripeLockInfo lockInfo)
{
    std::unique_lock<std::mutex> lock(mtx);
    workingSet.insert(lockInfo);
    return true;
}

void
StripeLockerNormalState::Unlock(StripeId id)
{
    unique_lock<mutex> lock(mtx);
    auto iter = workingSet.find(StripeLockInfo(id));
    if (iter != workingSet.end())
    {
        workingSet.erase(iter);
    }
}

bool
StripeLockerNormalState::Exists(StripeId id)
{
    return workingSet.find(StripeLockInfo(id)) != workingSet.end();
}

uint32_t
StripeLockerNormalState::Count(void)
{
    unique_lock<mutex> lock(mtx);
    return workingSet.size();
}

void
StripeLockerNormalState::WriteLog(void)
{
    stringstream ss;
    for (auto item : workingSet)
    {
        ss << "(id:" << item.id << ", owner:" << item.owner << "), ";
    }
    POS_TRACE_WARN(EID(BUSY_LOCKER_WARN), "NormalLocker, cnt:{}, items:{}",
        workingSet.size(), ss.str());
}

}; // namespace pos
