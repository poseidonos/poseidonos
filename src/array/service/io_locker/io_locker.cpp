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

#include "io_locker.h"

#include "src/include/array_mgmt_policy.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
IOLocker::IOLocker(void)
{
    lockers = new StripeLocker*[ArrayMgmtPolicy::MAX_ARRAY_CNT];
    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        lockers[i] = nullptr;
    }
}

IOLocker::~IOLocker(void)
{
    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        if (lockers[i] != nullptr)
        {
            delete lockers[i];
            lockers[i] = nullptr;
        }
    }
    delete lockers;
    lockers = nullptr;
}

bool
IOLocker::Register(string array)
{
    if (_Find(array) == nullptr)
    {
        StripeLocker* locker = new StripeLocker();
        auto ret = tempLockers.emplace(array, locker);
        return ret.second;
    }
    return true;
}

void
IOLocker::Unregister(string array)
{
    StripeLocker* locker = _Find(array);
    if (locker != nullptr)
    {
        _Erase(array);
        delete locker;
        locker = nullptr;
    }
}

bool
IOLocker::TryLock(string array, StripeId val)
{
    StripeLocker* locker = _Find(array);
    if (locker == nullptr)
    {
        return false;
    }

    return locker->TryLock(val);
}

void
IOLocker::Unlock(string array, StripeId val)
{
    StripeLocker* locker = _Find(array);
    if (locker != nullptr)
    {
        locker->Unlock(val);
    }
}

bool
IOLocker::TryChange(string array, LockerMode mode)
{
    StripeLocker* locker = _Find(array);
    if (locker == nullptr)
    {
        return false;
    }

    return locker->TryModeChanging(mode);
}

StripeLocker*
IOLocker::_Find(string array)
{
    auto it = tempLockers.find(array);
    if (it == tempLockers.end())
    {
        return nullptr;
    }

    return it->second;
}

void
IOLocker::_Erase(string array)
{
        tempLockers.erase(array);
}

// new iolocker with array index

bool
IOLocker::Register(unsigned int arrayIndex)
{
    if (lockers[arrayIndex] == nullptr)
    {
        StripeLocker* locker = new StripeLocker();
        lockers[arrayIndex] = locker;
        POS_TRACE_INFO((int)POS_EVENT_ID::LOCKER_DEBUG_MSG,
            "StripeLocker::Register, array:{}", arrayIndex);
        return true;
    }
    return false;
}

void
IOLocker::Unregister(unsigned int arrayIndex)
{
    if (lockers[arrayIndex] != nullptr)
    {
        delete lockers[arrayIndex];
        lockers[arrayIndex] = nullptr;
        POS_TRACE_INFO((int)POS_EVENT_ID::LOCKER_DEBUG_MSG,
            "StripeLocker::Unregister, array:{}", arrayIndex);
    }
}

bool
IOLocker::TryLock(unsigned int arrayIndex, StripeId val)
{
    if (lockers[arrayIndex] == nullptr)
    {
        return false;
    }

    return lockers[arrayIndex]->TryLock(val);
}

void
IOLocker::Unlock(unsigned int arrayIndex, StripeId val)
{
    if (lockers[arrayIndex] != nullptr)
    {
        lockers[arrayIndex]->Unlock(val);
    }
}

bool
IOLocker::TryChange(unsigned int arrayIndex, LockerMode mode)
{
    if (lockers[arrayIndex] == nullptr)
    {
        return false;
    }

    return lockers[arrayIndex]->TryModeChanging(mode);
}
} // namespace pos
