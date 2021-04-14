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

#include <vector>

#include "../log/log_handler.h"
#include "active_user_stripe_replayer.h"
#include "active_wb_stripe_replayer.h"
#include "replay_stripe.h"
#include "replay_task.h"

#include "src/mapper/i_vsamap.h"
#include "src/mapper/i_stripemap.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator/i_wbstripe_ctx.h"
#include "src/allocator/i_segment_ctx.h"
#include "src/array_models/interface/i_array_info.h"

namespace pos
{
class Mapper;
class Allocator;

class LogReplayer;

class ReplayLogs : public ReplayTask
{
public:
    ReplayLogs(LogList& logList, IVSAMap* vsaMap, IStripeMap* stripeMap,
        IBlockAllocator* blockAllocator, IWBStripeAllocator* wbStripeAllocator,
        IWBStripeCtx* wbStripeCtx, ISegmentCtx* segmentCtx, IArrayInfo* arrayInfo,
        ReplayProgressReporter* reporter, PendingStripeList& pendingWbStripes);
    virtual ~ReplayLogs(void);

    virtual int Start(void) override;
    virtual ReplayTaskId GetId(void) override;
    virtual int GetWeight(void) override;
    virtual int GetNumSubTasks(void) override;

private:
    void _CreateReplayStripe(void);
    void _DeleteVolumeLogs(LogHandlerInterface* log);
    ReplayStripe* _FindStripe(StripeId vsid);
    int _Replay(void);

    LogList& logList;

    IVSAMap* vsaMap;
    IStripeMap* stripeMap;
    IBlockAllocator* blockAllocator;
    IWBStripeAllocator* wbStripeAllocator;
    IWBStripeCtx* wbStripeCtx;
    ISegmentCtx* segmentCtx;
    IArrayInfo* arrayInfo;

    std::vector<ReplayStripe*> replayStripeList;
    ActiveWBStripeReplayer* wbStripeReplayer;
    ActiveUserStripeReplayer* userStripeReplayer;
};

} // namespace pos
