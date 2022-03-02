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

#include <vector>

#include "src/journal_manager/replay/active_user_stripe_replayer.h"
#include "src/journal_manager/replay/active_wb_stripe_replayer.h"
#include "src/journal_manager/replay/log_delete_checker.h"
#include "src/journal_manager/replay/replay_task.h"
#include "src/journal_manager/replay/replay_log_list.h"

#include "src/mapper/i_vsamap.h"
#include "src/mapper/i_stripemap.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/array_models/interface/i_array_info.h"

namespace pos
{
class IVSAMap;
class IStripeMap;
class IContextManager;
class IContextReplayer;
class ISegmentCtx;
class IArrayInfo;

class LogReplayer;
class ReplayStripe;
class LogHandlerInterface;

class ReplayLogs : public ReplayTask
{
public:
    ReplayLogs(ReplayLogList& logList, LogDeleteChecker* deleteChecker,
        IVSAMap* vsaMap, IStripeMap* stripeMap,
        ISegmentCtx* segmentCtx, IWBStripeAllocator* wbStripeAllocator,
        IContextManager* contextManager,
        IContextReplayer* contextReplayer, IArrayInfo* arrayInfo,
        ReplayProgressReporter* reporter, PendingStripeList& pendingWbStripes);
    virtual ~ReplayLogs(void);

    virtual int Start(void) override;
    virtual ReplayTaskId GetId(void) override;
    virtual int GetWeight(void) override;
    virtual int GetNumSubTasks(void) override;

private:
    int _ReplayFinishedStripes(void);
    int _ReplayUnfinishedStripes(void);

    int _ReplayStripe(ReplayStripe* stripe);

    ReplayStripe* _FindUserStripe(StripeId vsid);
    ReplayStripe* _FindGcStripe(StripeId vsid);
    void _MoveToReplayedStripe(ReplayStripe* stripe);

    ReplayLogList& logList;
    LogDeleteChecker* logDeleteChecker;

    IVSAMap* vsaMap;
    IStripeMap* stripeMap;
    ISegmentCtx* segmentCtx;
    IWBStripeAllocator* wbStripeAllocator;
    IContextManager* contextManager;
    IContextReplayer* contextReplayer;
    IArrayInfo* arrayInfo;

    std::vector<ReplayStripe*> replayingStripeList;
    std::vector<ReplayStripe*> replayedStripeList;

    ActiveWBStripeReplayer* wbStripeReplayer;
    ActiveUserStripeReplayer* userStripeReplayer;
};

} // namespace pos
