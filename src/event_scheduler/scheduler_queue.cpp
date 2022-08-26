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

#include "src/event_scheduler/scheduler_queue.h"

#include <assert.h>

#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/qos/qos_manager.h"
#include "src/spdk_wrapper/event_framework_api.h"
namespace pos
{
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Constructor 
 */
/* --------------------------------------------------------------------------*/
SchedulerQueue::SchedulerQueue(QosManager* qosManager_)
: qosManager(qosManager_)
{
    if (nullptr == qosManager)
    {
        qosManager = QosManagerSingleton::Instance();
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Destructor
 */
/* --------------------------------------------------------------------------*/
SchedulerQueue::~SchedulerQueue(void)
{
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Pop oldest entry of queue
 *           Lock can be optimized with Boost library
 *
 * @Returns  If queue is empty, nullptr
 *           otherwise, oldest entry of queue
 */
/* --------------------------------------------------------------------------*/
EventSmartPtr
SchedulerQueue::DequeueEvent(void)
{
    EventSmartPtr event = nullptr;
    if (false == queue.empty())
    {
        event = queue.front();
        queue.pop();
    }
    return event;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Push new entry to queue
 *           Lock can be optimized with Boost library
 *
 * @Param    input
 */
/* --------------------------------------------------------------------------*/
void
SchedulerQueue::EnqueueEvent(EventSmartPtr input)
{
    if (unlikely(nullptr == input))
    {
        POS_TRACE_WARN(EID(EVTQ_INVALID_EVENT), "");
        return;
    }

    queue.push(input);

    if (false == input->IsFrontEnd())
    {
        qosManager->IncreasePendingBackendEvents(input->GetEventType());
        qosManager->LogEvent(input->GetEventType());
    }
}
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Push new entry to queue
 *           Lock can be optimized with Boost library
 *
 * @Param    input
 */
/* --------------------------------------------------------------------------*/
uint32_t
SchedulerQueue::GetQueueSize(void)
{
    return queue.size();
}
} // namespace pos
