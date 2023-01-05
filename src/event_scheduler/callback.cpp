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

#include "callback.h"

#include <air/Air.h>

#include "src/debug_lib/dump_buffer.h"
#include "src/debug_lib/debug_info_queue.h"
#include "src/debug_lib/debug_info_queue.hpp"
#include "src/debug_lib/dump_shared_ptr.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/backend_event.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/lib/system_timeout_checker.h"
#include "src/logger/logger.h"
#include "src/signal_handler/user_signal_interface.h"

namespace pos
{
const uint32_t Callback::CALLER_FRAME = 0;
// 30 sec, default timeout
const uint64_t Callback::DEFAULT_TIMEOUT_NS = 5ULL * 1000 * 1000 * 1000;
const uint64_t Callback::MAX_TIMEOUT_SEC = 120ULL;
uint64_t Callback::timeoutNs = Callback::DEFAULT_TIMEOUT_NS;
static const char* DUMP_NAME = "Callback_Error";
static const bool DEFAULT_DUMP_ON = true;

DebugInfoQueue<DumpBuffer> dumpCallbackError(DUMP_NAME,
    DebugInfoQueue<DumpBuffer>::MAX_ENTRIES_FOR_CALLBACK_ERROR,
    DEFAULT_DUMP_ON);

Callback::Callback(bool isFrontEnd, CallbackType type, uint32_t weight, SystemTimeoutChecker* timeoutCheckerArg, EventScheduler* eventSchedulerArg)
: Event(isFrontEnd),
  errorCount(0),
  errorBitMap((int)IOErrorType::CALLBACK_ERROR_MAX_COUNT),
  completionCount(0),
  waitingCount(1),
  weight(weight),
  callee(nullptr),
  timeoutChecker(timeoutCheckerArg),
  returnAddress(nullptr),
  executed(false),
  type(type),
  eventScheduler(eventSchedulerArg)
{
    objectAddress = reinterpret_cast<uint64_t>(this);
    airlog("LAT_Callback", "alloc", type, objectAddress);
    airlog("Callback_Constructor", "internal", type, 1);
    if (DumpSharedModuleInstanceEnable::debugLevelEnable)
    {
        returnAddress = __builtin_return_address(Callback::CALLER_FRAME);
        if (nullptr == timeoutChecker)
        {
            timeoutChecker = new SystemTimeoutChecker;
            timeoutChecker->SetTimeout(timeoutNs);
        }
    }
    if (eventScheduler == nullptr)
    {
        eventScheduler = EventSchedulerSingleton::Instance();
    }

    ioTimeoutChecker = IoTimeoutCheckerSingleton::Instance();

    createdTime = ioTimeoutChecker->GetCurrentRoughTime();
    ioTimeoutChecker->IncreasePendingCnt(type, createdTime);
    
}

// LCOV_EXCL_START
Callback::~Callback(void)
{
    airlog("LAT_Callback", "free", type, objectAddress);
    airlog("Callback_Destructor", "internal", type, 1);
    if (unlikely(executed == false))
    {
        DumpBuffer buffer(this, sizeof(Callback), &dumpCallbackError);
        dumpCallbackError.AddDebugInfo(buffer, 0);
    }

    if (unlikely(errorCount > 0))
    {
        POS_EVENT_ID eventId = EID(CALLBACK_DESTROY_WITH_ERROR);
        POS_TRACE_WARN(
            eventId,
            "Callback Error : Type : {}, Critical Error : {}",
            static_cast<uint32_t>(type),
            static_cast<uint32_t>(_GetMostCriticalError()));
        DumpBuffer buffer(this, sizeof(Callback), &dumpCallbackError);
        dumpCallbackError.AddDebugInfo(buffer, 0);
    }
    if (DumpSharedModuleInstanceEnable::debugLevelEnable && timeoutChecker != nullptr)
    {
        if (timeoutChecker->CheckTimeout())
        {
            try
            {
                POS_EVENT_ID eventId = EID(CALLBACK_TIMEOUT);
                POS_TRACE_DEBUG_IN_MEMORY(
                    ModuleInDebugLogDump::CALLBACK_TIMEOUT,
                    eventId,
                    "Callback Timeout. Caller : {}",
                    returnAddress);
                POS_TRACE_DEBUG(
                    eventId,
                    "Callback Timeout. Caller : {}",
                    returnAddress);
            }
            catch (...)
            {
            }
        }
        delete timeoutChecker;
        timeoutChecker = nullptr;
    }

    ioTimeoutChecker->DecreasePendingCnt(type, createdTime);
}
// LCOV_EXCL_STOP

void
Callback::SetTimeout(uint64_t timeoutSec)
{
    if (timeoutSec > MAX_TIMEOUT_SEC)
    {
        timeoutSec = MAX_TIMEOUT_SEC;
    }
    if (timeoutSec != 0)
    {
        timeoutNs = timeoutSec * 1000ULL * 1000ULL * 1000ULL;
    }
}

bool
Callback::Execute(void)
{
    airlog("LAT_Callback", "exe_begin", type, objectAddress);
    bool done = _DoSpecificJob();

    if (done)
    {
        _InvokeCallee();
        executed = true;
    }

    airlog("LAT_Callback", "exe_end", type, objectAddress);
    return done;
}

void
Callback::_PreCallExecuteCallee(void)
{
    return;
}

uint32_t
Callback::_GetErrorCount(void)
{
    return errorCount;
}

IOErrorType
Callback::_GetMostCriticalError(void)
{
    return static_cast<IOErrorType>(errorBitMap.FindFirstSetBit(0));
}

void
Callback::InformError(IOErrorType inputIOErrorType)
{
    if (inputIOErrorType != IOErrorType::SUCCESS)
    {
        errorCount += 1;
        errorBitMap.SetBit(static_cast<int>(inputIOErrorType));
    }
}

void
Callback::_InvokeCallee(void)
{
    if (callee == nullptr)
    {
        return;
    }

    bool isOkToCall = callee->_RecordCallerCompletionAndCheckOkToCall(
        errorCount, errorBitMap, weight);
    if (isOkToCall == true)
    {
        _PreCallExecuteCallee();
        bool done = false;
        if (GetEventType() != BackendEvent_UserdataRebuild)
        {
            done = callee->Execute();
            if (likely(done))
            {
                return;
            }
        }

        eventScheduler->EnqueueEvent(callee);
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
    if (unlikely(nullptr == inputCallee))
    {
        POS_EVENT_ID eventId = EID(CALLBACK_INVALID_CALLEE);
        POS_TRACE_ERROR(static_cast<uint32_t>(eventId),
            "Invalid callee for callback");
        return;
    }

    if (unlikely(nullptr != callee))
    {
        POS_EVENT_ID eventId = EID(CALLBACK_INVALID_CALLEE);
        POS_TRACE_ERROR(static_cast<uint32_t>(eventId),
            "Invalid callee for callback");
        return;
    }

    callee = inputCallee;
}

bool
Callback::_RecordCallerCompletionAndCheckOkToCall(uint32_t transferredErrorCount,
    BitMapMutex& inputErrorBitMap, uint32_t transferredWeight)
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
        POS_EVENT_ID eventId = EID(CALLBACK_INVALID_COUNT);
        POS_TRACE_ERROR(static_cast<uint32_t>(eventId),
            "CompletionCount exceeds WaitingCount, event_type: {}, buffer_addr: {},  increasedCompletionCount: {}, localWaitingCount: {}", type, (void*)this, increasedCompletionCount, localWaitingCount);
    }

    return isOkToBeCalled;
}

} // namespace pos
