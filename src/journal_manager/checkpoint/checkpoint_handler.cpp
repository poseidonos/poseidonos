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
#include "src/allocator/allocator.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
CheckpointHandler::CheckpointHandler(CheckpointObserver* observer)
: mapper(MapperSingleton::Instance()),
  allocator(AllocatorSingleton::Instance()),
  obs(observer),
  status(INIT),
  numMapsToFlush(0),
  numMapsFlushed(0)
{
    _Reset();
}

void
CheckpointHandler::SetMapperToUse(Mapper* mapperToUse)
{
    mapper = mapperToUse;
}

void
CheckpointHandler::SetAllocatorToUse(Allocator* allocatorToUse)
{
    allocator = allocatorToUse;
}

int
CheckpointHandler::Start(MapPageList pendingDirtyPages)
{
    int ret = 0;

    _SetStatus(STARTED);

    assert(numMapsToFlush == 0);
    assert(pendingDirtyPages.size() != 0);

    numMapsToFlush = pendingDirtyPages.size(); // # of flushing pages
    numMapsFlushed = 0;

    IBOF_TRACE_INFO(EID(JOURNAL_CHECKPOINT_STARTED),
        "Checkpoint started with {} maps to flush", numMapsToFlush);

    for (auto mapIt = pendingDirtyPages.begin();
         mapIt != pendingDirtyPages.end(); mapIt++)
    {
        EventSmartPtr mapFlushCallback(new CheckpointMetaFlushCompleted(this, mapIt->first));
        ret = mapper->StartDirtyPageFlush(mapIt->first,
            mapIt->second, mapFlushCallback);

        if (ret != 0)
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_CHECKPOINT_FAILED,
                "Failed to start flushing dirty map pages");
            return ret;
        }
    }

    EventSmartPtr allocMetaFlushCallback(new CheckpointMetaFlushCompleted(this, ALLOCATOR_META_ID));
    ret = allocator->FlushMetadata(allocMetaFlushCallback);
    if (ret != 0)
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::JOURNAL_CHECKPOINT_FAILED,
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
    std::unique_lock<std::mutex> lock(mapFlushCountLock);
    numMapsFlushed++;

    return (numMapsFlushed == numMapsToFlush);
}

int
CheckpointHandler::FlushCompleted(int metaId)
{
    if (metaId == ALLOCATOR_META_ID)
    {
        IBOF_TRACE_INFO(EID(JOURNAL_CHECKPOINT_STATUS),
            "Allocator meta flush completed");
        assert(allocatorMetaFlushCompleted == false);
        allocatorMetaFlushCompleted = true;
    }
    else
    {
        IBOF_TRACE_INFO(EID(JOURNAL_CHECKPOINT_STATUS),
            "Map {} flush completed", metaId);
        mapFlushCompleted = _IncreaseNumMapsFlushed();
    }

    _TryToComplete();
    return 0;
}

void
CheckpointHandler::_TryToComplete(void)
{
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_CHECKPOINT_STATUS,
        "Try to complete CP, mapCompleted {} allocatorCompleted {}",
        mapFlushCompleted, allocatorMetaFlushCompleted);

    std::unique_lock<std::mutex> lock(completionLock);
    if ((mapFlushCompleted == true) && (allocatorMetaFlushCompleted == true) && (status != COMPLETED))
    {
        // check status to complete checkpoint only once
        _SetStatus(COMPLETED);
        obs->CheckpointCompleted();
        IBOF_TRACE_INFO(EID(JOURNAL_CHECKPOINT_COMPLETED), "Checkpoint completed");

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
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_CHECKPOINT_STATUS,
        "Checkpoint status changed from {} to {}", status, to);

    status = to;
}

} // namespace ibofos
