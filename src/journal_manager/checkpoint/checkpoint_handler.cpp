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

#include "checkpoint_handler.h"

#include <thread>

#include "checkpoint_meta_flush_completed.h"
#include "checkpoint_observer.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
// Constructor for injecting private member values in product code
CheckpointHandler::CheckpointHandler(CheckpointObserver* observer, int numMapsToFlush, int numMapsFlushed)
: mapFlush(nullptr),
  contextManager(nullptr),
  obs(observer),
  status(INIT),
  numMapsToFlush(numMapsToFlush),
  numMapsFlushed(numMapsFlushed),
  allocatorMetaFlushCompleted(false),
  mapFlushCompleted(false)
{
}

CheckpointHandler::CheckpointHandler(CheckpointObserver* observer)
: CheckpointHandler(observer, 0, 0)
{
    _Reset();
}

void
CheckpointHandler::Init(IMapFlush* mapFlushToUse, IContextManager* contextManagerToUse)
{
    mapFlush = mapFlushToUse;
    contextManager = contextManagerToUse;
}

int
CheckpointHandler::Start(MapPageList pendingDirtyPages)
{
    int ret = 0;

    _SetStatus(STARTED);

    assert(numMapsToFlush == 0);
    assert(pendingDirtyPages.size() != 0);

    numMapsToFlush = pendingDirtyPages.size();
    numMapsFlushed = 0;

    int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_CHECKPOINT_STARTED);
    POS_TRACE_INFO(eventId, "Checkpoint started with {} maps to flush", numMapsToFlush);

    for (auto mapIt = pendingDirtyPages.begin();
        mapIt != pendingDirtyPages.end(); mapIt++)
    {
        eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_DEBUG);
        POS_TRACE_DEBUG(eventId, "Request to flush map {}, {} pages",
            mapIt->first, (mapIt->second).size());

        EventSmartPtr mapFlushCallback(new CheckpointMetaFlushCompleted(this, mapIt->first));
        ret = mapFlush->FlushDirtyMpages(mapIt->first, mapFlushCallback, mapIt->second);
        if (ret != 0)
        {
            // TODO(Cheolho.kang): Add status that can additionally indicate checkpoint status
            POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_CHECKPOINT_FAILED,
                "Failed to start flushing dirty map pages");
            return ret;
        }
    }

    EventSmartPtr allocMetaFlushCallback(new CheckpointMetaFlushCompleted(this,
        ALLOCATOR_META_ID));
    ret = contextManager->FlushContextsAsync(allocMetaFlushCallback);
    if (ret != 0)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_CHECKPOINT_FAILED,
            "Failed to start flushing allocator meta pages");
    }

    if (status != COMPLETED)
    {
        _SetStatus(WAITING_FOR_FLUSH_DONE);
    }

    return ret;
}

bool
CheckpointHandler::_IncreaseNumMapsFlushed(void)
{
    numMapsFlushed++;
    return (numMapsFlushed.load(memory_order_seq_cst)
        == numMapsToFlush.load(memory_order_seq_cst));
}

int
CheckpointHandler::FlushCompleted(int metaId)
{
    if (metaId == ALLOCATOR_META_ID)
    {
        POS_TRACE_INFO(EID(JOURNAL_CHECKPOINT_STATUS),
            "Allocator meta flush completed");
        assert(allocatorMetaFlushCompleted == false);
        allocatorMetaFlushCompleted = true;
    }
    else
    {
        POS_TRACE_INFO(EID(JOURNAL_CHECKPOINT_STATUS),
            "Map {} flush completed", metaId);
        mapFlushCompleted = _IncreaseNumMapsFlushed();
    }

    _TryToComplete();
    return 0;
}

void
CheckpointHandler::_TryToComplete(void)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::JOURNAL_CHECKPOINT_STATUS,
        "Try to complete CP, mapCompleted {} allocatorCompleted {}",
        mapFlushCompleted, allocatorMetaFlushCompleted);

    std::unique_lock<std::mutex> lock(completionLock);
    if ((mapFlushCompleted == true) && (allocatorMetaFlushCompleted == true)
        && (status != COMPLETED))
    {
        // check status to complete checkpoint only once
        _SetStatus(COMPLETED);
        obs->CheckpointCompleted();
        POS_TRACE_INFO(EID(JOURNAL_CHECKPOINT_COMPLETED), "Checkpoint completed");

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
}

void
CheckpointHandler::_SetStatus(CheckpointStatus to)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::JOURNAL_CHECKPOINT_STATUS,
        "Checkpoint status changed from {} to {}", status, to);

    status = to;
}

CheckpointStatus
CheckpointHandler::GetStatus(void)
{
    return status;
}

} // namespace pos
