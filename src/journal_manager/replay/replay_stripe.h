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

#include "src/include/address_type.h"
#include "src/journal_manager/replay/i_replay_stripe.h"
#include "src/journal_manager/replay/replay_log.h"
#include "src/journal_manager/statistics/stripe_replay_status.h"
namespace pos
{
class IVSAMap;
class IStripeMap;
class IContextReplayer;
class ISegmentCtx;
class IArrayInfo;

class LogHandlerInterface;
class StripeReplayStatus;
class ReplayEventFactory;
class ActiveWBStripeReplayer;
class ActiveUserStripeReplayer;
class ReplayEvent;

using ReplayEventList = std::list<ReplayEvent*>;

class ReplayStripe : public IReplayStripe
{
public:
    ReplayStripe(void) = delete;
    ReplayStripe(StripeId vsid, IVSAMap* vsaMap, IStripeMap* stripeMap,
        IContextReplayer* ctxReplayer,
        ISegmentCtx* segmentCtx, IArrayInfo* arrayInfo,
        ActiveWBStripeReplayer* wbReplayer, ActiveUserStripeReplayer* userReplayer);
    ReplayStripe(IVSAMap* vsaMap, IStripeMap* stripeMap,
        ActiveWBStripeReplayer* wbReplayer, ActiveUserStripeReplayer* userReplayer,
        StripeReplayStatus* status, ReplayEventFactory* factory, ReplayEventList* replayEvents);
    virtual ~ReplayStripe(void);

    virtual void AddLog(ReplayLog replayLog) override;
    virtual int Replay(void) override;

    StripeId GetVsid(void) { return status->GetVsid(); }
    int GetVolumeId(void) { return status->GetVolumeId(); }
    bool IsFlushed(void) { return status->IsFlushed(); }

    void DeleteBlockMapReplayEvents(void);

protected:
    void _CreateSegmentAllocationEvent(void);
    void _CreateStripeAllocationEvent(void);
    void _CreateStripeFlushReplayEvent(void);

    int _ReplayEvents(void);

    StripeReplayStatus* status;
    ReplayEventFactory* replayEventFactory;

    ReplayEventList replayEvents;

    ActiveWBStripeReplayer* wbStripeReplayer;
    ActiveUserStripeReplayer* userStripeReplayer;

    IVSAMap* vsaMap;
    IStripeMap* stripeMap;

    bool replaySegmentInfo;
};

} // namespace pos
