/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
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

#include "io_timeout_checker.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"

namespace pos
{
IoTimeoutChecker::IoTimeoutChecker(void)
: initialize(false),
publisher(nullptr),
currentIdx(0)
{
}

IoTimeoutChecker::~IoTimeoutChecker(void)
{
    if (nullptr != publisher)
    {
        publisher->TimerStop();
        delete publisher;
    }
}

void
IoTimeoutChecker::Initialize(void)
{
    initialize = true;

    publisher = new PublishPendingIo(TIMER_RESOLUTION_MS, CHECK_RESOLUSTION_RANGE);

    for (int idx = 0; idx < CallbackType::Total_CallbackType_Cnt; idx++)
    {
        pendingIoCnt[idx].oldestIdx = 0;        
    }
}

void
IoTimeoutChecker::MoveOldestIdx(CallbackType callbackType)
{
    if (false == initialize)
    {
        return;
    }

    uint32_t newPosition = pendingIoCnt[callbackType].oldestIdx;
    while(pendingIoCnt[callbackType].pendingIoCnt[newPosition] == 0)
    {
        if (newPosition == currentIdx)
        {
            break;
        }
        newPosition++;
        if (newPosition >= CHECK_RESOLUSTION_RANGE)
        {
            newPosition = 0;
        }
    }
    pendingIoCnt[callbackType].oldestIdx = newPosition;
}

void
IoTimeoutChecker::MoveCurrentIdx(uint64_t pendingTime)
{
    if (false == initialize)
    {
        return;
    }

    currentIdx = pendingTime;

    for (int idx = 0 ; idx < CallbackType::Total_CallbackType_Cnt; idx++)
    {
        if (currentIdx == pendingIoCnt[idx].oldestIdx)
        {
            POS_TRACE_WARN(EID(INVALID_RANGE_OF_PENDING_IO_CHECKING), "Invalid Range Type : {} ", idx);
        }
    }
}

void
IoTimeoutChecker::IncreasePendingCnt(CallbackType callbackType, uint64_t pendingTime)
{
    if (false == initialize)
    {
        return;
    }

    pendingIoCnt[callbackType].pendingIoCnt[pendingTime]++;
}

void
IoTimeoutChecker::DecreasePendingCnt(CallbackType callbackType, uint64_t pendingTime)
{
    if (false == initialize)
    {
        return;
    }

    pendingIoCnt[callbackType].pendingIoCnt[pendingTime]--;

    if (pendingIoCnt[callbackType].pendingIoCnt[pendingTime] < 0)
    {        
        POS_TRACE_WARN(EID(INVALID_RANGE_OF_PENDING_IO_CHECKING), "Pending Callback Type : {} ", callbackType);
    }
}

bool
IoTimeoutChecker::_CheckPeningOverTime(CallbackType callbackType)
{    
    if (false == initialize)
    {
        return false;
    }

    uint32_t calPendingTime = 0;
    bool ret = false;

    if (currentIdx >= pendingIoCnt[callbackType].oldestIdx)
    {
        calPendingTime = currentIdx - pendingIoCnt[callbackType].oldestIdx;
    }
    else
    {
        calPendingTime = currentIdx + CHECK_RESOLUSTION_RANGE
                            - pendingIoCnt[callbackType].oldestIdx;
    }

    if (CHECK_TIMEOUT_THRESHOLD <= calPendingTime)
    {
        POS_TRACE_WARN(EID(PENDING_IO_TIMEOUT), "Pending Callback Type : {} current Time Idx {}, oldest Time Idx {} ",
                                            callbackType, currentIdx, pendingIoCnt[callbackType].oldestIdx);
        ret = true;
    }

    return ret;
}


bool 
IoTimeoutChecker::FindPendingIo(CallbackType callbackType)
{    
    if (false == initialize)
    {
        return false;
    }
 
    return _CheckPeningOverTime(callbackType);
}

uint64_t
IoTimeoutChecker::GetCurrentRoughTime(void)
{    
    if (false == initialize)
    {
        return 0;
    }

    return publisher->GetCurrentRoughTime();
}

void
IoTimeoutChecker::GetPendingIoCount(CallbackType callbackType, std::vector<int> &pendingIoCntList)
{
    if (false == initialize)
    {
        return;
    }

    POS_TRACE_INFO(EID(PENDING_IO_TIMEOUT), "currentIdx : {} oldestidx: {}", currentIdx, pendingIoCnt[callbackType].oldestIdx);

    if (currentIdx >= pendingIoCnt[callbackType].oldestIdx)
    {
        for (uint32_t idx = pendingIoCnt[callbackType].oldestIdx ; idx < currentIdx; idx++)
        {
            pendingIoCntList.push_back(pendingIoCnt[callbackType].pendingIoCnt[idx]);
        }
    }
    else
    {
        for (uint32_t idx = pendingIoCnt[callbackType].oldestIdx ; idx < CHECK_RESOLUSTION_RANGE; idx++)
        {
            pendingIoCntList.push_back(pendingIoCnt[callbackType].pendingIoCnt[idx]);
        }
        for (uint32_t idx = 0 ; idx < currentIdx; idx++)
        {
            pendingIoCntList.push_back(pendingIoCnt[callbackType].pendingIoCnt[idx]);
        }
    }

}

} // namespace pos
