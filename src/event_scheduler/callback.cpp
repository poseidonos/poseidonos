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

#include "Air.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/dump/dump_shared_ptr.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/lib/system_timeout_checker.h"
#include "src/logger/logger.h"
#include "src/dump/dump_module.h"
#include "src/dump/dump_module.hpp"
#include "src/dump/dump_buffer.h"

namespace pos
{

const uint32_t Callback::CALLER_FRAME = 0;
// 30 sec, default timeout
const uint64_t Callback::DEFAULT_TIMEOUT_NS = 30ULL * 1000 * 1000 * 1000;
static const char* DUMP_NAME = "Callback_Error";
static const bool DEFAULT_DUMP_ON = true;

DumpModule<DumpBuffer> dumpCallbackError(DUMP_NAME,
    DumpModule<DumpBuffer>::MAX_ENTRIES_FOR_CALLBACK_ERROR,
    DEFAULT_DUMP_ON);

Callback::Callback(bool isFrontEnd, CallbackType type, uint32_t weight)
: Event(isFrontEnd),
  errorCount(0),
  errorBitMap((int)IOErrorType::CALLBACK_ERROR_MAX_COUNT),
  completionCount(0),
  waitingCount(1),
  weight(weight),
  callee(nullptr),
  timeoutChecker(nullptr),
  returnAddress(nullptr),
  executed(false),
  type(type)
{
    objectAddress = reinterpret_cast<uint64_t>(this);
    airlog("LAT_Callback", "AIR_NEW", type, objectAddress);
    if (DumpSharedModuleInstanceEnable::debugLevelEnable)
    {
        returnAddress = __builtin_return_address(Callback::CALLER_FRAME);
        timeoutChecker = new SystemTimeoutChecker;
        timeoutChecker->SetTimeout(DEFAULT_TIMEOUT_NS);
    }
}

Callback::~Callback(void)
{
    airlog("LAT_Callback", "AIR_FREE", type, objectAddress);
    if (unlikely(executed == false))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::CALLBACK_DESTROY_WITHOUT_EXECUTED;
        POS_TRACE_WARN(
            eventId,
            PosEventId::GetString(eventId),
            static_cast<uint32_t>(GetEventType()));
        DumpBuffer buffer(this, sizeof(Callback), &dumpCallbackError);
        dumpCallbackError.AddDump(buffer, 0);
    }

    if (unlikely(errorCount > 0))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::CALLBACK_DESTROY_WITH_ERROR;
        POS_TRACE_WARN(
            eventId,
            PosEventId::GetString(eventId),
            static_cast<uint32_t>(GetEventType()),
            static_cast<uint32_t>(_GetMostCriticalError()));
        DumpBuffer buffer(this, sizeof(Callback), &dumpCallbackError);
        dumpCallbackError.AddDump(buffer, 0);
    }
    if (DumpSharedModuleInstanceEnable::debugLevelEnable && timeoutChecker != nullptr)
    {
        if (timeoutChecker->CheckTimeout())
        {
            try
            {
                POS_EVENT_ID eventId = POS_EVENT_ID::CALLBACK_TIMEOUT;
                POS_TRACE_DEBUG_IN_MEMORY(
                    ModuleInDebugLogDump::CALLBACK_TIMEOUT,
                    eventId,
                    PosEventId::GetString(eventId),
                    returnAddress);
                POS_TRACE_DEBUG(
                    eventId,
                    PosEventId::GetString(eventId),
                    returnAddress);
            }
            catch (...)
            {
            }
        }
        delete timeoutChecker;
    }
}

bool
Callback::Execute(void)
{
    airlog("LAT_Callback", "AIR_EXE_BEGIN", type, objectAddress);
    bool done = _DoSpecificJob();

    if (done)
    {
        _InvokeCallee();
        executed = true;
    }

    airlog("LAT_Callback", "AIR_EXE_END", type, objectAddress);
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
        bool done = callee->Execute();

        if (likely(done))
        {
            return;
        }

        EventSchedulerSingleton::Instance()->EnqueueEvent(callee);
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
        POS_EVENT_ID eventId = POS_EVENT_ID::CALLBACK_INVALID_CALLEE;
        POS_TRACE_ERROR(static_cast<uint32_t>(eventId),
            PosEventId::GetString(eventId));
        return;
    }

    if (unlikely(_IsCalleeSet()))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::CALLBACK_INVALID_CALLEE;
        POS_TRACE_ERROR(static_cast<uint32_t>(eventId),
            PosEventId::GetString(eventId));
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
        POS_EVENT_ID eventId = POS_EVENT_ID::CALLBACK_INVALID_COUNT;
        POS_TRACE_ERROR(static_cast<uint32_t>(eventId),
            PosEventId::GetString(eventId));
    }

    return isOkToBeCalled;
}

} // namespace pos
