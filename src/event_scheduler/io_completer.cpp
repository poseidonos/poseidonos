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

#include "src/event_scheduler/io_completer.h"

#include <stdexcept>

#include "src/bio/ubio.h"
#include "src/event_scheduler/callback.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/include/core_const.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/event_scheduler/event_factory.h"

namespace pos
{

EventFactory* IoCompleter::recoveryEventFactory = nullptr;

IoCompleter::IoCompleter(UbioSmartPtr ubio,
    EventScheduler* eventSchedulerArg,
    EventFrameworkApi* eventFrameworkApiArg)
: ubio(ubio),
  eventScheduler(eventSchedulerArg),
  eventFrameworkApi(eventFrameworkApiArg)
{
    if (nullptr == eventScheduler)
    {
        eventScheduler = EventSchedulerSingleton::Instance();
    }
    if (nullptr == eventFrameworkApi)
    {
        eventFrameworkApi = EventFrameworkApiSingleton::Instance();
    }
}

void
IoCompleter::CompleteOriginUbio(void)
{
    UbioSmartPtr origin = ubio->GetOriginUbio();
    if (origin != nullptr)
    {
        CallbackSmartPtr callee = origin->GetCallback();
        origin->ClearCallback();
        CallbackSmartPtr callback = ubio->GetCallback();
        if (callback != nullptr)
        {
            callback->SetCallee(callee);
            origin->Complete(IOErrorType::SUCCESS);
        }
        ubio->ClearOrigin();
    }
}

void
IoCompleter::CompleteUbio(IOErrorType errorType, bool executeCallback)
{
    bool ioFailed = (errorType != IOErrorType::SUCCESS);
    bool recoveryAllowed = ubio->CheckRecoveryAllowed();
    if (ioFailed && recoveryAllowed)
    {
        _SubmitRecovery(ubio);
        return;
    }

    CompleteUbioWithoutRecovery(errorType, executeCallback);
}

void
IoCompleter::CompleteUbioWithoutRecovery(IOErrorType errorType, bool executeCallback)
{
    ubio->Complete(errorType);

    bool asyncMode = (false == ubio->IsSyncMode());

    if (executeCallback && asyncMode)
    {
        CallbackSmartPtr callback = ubio->GetCallback();
        ubio->ClearCallback();
        callback->InformError(errorType);

        uint32_t originCore = ubio->GetOriginCore();

        bool done = false;
        if (originCore != INVALID_CORE)
        {
            bool keepCurrentReactor = eventFrameworkApi->IsSameReactorNow(originCore);
            if (keepCurrentReactor)
            {
                done = callback->Execute();
            }
            if (false == done)
            {
                done = SpdkEventScheduler::SendSpdkEvent(originCore, callback);
            }
        }

        if (false == done)
        {
            eventScheduler->EnqueueEvent(callback);
        }
    }
}

void
IoCompleter::_SubmitRecovery(UbioSmartPtr ubio)
{
    ubio->SetError(IOErrorType::DEVICE_ERROR);
    if (recoveryEventFactory == nullptr)
    {
        throw std::runtime_error("recoveryEventFactory is nullptr");
    }
    EventSmartPtr failure = recoveryEventFactory->Create(ubio);
    eventScheduler->EnqueueEvent(failure);
}

void
IoCompleter::RegisterRecoveryEventFactory(EventFactory* recoveryEventFactory)
{
    IoCompleter::recoveryEventFactory = recoveryEventFactory;
}

} // namespace pos
