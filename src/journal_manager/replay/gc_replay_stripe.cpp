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

#include "src/journal_manager/replay/gc_replay_stripe.h"

#include "src/include/pos_event_id.h"
#include "src/journal_manager/log/gc_stripe_flushed_log_handler.h"
#include "src/journal_manager/replay/active_user_stripe_replayer.h"
#include "src/journal_manager/replay/replay_event_factory.h"
#include "src/logger/logger.h"
#include "src/mapper/i_stripemap.h"

namespace pos
{
GcReplayStripe::GcReplayStripe(StripeId vsid, IVSAMap* vsaMap, IStripeMap* stripeMap,
    IContextReplayer* contextReplayer,
    IBlockAllocator* blockAllocator, IArrayInfo* arrayInfo,
    ActiveWBStripeReplayer* wbReplayer, ActiveUserStripeReplayer* userReplayer)
: ReplayStripe(vsid, vsaMap, stripeMap, contextReplayer, blockAllocator,
      arrayInfo, wbReplayer, userReplayer)
{
}

void
GcReplayStripe::AddLog(ReplayLog replayLog)
{
    ReplayStripe::AddLog(replayLog);
    _AddLog(replayLog.log);
}

void
GcReplayStripe::_AddLog(LogHandlerInterface* log)
{
    assert(log->GetType() == LogType::GC_STRIPE_FLUSHED);

    GcStripeFlushedLogHandler* gcLogHandler = dynamic_cast<GcStripeFlushedLogHandler*>(log);

    // This should be true
    // TODO (huijeong.kim) pass the exact type instead of dynamic casting here
    assert(gcLogHandler != nullptr);

    GcStripeFlushedLog* gcLog = gcLogHandler->GetGcStripeFlushedLog();
    GcBlockMapUpdate* blockLogs = gcLogHandler->GetGcBlockMapUpdateLogs();

    status->GcBlockLogFound(blockLogs, gcLog->numBlockMaps);
    for (int blkCount = 0; blkCount < gcLog->numBlockMaps; blkCount++)
    {
        ReplayEvent* blockWriteEvent =
            replayEventFactory->CreateBlockWriteReplayEvent(gcLog->volId,
                blockLogs[blkCount].rba, blockLogs[blkCount].vsa, 1, replaySegmentInfo);
        replayEvents.push_back(blockWriteEvent);
    }

    status->GcStripeLogFound(*gcLog);
    _CreateStripeFlushReplayEvent();

    _CreateSegmentAllocationEvent();
}

int
GcReplayStripe::Replay(void)
{
    int result = ReplayStripe::Replay();
    if (result == 0)
    {
        userStripeReplayer->Update(status->GetUserLsid());
    }
    return result;
}

} // namespace pos
