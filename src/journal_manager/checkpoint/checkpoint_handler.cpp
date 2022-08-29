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

#include "checkpoint_handler.h"

#include <thread>

#include "checkpoint_meta_flush_completed.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
// Constructor for unit test mocking
CheckpointHandler::CheckpointHandler(const int arrayId)
: CheckpointHandler(0, 0, nullptr, arrayId)
{
    _Reset();
}

// Constructor for injecting private member values in unit test
CheckpointHandler::CheckpointHandler(int numMapsToFlush, int numMapsFlushed, EventSmartPtr callback, const int arrayId)
: mapFlush(nullptr),
  contextManager(nullptr),
  scheduler(nullptr),
  status(INIT),
  numMapsToFlush(numMapsToFlush),
  numMapsFlushed(numMapsFlushed),
  allocatorMetaFlushCompleted(false),
  mapFlushCompleted(false),
  checkpointCompletionCallback(callback),
  arrayId(arrayId),
  logGroupIdInProgress(INT32_MAX)
{
}

void
CheckpointHandler::Init(IMapFlush* mapFlushToUse, IContextManager* contextManagerToUse, EventScheduler* eventScheduler)
{
    mapFlush = mapFlushToUse;
    contextManager = contextManagerToUse;
    scheduler = eventScheduler;
    logGroupIdInProgress = INT32_MAX;
}

void
CheckpointHandler::UpdateLogGroupInProgress(int logGroupId)
{
    logGroupIdInProgress = logGroupId;
}

int
CheckpointHandler::Start(MapList pendingDirtyMaps, EventSmartPtr callback)
{
    int ret = 0;

    checkpointCompletionCallback = callback;
    _SetStatus(STARTED);

    assert(numMapsToFlush == 0);

    numMapsToFlush = pendingDirtyMaps.size();
    numMapsFlushed = 0;

    if (numMapsToFlush == numMapsFlushed)
    {
        mapFlushCompleted = true;
    }
    else
    {
        int eventId = static_cast<int>(EID(JOURNAL_CHECKPOINT_STARTED));
        POS_TRACE_INFO(eventId, "Checkpoint started with {} maps to flush, arrayId:{}", numMapsToFlush, arrayId);

        for (auto mapId : pendingDirtyMaps)
        {
            eventId = static_cast<int>(EID(JOURNAL_DEBUG));

            EventSmartPtr eventMapFlush(new CheckpointMetaFlushCompleted(this, mapId, logGroupIdInProgress));
            ret = mapFlush->FlushDirtyMpages(mapId, eventMapFlush);
            if (ret != 0)
            {
                // TODO(Cheolho.kang): Add status that can additionally indicate checkpoint status
                POS_TRACE_ERROR(EID(JOURNAL_CHECKPOINT_FAILED),
                    "Failed to start flushing dirty map pages, arrayId:{}", arrayId);
                return ret;
            }
        }
    }

    EventSmartPtr allocMetaFlushCallback(new CheckpointMetaFlushCompleted(this,
        ALLOCATOR_META_ID, logGroupIdInProgress));

    ret = contextManager->FlushContexts(allocMetaFlushCallback, false,
        logGroupIdInProgress);
    if (ret != 0)
    {
        POS_TRACE_ERROR(EID(JOURNAL_CHECKPOINT_FAILED),
            "Failed to start flushing allocator meta pages, arrayId:{}", arrayId);
    }

    std::unique_lock<std::mutex> lock(completionLock);
    if (status != COMPLETED)
    {
        _SetStatus(WAITING_FOR_FLUSH_DONE);
    }

    return ret;
}

void
CheckpointHandler::_CheckMapFlushCompleted(void)
{
    int flushResult = numMapsFlushed.fetch_add(1) + 1;
    if (numMapsToFlush == flushResult)
    {
        mapFlushCompleted = true;
    }
}

int
CheckpointHandler::FlushCompleted(int metaId, int logGroupId)
{
    if (metaId == ALLOCATOR_META_ID)
    {
        POS_TRACE_INFO(EID(JOURNAL_CHECKPOINT_STATUS),
            "Allocator meta flush completed, arrayId:{}", arrayId);
        assert(allocatorMetaFlushCompleted == false);

        allocatorMetaFlushCompleted = true;
        contextManager->ResetFlushedInfo(logGroupId);
    }
    else
    {
        POS_TRACE_INFO(EID(JOURNAL_CHECKPOINT_STATUS),
            "Map {} flush completed, arrayId:{}", metaId, arrayId);
        _CheckMapFlushCompleted();
    }

    _TryToComplete();
    return 0;
}

void
CheckpointHandler::_TryToComplete(void)
{
    POS_TRACE_DEBUG(EID(JOURNAL_CHECKPOINT_STATUS),
        "Try to complete CP, mapCompleted {} allocatorCompleted {}, arrayId:{}",
        mapFlushCompleted, allocatorMetaFlushCompleted, arrayId);

    std::unique_lock<std::mutex> lock(completionLock);
    if ((mapFlushCompleted == true) && (allocatorMetaFlushCompleted == true)
        && (status != COMPLETED))
    {
        // check status to complete checkpoint only once
        _SetStatus(COMPLETED);

        // completed status must not be changed by ohter threads
        assert(status == COMPLETED);

        scheduler->EnqueueEvent(checkpointCompletionCallback);
        POS_TRACE_INFO(EID(JOURNAL_CHECKPOINT_COMPLETED), "Checkpoint completed, arrayId:{}", arrayId);

        _Reset();
    }
}

void
CheckpointHandler::_Reset(void)
{
    mapFlushCompleted = false;
    allocatorMetaFlushCompleted = false;

    numMapsToFlush = 0;
    numMapsFlushed = 0;

    checkpointCompletionCallback = nullptr;
}

void
CheckpointHandler::_SetStatus(CheckpointStatus to)
{
    POS_TRACE_DEBUG(EID(JOURNAL_CHECKPOINT_STATUS),
        "Checkpoint status changed from {} to {}, arrayId:{}", status, to, arrayId);

    status = to;
}

CheckpointStatus
CheckpointHandler::GetStatus(void)
{
    return status;
}
} // namespace pos
