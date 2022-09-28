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

#include "src/journal_manager/journal_writer.h"

#include "src/event_scheduler/event_scheduler.h"
#include "src/journal_manager/journaling_status.h"
#include "src/journal_manager/log_buffer/log_write_context_factory.h"
#include "src/journal_manager/log_write/gc_log_write_completed.h"
#include "src/journal_manager/log_write/journal_event_factory.h"
#include "src/journal_manager/log_write/log_write_handler.h"

namespace pos
{
JournalWriter::JournalWriter(void)
: logWriteHandler(nullptr),
  logFactory(nullptr),
  eventFactory(nullptr),
  status(nullptr),
  eventScheduler(nullptr)
{
}

JournalWriter::~JournalWriter(void)
{
}

int
JournalWriter::Init(LogWriteHandler* writeHandler, LogWriteContextFactory* logWriteEventFactory,
    JournalEventFactory* journalEventFactory, JournalingStatus* journalingStatus, EventScheduler* scheduler)
{
    logWriteHandler = writeHandler;
    logFactory = logWriteEventFactory;
    eventFactory = journalEventFactory;

    status = journalingStatus;

    eventScheduler = scheduler;

    return 0;
}

int
JournalWriter::_CanBeWritten(void)
{
    int eventId = static_cast<int>(EID(JOURNAL_READY));
    if (status == nullptr)
    {
        eventId = static_cast<int>(EID(JOURNAL_INVALID));
        POS_TRACE_ERROR(eventId, "status is nullptr");
        return (-1) * eventId;
    }

    int returnCode = 0;
    JournalManagerStatus currentStatus = status->Get();
    if (currentStatus == JOURNALING)
    {
        returnCode = 0;
    }
    else if (currentStatus == JOURNAL_INVALID)
    {
        eventId = static_cast<int>(EID(JOURNAL_INVALID));
        returnCode = (-1) * eventId;
    }
    else if (currentStatus != JOURNALING)
    {
        eventId = static_cast<int>(EID(JOURNAL_NOT_READY));
        returnCode = eventId;
    }

    POS_TRACE_INFO_CONDITIONALLY(&changeLogger, eventId, currentStatus, "status changed");

    return returnCode;
}

int
JournalWriter::AddBlockMapUpdatedLog(VolumeIoSmartPtr volumeIo, EventSmartPtr callbackEvent)
{
    int result = _CanBeWritten();
    if (result == 0)
    {
        LogWriteContext* logWriteContext =
            logFactory->CreateBlockMapLogWriteContext(volumeIo, callbackEvent);
        result = logWriteHandler->AddLog(logWriteContext);
        if (result != 0)
        {
            delete logWriteContext;
        }

        return result;
    }
    else
    {
        return result;
    }
}

int
JournalWriter::AddStripeMapUpdatedLog(Stripe* stripe, StripeAddr oldAddr, EventSmartPtr callbackEvent)
{
    int result = _CanBeWritten();
    if (result == 0)
    {
        LogWriteContext* logWriteContext =
            logFactory->CreateStripeMapLogWriteContext(stripe, oldAddr, callbackEvent);
        result = logWriteHandler->AddLog(logWriteContext);
        if (result != 0)
        {
            delete logWriteContext;
        }

        return result;
    }
    else
    {
        return result;
    }
}

int
JournalWriter::AddGcStripeFlushedLog(GcStripeMapUpdateList mapUpdates, EventSmartPtr callbackEvent)
{
    int result = _CanBeWritten();
    if (result == 0)
    {
        return _AddGcLogs(mapUpdates, callbackEvent);
    }
    else
    {
        return result;
    }
}

int
JournalWriter::_AddGcLogs(GcStripeMapUpdateList mapUpdates, EventSmartPtr callbackEvent)
{
    LogWriteContext* stripeFlushedLogWriteContext =
        logFactory->CreateGcStripeFlushedLogWriteContext(mapUpdates, callbackEvent);

    EventSmartPtr gcStripeLogWriteRequest = eventFactory->CreateLogWriteEvent(stripeFlushedLogWriteContext);
    EventSmartPtr gcLogWriteCompleted = eventFactory->CreateGcBlockLogWriteCompletedEvent(gcStripeLogWriteRequest);

    auto blockContexts = logFactory->CreateGcBlockMapLogWriteContexts(mapUpdates, gcLogWriteCompleted);

    GcLogWriteCompleted* gcLogCallback = dynamic_cast<GcLogWriteCompleted*>(gcLogWriteCompleted.get());
    assert(gcLogCallback != nullptr);

    uint64_t numLogs = (blockContexts.size() == 0) ? 1 : blockContexts.size();
    gcLogCallback->SetNumLogs(numLogs);

    EventSmartPtr gcJournalWrite = eventFactory->CreateGcLogWriteEvent(blockContexts, gcLogWriteCompleted);
    eventScheduler->EnqueueEvent(gcJournalWrite);

    return EID(SUCCESS);
}
} // namespace pos
