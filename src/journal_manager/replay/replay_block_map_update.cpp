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

#include "src/journal_manager/replay/replay_block_map_update.h"

#include "src/allocator/i_block_allocator.h"
#include "src/journal_manager/replay/active_wb_stripe_replayer.h"
#include "src/journal_manager/statistics/stripe_replay_status.h"

namespace pos
{
ReplayBlockMapUpdate::ReplayBlockMapUpdate(IVSAMap* vsaMap, ISegmentCtx* segmentCtx,
    StripeReplayStatus* status, ActiveWBStripeReplayer* wbReplayer, int volId, BlkAddr startRba, VirtualBlkAddr startVsa,
    uint64_t numBlks, bool replaySegmentInfo)
: ReplayEvent(status),
  vsaMap(vsaMap),
  segmentCtx(segmentCtx),
  volId(volId),
  startRba(startRba),
  startVsa(startVsa),
  numBlks(numBlks),
  replaySegmentInfo(replaySegmentInfo),
  wbStripeReplayer(wbReplayer)
{
}

ReplayBlockMapUpdate::~ReplayBlockMapUpdate(void)
{
}

void
ReplayBlockMapUpdate::_ReadBlockMap(void)
{
    readMap.resize(numBlks);

    for (uint32_t offset = 0; offset < numBlks; offset++)
    {
        readMap[offset] = vsaMap->GetVSAWithSyncOpen(volId, startRba + offset);
    }
}

int
ReplayBlockMapUpdate::Replay(void)
{
    _ReadBlockMap();

    int result = 0;
    for (uint32_t offset = 0; offset < numBlks; offset++)
    {
        VirtualBlkAddr currentVsa = _GetVsa(offset);
        VirtualBlkAddr read = readMap[offset];

        // TODO (huijeong.kim) Read vsa can be the latest one than current vsa in the log
        if (IsSameVsa(read, currentVsa) == false)
        {
            if (replaySegmentInfo == true)
            {
                _InvalidateOldBlock(offset);
            }

            result = _UpdateMap(offset);
        }
    }

    if (status->IsFlushed() == false)
    {
        for (uint32_t offset = 0; offset < numBlks; offset++)
        {
            _UpdateReverseMap(offset);
        }
    }
    return result;
}

void
ReplayBlockMapUpdate::_UpdateReverseMap(uint32_t offset)
{
    wbStripeReplayer->UpdateRevMaps(volId, startVsa.stripeId, startVsa.offset + offset, startRba + offset);
}

void
ReplayBlockMapUpdate::_InvalidateOldBlock(uint32_t offset)
{
    VirtualBlkAddr read = readMap[offset];

    if (read.stripeId != UNMAP_STRIPE)
    {
        VirtualBlks blksToInvalidate = {
            .startVsa = read,
            .numBlks = 1};
        bool allowVictimSegRelease = true;
        segmentCtx->InvalidateBlks(blksToInvalidate, allowVictimSegRelease);
        status->BlockInvalidated(blksToInvalidate.numBlks);
    }
}

int
ReplayBlockMapUpdate::_UpdateMap(uint32_t offset)
{
    BlkAddr rba = _GetRba(offset);
    VirtualBlks virtualBlks = {
        .startVsa = _GetVsa(offset),
        .numBlks = 1};

    int result = vsaMap->SetVSAsWithSyncOpen(volId, rba, virtualBlks);
    assert(result >= 0);

    if (replaySegmentInfo == true)
    {
        segmentCtx->ValidateBlks(virtualBlks);
    }

    status->BlockWritten(virtualBlks.startVsa.offset, virtualBlks.numBlks);

    return result;
}

} // namespace pos
