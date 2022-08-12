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

#include <list>

#include "src/journal_manager/replay/log_delete_checker.h"
#include "src/journal_manager/replay/pending_stripe.h"
#include "src/journal_manager/replay/replay_log_list.h"
#include "src/journal_manager/replay/replay_progress_reporter.h"
#include "src/journal_manager/replay/replay_state_changer.h"
#include "src/journal_manager/replay/replay_task.h"

namespace pos
{
class Allocator;
class Mapper;

class JournalConfiguration;
class IJournalLogBuffer;
class LogReplayer;

class IVSAMap;
class IStripeMap;
class IMapFlush;
class ISegmentCtx;
class IWBStripeAllocator;
class IContextManager;
class IContextReplayer;
class IArrayInfo;
class IVolumeInfoManager;

class ReplayHandler
{
public:
    ReplayHandler(void) = default;
    explicit ReplayHandler(IStateControl* iState);
    virtual ~ReplayHandler(void);

    virtual void Init(JournalConfiguration* journalConfiguration,
        IJournalLogBuffer* journalLogBuffer, IVSAMap* vsaMap, IStripeMap* stripeMap,
        IMapFlush* mapFlush, ISegmentCtx* segmentCtx,
        IWBStripeAllocator* wbStripeAllocator, IContextManager* contextManager,
        IContextReplayer* contextReplayer, IArrayInfo* arrayInfo, IVolumeInfoManager* volumeManager);
    virtual void Dispose(void);

    virtual int Start(void);

private:
    void _InitializeTaskList(IVSAMap* vsaMap, IStripeMap* stripeMap,
        IMapFlush* mapFlush, ISegmentCtx* segmentCtx,
        IWBStripeAllocator* wbStripeAllocator, IContextManager* contextManager,
        IContextReplayer* contextReplayer, IArrayInfo* arrayInfo, IVolumeInfoManager* volumeManager);
    void _AddTask(ReplayTask* task);
    int _ExecuteReplayTasks(void);

    ReplayStateChanger replayState;

    ReplayLogList logList;
    PendingStripeList pendingWbStripes;

    LogDeleteChecker* logDeleteChecker;

    JournalConfiguration* config;
    IJournalLogBuffer* logBuffer;

    ReplayProgressReporter* reporter;
    std::list<ReplayTask*> taskList;
};

} // namespace pos
