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

#include "flush_metadata.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
FlushMetadata::FlushMetadata(IMapFlush* mapFlush, IContextManager* ctxManager,
    ReplayProgressReporter* reporter)
: ReplayTask(reporter),
  mapFlush(mapFlush),
  contextManager(ctxManager)
{
}

FlushMetadata::~FlushMetadata(void)
{
}

int
FlushMetadata::GetNumSubTasks(void)
{
    return 2;
}

int
FlushMetadata::Start(void)
{
    int eventId = static_cast<int>(EID(JOURNAL_REPLAY_STATUS));
    POS_TRACE_DEBUG(eventId, "[ReplayTask] Start flushing replayed metadata");

    int result = mapFlush->StoreAll();
    reporter->SubTaskCompleted(GetId(), 1);

    if (result == 0)
    {
        result = contextManager->FlushContexts(nullptr, true);
        reporter->SubTaskCompleted(GetId(), 1);
    }

    return result;
}

ReplayTaskId
FlushMetadata::GetId(void)
{
    return ReplayTaskId::FLUSH_METADATA;
}

int
FlushMetadata::GetWeight(void)
{
    return 20;
}

} // namespace pos
