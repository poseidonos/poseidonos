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

#include "src/io_scheduler/dispatcher_policy.h"

#include "src/event_scheduler/backend_policy.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/io_scheduler/io_worker.h"

namespace pos
{
DispatcherPolicyI::DispatcherPolicyI(IODispatcher* dispatcherInput, EventScheduler* schedulerInput)
{
    ioDispatcher = dispatcherInput;
    eventScheduler = schedulerInput;
}

DispatcherPolicyI::~DispatcherPolicyI()
{
    ioDispatcher = nullptr;
    eventScheduler = nullptr;
}

DispatcherPolicyDirect::DispatcherPolicyDirect(IODispatcher* dispatcherInput, EventScheduler* schedulerInput)
: DispatcherPolicyI(dispatcherInput, schedulerInput)
{
}

DispatcherPolicyDirect::~DispatcherPolicyDirect()
{
}

void
DispatcherPolicyDirect::Submit(IOWorker* ioWorker, UbioSmartPtr ubio)
{
    ioWorker->EnqueueUbio(ubio);
}

void
DispatcherPolicyDirect::Process(void)
{
}

DispatcherPolicyQos::DispatcherPolicyQos(IODispatcher* dispatcherInput, EventScheduler* schedulerInput)
: DispatcherPolicyI(dispatcherInput, schedulerInput)
{
}

DispatcherPolicyQos::~DispatcherPolicyQos()
{
}

void
DispatcherPolicyQos::Submit(IOWorker* ioWorker, UbioSmartPtr ubio)
{
    BackendEvent etype = ubio->GetEventType();
    int32_t popCount = eventScheduler->GetAllowedIoCount(etype);
    if (popCount > 0)
    {
        ioWorker->EnqueueUbio(ubio);
    }
    else
    {
        std::unique_lock<std::mutex> uniqueLock(ioDispatcher->ioQueueLock[etype]);
        ioDispatcher->ioQueue[etype].push(std::make_pair(ioWorker, ubio));
    }
}

void
DispatcherPolicyQos::Process(void)
{
    for (unsigned int event = 0; (BackendEvent)event < BackendEvent_Count; event++)
    {
        int32_t popCount = eventScheduler->GetAllowedIoCount(static_cast<BackendEvent>(event));
        ioDispatcher->ioQueueLock[event].lock();
        while (ioDispatcher->ioQueue[event].size() > 0 && popCount > 0)
        {
            std::pair<IOWorker*, UbioSmartPtr> t = ioDispatcher->ioQueue[event].front();
            ioDispatcher->ioQueue[event].pop();
            t.first->EnqueueUbio(t.second);
            popCount -= t.second->GetSize() / Ubio::BYTES_PER_UNIT;
        }
        ioDispatcher->ioQueueLock[event].unlock();
    }
}

} // namespace pos
