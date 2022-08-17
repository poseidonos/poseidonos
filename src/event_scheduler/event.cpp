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

#include "src/event_scheduler/event.h"

#include <air/Air.h>

#include "src/cpu_affinity/affinity_manager.h"
#include "src/include/core_const.h"

namespace pos
{
Event::Event(bool isFrontEndEvent, BackendEvent eventType, AffinityManager* affinityManagerArg)
: frontEndEvent(isFrontEndEvent),
  event(eventType),
  numa(INVALID_NUMA),
  affinityManager(affinityManagerArg)
{
    airlog("Event_Constructor", "internal", static_cast<uint64_t>(event), 1);
    if (nullptr == affinityManager)
    {
        affinityManager = AffinityManagerSingleton::Instance();
    }
    numa = affinityManager->GetNumaIdFromCurrentThread();
}

// LCOV_EXCL_START
Event::~Event(void)
{
    airlog("Event_Destructor", "internal", static_cast<uint64_t>(event), 1);
}
// LCOV_EXCL_STOP

bool
Event::IsFrontEnd(void)
{
    return frontEndEvent;
}

BackendEvent
Event::GetEventType(void)
{
    return event;
}
void
Event::SetFrontEnd(bool state)
{
    frontEndEvent = state;
}

uint32_t
Event::GetNumaId(void)
{
    if (numa != INVALID_NUMA)
    {
        return numa;
    }
    // If it cannot be get numa Id, just set 0.
    return 0;
}

void
Event::SetEventType(BackendEvent eventType)
{
    event = eventType;
}
} // namespace pos
