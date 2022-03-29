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

#include "stripe_locker.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{

StripeLocker::StripeLocker(void)
{
    normalLocker = new StripeLockerNormalState();
    busyLocker = new StripeLockerBusyState();
}

StripeLocker::~StripeLocker(void)
{
    delete busyLocker;
    delete normalLocker;
}

bool
StripeLocker::TryBusyLock(StripeId from, StripeId to)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::LOCKER_DEBUG_MSG,
        "StripeLocker::TryBusyLock-try {} ~ {}", from, to);

    unique_lock<mutex> lock(lockerMtx);
    isBusyRangeChanging = true;

    if (busyLocker->Count() > 0)
    {
        return false;
    }

    for (StripeId id = from; id <= to; id++)
    {
        if (normalLocker->Exists(id) == true)
        {
            return false;
        }
    }

    // POS_TRACE_DEBUG((int)POS_EVENT_ID::LOCKER_DEBUG_MSG,
    //     "StripeLocker::TryBusyLock, {} ~ {}", from, to);
    if (busyRange == nullptr)
    {
        busyRange = new BusyRange();
    }
    busyRange->SetRange(from, to);

    for (StripeId id = from; id <= to; id++)
    {
        bool ret = busyLocker->TryLock(id);
        assert(ret == true);
    }
    isBusyRangeChanging = false;
    return true;
}

bool
StripeLocker::ResetBusyLock(void)
{
    unique_lock<mutex> lock(lockerMtx);
    if (busyLocker->Count() > 0)
    {
        return false;
    }

    POS_TRACE_DEBUG((int)POS_EVENT_ID::LOCKER_DEBUG_MSG,
        "StripeLocker::ResetBusyLock");
    delete busyRange;
    busyRange = nullptr;
    isBusyRangeChanging = false;
    return true;
}

bool
StripeLocker::TryLock(StripeId id)
{
    unique_lock<mutex> lock(lockerMtx, defer_lock);
    if (lock.try_lock())
    {
        if (isBusyRangeChanging == true)
        {
            // POS_TRACE_DEBUG((int)POS_EVENT_ID::LOCKER_DEBUG_MSG,
            //     "Locker is now state changing. Stripe {} is refused", id);
            return false;
        }
        if (busyRange != nullptr && busyRange->IsBusy(id) == true)
        {
            return busyLocker->TryLock(id);
        }
        return normalLocker->TryLock(id);
    }
    else
    {
        return false;
    }
}

void
StripeLocker::Unlock(StripeId id)
{
    unique_lock<mutex> lock(lockerMtx);
    if (busyRange != nullptr && busyRange->IsBusy(id) == true)
    {
        busyLocker->Unlock(id);
        return;
    }
    else
    {
        normalLocker->Unlock(id);
    }
}

} // namespace pos
