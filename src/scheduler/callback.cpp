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

#include "callback.h"

#include "event_argument.h"
#include "event_scheduler.h"
#include "src/dump/dump_shared_ptr.h"
#include "src/include/branch_prediction.h"
#include "src/include/ibof_event_id.hpp"
#include "src/lib/system_timeout_checker.h"
#include "src/logger/logger.h"

namespace ibofos
{
Callback::Callback(bool isFrontEnd, uint32_t weight)
: Event(isFrontEnd),
  errorCount(0),
  errorBitMap((int)CallbackError::CALLBACK_ERROR_MAX_COUNT),
  completionCount(0),
  waitingCount(1),
  weight(weight),
  callee(nullptr),
  timeoutChecker(nullptr),
  returnAddress(nullptr)
{
    if (DumpSharedModuleInstanceEnable::debugLevelEnable)
    {
        returnAddress = __builtin_return_address(Callback::CALLER_FRAME);
        timeoutChecker = new SystemTimeoutChecker;
        timeoutChecker->SetTimeout(DEFAULT_TIMEOUT_NS);
    }
}

Callback::~Callback(void)
{
    if (DumpSharedModuleInstanceEnable::debugLevelEnable && timeoutChecker != nullptr)
    {
        if (timeoutChecker->CheckTimeout())
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::CALLBACK_TIMEOUT;
            IBOF_TRACE_DEBUG_IN_MEMORY(
                ModuleInDebugLogDump::CALLBACK_TIMEOUT,
                eventId,
                IbofEventId::GetString(eventId),
                returnAddress);
        }
        delete timeoutChecker;
    }
}

bool
Callback::Execute(void)
{
    bool done = _DoSpecificJob();

    if (done)
    {
        _InvokeCallee();
    }

    return done;
}

uint32_t
Callback::_GetErrorCount(void)
{
    return errorCount;
}

CallbackError
Callback::_GetMostCriticalError(void)
{
    return static_cast<CallbackError>(errorBitMap.FindFirstSetBit(0));
}

void
Callback::InformError(CallbackError inputCallbackError)
{
    if (inputCallbackError != CallbackError::SUCCESS)
    {
        errorCount += 1;
        errorBitMap.SetBit(static_cast<int>(inputCallbackError));
    }
}

void
Callback::_InvokeCallee(void)
{
    if (callee == nullptr)
    {
        return;
    }

    bool isOkToCall = callee->_NotifyCompletion(errorCount,
        errorBitMap, weight);
    if (isOkToCall == true)
    {
        bool done = callee->Execute();

        if (likely(done))
        {
            return;
        }

        EventArgument::GetEventScheduler()->EnqueueEvent(callee);
    }
}

void
Callback::SetWaitingCount(uint32_t inputWaitingCount)
{
    waitingCount = inputWaitingCount;
}

void
Callback::SetCallee(CallbackSmartPtr inputCallee)
{
    if (unlikely(_IsInvalidCallee(inputCallee)))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::CALLBACK_INVALID_CALLEE;
        IBOF_TRACE_ERROR(static_cast<uint32_t>(eventId),
            IbofEventId::GetString(eventId));
        return;
    }

    if (unlikely(_IsCalleeSet()))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::CALLBACK_INVALID_CALLEE;
        IBOF_TRACE_ERROR(static_cast<uint32_t>(eventId),
            IbofEventId::GetString(eventId));
        return;
    }

    callee = inputCallee;
}

bool
Callback::_IsInvalidCallee(CallbackSmartPtr inputCallee)
{
    bool isInvalidCallee = (inputCallee == nullptr);
    return isInvalidCallee;
}

bool
Callback::_IsCalleeSet(void)
{
    bool isCalleeSet = (_IsInvalidCallee(callee) == false);
    return isCalleeSet;
}

bool
Callback::_NotifyCompletion(uint32_t transferredErrorCount,
    BitMapMutex& inputErrorBitMap,
    uint32_t transferredWeight)
{
    if (transferredErrorCount > 0)
    {
        errorBitMap.SetBitMap(inputErrorBitMap);
        errorCount += transferredErrorCount;
    }

    bool isOkToBeCalled = false;
    uint32_t localWaitingCount = waitingCount;
    uint32_t increasedCompletionCount = (completionCount += transferredWeight);
    if (increasedCompletionCount == localWaitingCount)
    {
        isOkToBeCalled = true;
    }
    else if (unlikely(increasedCompletionCount > localWaitingCount))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::CALLBACK_INVALID_COUNT;
        IBOF_TRACE_ERROR(static_cast<uint32_t>(eventId),
            IbofEventId::GetString(eventId));
    }

    return isOkToBeCalled;
}

} // namespace ibofos
