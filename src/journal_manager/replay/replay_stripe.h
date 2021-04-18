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

#include <list>

#include "../log/log_handler.h"
#include "replay_event.h"
#include "src/journal_manager/statistics/stripe_info.h"
#include "src/journal_manager/statistics/stripe_replay_status.h"

#include "src/allocator/context_manager/active_stripe_index_info.h"
#include "src/include/address_type.h"

#include "src/mapper/i_vsamap.h"
#include "src/mapper/i_stripemap.h"
#include "src/allocator/i_wbstripe_ctx.h"
#include "src/allocator/i_segment_ctx.h"
#include "src/allocator/i_block_allocator.h"
#include "src/array_models/interface/i_array_info.h"

namespace pos
{
class ReplayStripe;
class ReplayEvent;
class ActiveWBStripeReplayer;
class ActiveUserStripeReplayer;

class ReplayStripe
{
public:
    ReplayStripe(void) = delete;
    ReplayStripe(StripeId vsid, IVSAMap* vsaMap, IStripeMap* stripeMap,
        IWBStripeCtx* wbStripeCtx, ISegmentCtx* segmentCtx,
        IBlockAllocator* blockAllocator, IArrayInfo* arrayInfo,
        ActiveWBStripeReplayer* wbReplayer, ActiveUserStripeReplayer* userReplayer);
    virtual ~ReplayStripe(void);

    void AddLog(LogHandlerInterface* log);
    int Replay(void);

    // TODO(huijeong.kim): remove this function
    StripeId GetVsid(void) { return status->GetVsid(); }
    int GetVolumeId(void) { return status->GetVolumeId(); }
    bool IsFlushed(void) { return status->IsFlushed(); }

    void DeleteBlockMapReplayEvents(void);

private:
    void _CreateBlockWriteReplayEvent(BlockWriteDoneLog dat);

    void _CreateStripeEvents(void);
    void _CreateStripeAllocationEvent(void);
    void _CreateStripeFlushReplayEvent(void);

    void _UpdateStripeInfo(BlockWriteDoneLog log);
    void _UpdateStripeInfo(StripeMapUpdatedLog log);

    int _UpdateActiveStripeInfo(void);
    int _ReplayEvents(void);

    StripeReplayStatus* status;

    // TODO (huijeong.kim) to be moved out, to ReplayLogs
    std::list<ReplayEvent*> replayEvents;

    ActiveWBStripeReplayer* wbStripeReplayer;
    ActiveUserStripeReplayer* userStripeReplayer;

    IVSAMap* vsaMap;
    IStripeMap* stripeMap;
    IWBStripeCtx* wbStripeCtx;
    ISegmentCtx* segmentCtx;
    IBlockAllocator* blockAllocator;
    IArrayInfo* arrayInfo;
};

} // namespace pos
