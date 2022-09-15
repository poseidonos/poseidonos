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

#pragma once

#include <atomic>
#include <mutex>

#include "src/allocator/i_context_manager.h"
#include "src/journal_manager/checkpoint/checkpoint_status.h"
#include "src/mapper/i_map_flush.h"
#include "src/mapper/include/mpage_info.h"

namespace pos
{
class EventScheduler;

class CheckpointHandler
{
public:
    CheckpointHandler(const int arrayId);
    CheckpointHandler(int numMapsToFlush, int numMapsFlushed, EventSmartPtr callback, const int arrayId);
    virtual ~CheckpointHandler(void) = default;

    virtual void Init(IMapFlush* mapFlushToUse, IContextManager* contextManagerToUse, EventScheduler* eventScheduler);

    virtual int Start(MapList pendingDirtyMaps, EventSmartPtr callback);
    virtual int FlushCompleted(int metaId, int logGroupId);

    virtual CheckpointStatus GetStatus(void);
    virtual void UpdateLogGroupInProgress(int logGroupId);

private:
    void _CheckMapFlushCompleted(void);
    void _TryToComplete(void);
    void _Reset(void);

    void _SetStatus(CheckpointStatus to);

    static const int ALLOCATOR_META_ID = 1000;

    IMapFlush* mapFlush;
    IContextManager* contextManager;
    EventScheduler* scheduler;

    std::atomic<CheckpointStatus> status;
    std::atomic<int> numMapsToFlush;
    std::atomic<int> numMapsFlushed;

    std::atomic<bool> allocatorMetaFlushCompleted;
    std::atomic<bool> mapFlushCompleted;

    std::mutex completionLock;
    EventSmartPtr checkpointCompletionCallback;
    int arrayId;
    int logGroupIdInProgress;
};

} // namespace pos
