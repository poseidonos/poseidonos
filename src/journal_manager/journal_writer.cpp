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

#include "src/journal_manager/journal_writer.h"

#include "src/journal_manager/journaling_status.h"
#include "src/journal_manager/log_buffer/log_write_context_factory.h"
#include "src/journal_manager/log_write/log_write_handler.h"

namespace pos
{
JournalWriter::JournalWriter(void)
: logWriteHandler(nullptr),
  logFactory(nullptr),
  status(nullptr)
{
}

JournalWriter::~JournalWriter(void)
{
}

int
JournalWriter::Init(LogWriteHandler* writeHandler, LogWriteContextFactory* factory,
    JournalingStatus* journalingStatus)
{
    logWriteHandler = writeHandler;
    logFactory = factory;

    status = journalingStatus;

    return 0;
}

int
JournalWriter::_CanBeWritten(void)
{
    if (status == nullptr)
    {
        return (-1) * static_cast<int>(POS_EVENT_ID::JOURNAL_INVALID);
    }
    else if (status->Get() == JOURNALING)
    {
        return 0;
    }
    else if (status->Get() == JOURNAL_INVALID)
    {
        return (-1) * static_cast<int>(POS_EVENT_ID::JOURNAL_INVALID);
    }
    else if (status->Get() != JOURNALING)
    {
        return static_cast<int>(POS_EVENT_ID::JOURNAL_NOT_READY);
    }

    return -1;
}

int
JournalWriter::AddBlockMapUpdatedLog(VolumeIoSmartPtr volumeIo,
    MpageList dirty, EventSmartPtr callbackEvent)
{
    int result = _CanBeWritten();
    if (result == 0)
    {
        LogWriteContext* logWriteContext =
            logFactory->CreateBlockMapLogWriteContext(volumeIo, dirty, callbackEvent);
        return logWriteHandler->AddLog(logWriteContext);
    }
    else
    {
        return result;
    }
}

int
JournalWriter::AddStripeMapUpdatedLog(Stripe* stripe, StripeAddr oldAddr,
    MpageList dirty, EventSmartPtr callbackEvent)
{
    int result = _CanBeWritten();
    if (result == 0)
    {
        LogWriteContext* logWriteContext =
            logFactory->CreateStripeMapLogWriteContext(stripe, oldAddr, dirty, callbackEvent);
        return logWriteHandler->AddLog(logWriteContext);
    }
    else
    {
        return result;
    }
}

int
JournalWriter::AddGcStripeFlushedLog(GcStripeMapUpdateList mapUpdates, MapPageList dirty, EventSmartPtr callbackEvent)
{
    int result = _CanBeWritten();
    if (result == 0)
    {
        MapPageList dummyList;
        LogWriteContext* blockWriteDoneLogWriteContext =
            logFactory->CreateGcBlockMapLogWriteContext(mapUpdates, dummyList, nullptr);
        result = logWriteHandler->AddLog(blockWriteDoneLogWriteContext);

        if (result < 0)
        {
            return result;
        }

        LogWriteContext* stripeFlushedLogWriteContext =
            logFactory->CreateGcStripeFlushedLogWriteContext(mapUpdates, dirty, callbackEvent);
        result = logWriteHandler->AddLog(stripeFlushedLogWriteContext);
        if (result < 0)
        {
            return result;
        }
    }

    return result;
}
} // namespace pos
