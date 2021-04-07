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

#pragma once

#include <memory>

namespace ibofos
{
class Event;
using EventSmartPtr = std::shared_ptr<Event>;
/* --------------------------------------------------------------------------*/
/**
 * @Synopsis Object that represent an event
 *           Be generated and deleted at run time
 */
/* --------------------------------------------------------------------------*/
#if defined QOS_ENABLED_BE
enum BackendEvent
{
    BackendEvent_Start = 0,
    BackendEvent_Flush = BackendEvent_Start,
    BackendEvent_GC,
    BackendEvent_UserdataRebuild,
    BackendEvent_MetadataRebuild,
    BackendEvent_FrontendIO,
    BackendEvent_MetaIO,
    BackendEvent_End,
    BackendEvent_Count = BackendEvent_End - BackendEvent_Start,
    BackendEvent_Unknown
};
enum EventPriority
{
    EventPriority_Start = 0,
    EventPriority_Critical = EventPriority_Start,
    EventPriority_High,
    EventPriority_Low,
    EventPriority_End,
    EventPriority_Count = EventPriority_End - EventPriority_Start,
    EventPriority_Unknown
};
#endif

class Event
{
public:
#if defined QOS_ENABLED_BE
    Event(bool isFrontEndEvent = false, BackendEvent event = BackendEvent_Unknown);
    BackendEvent GetEventType(void);
    void SetFrontEnd(bool state);
    void SetEventType(BackendEvent event);
#else
    Event(bool isFrontEndEvent = false);
#endif
    virtual ~Event(void);
    virtual bool Execute(void) = 0;
    bool IsFrontEnd(void);

private:
    bool frontEndEvent;
#if defined QOS_ENABLED_BE
    BackendEvent event;
#endif
};

} // namespace ibofos
