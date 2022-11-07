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

#include "force_flush_locker.h"
#include "src/logger/logger.h"
#include "src/include/pos_event_id.h"

namespace pos
{

ForceFlushLocker::ForceFlushLocker(void)
{
    normalLocker = new ForceFlushLockerNormalState();
    busyLocker = new ForceFlushLockerBusyState();
}

ForceFlushLocker::~ForceFlushLocker(void)
{
    delete busyLocker;
    delete normalLocker;
}

bool
ForceFlushLocker::TryForceFlushLock(uint32_t volId)
{
    unique_lock<mutex> lock(lockerMtx);
    if (normalLocker->Exists(volId) == true)
    {
        return false;
    }
    bool ret = busyLocker->TryLock(volId);
    if (ret == true)
    {
        POS_TRACE_INFO(EID(GC_FORCE_FLUSH_LOCK_ACQUIRED), "vol_id:{}", volId);
    }
    else
    {
        POS_TRACE_INFO(EID(GC_FORCE_FLUSH_TRYLOCK_FAILED), "vol_id:{}", volId);
    }
    return ret;
}

void
ForceFlushLocker::UnlockForceFlushLock(uint32_t volId)
{
    unique_lock<mutex> lock(lockerMtx);
    busyLocker->Unlock(volId);
    POS_TRACE_INFO(EID(GC_FORCE_FLUSH_LOCK_RELEASED), "vol_id:{}", volId);
}

bool
ForceFlushLocker::TryLock(uint32_t volId)
{
    unique_lock<mutex> lock(lockerMtx);
    if (busyLocker->Exists(volId) == true)
    {
        POS_TRACE_DEBUG(EID(GC_NORMAL_FLUSH_TRYLOCK_FAILED), "vol_id:{}", volId);
        return false;
    }
    return normalLocker->TryLock(volId);
}

void
ForceFlushLocker::Unlock(uint32_t volId)
{
    unique_lock<mutex> lock(lockerMtx);
    normalLocker->Unlock(volId);
}

void
ForceFlushLocker::Reset(uint32_t volId)
{
    unique_lock<mutex> lock(lockerMtx);
    normalLocker->Reset(volId);
    busyLocker->Reset(volId);
    POS_TRACE_INFO(EID(GC_FLUSH_LOCK_RESET), "vol_id:{}", volId);
}

} // namespace pos
