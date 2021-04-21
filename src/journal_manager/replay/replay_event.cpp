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

#include "replay_event.h"

#include "src/allocator/allocator.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/include/pos_event_id.h"
#include "src/journal_manager/statistics/stripe_replay_status.h"
#include "src/logger/logger.h"
#include "src/mapper/i_stripemap.h"
#include "src/mapper/i_vsamap.h"
#include "src/mapper/include/mapper_const.h"

namespace pos
{
ReplayEvent::ReplayEvent(StripeReplayStatus* status)
: status(status)
{
}

ReplayEvent::~ReplayEvent(void)
{
}

ReplayBlockMapUpdate::ReplayBlockMapUpdate(IVSAMap* vsaMap, IBlockAllocator* blkAllocator,
    StripeReplayStatus* status, BlockWriteDoneLog dat)
: ReplayEvent(status),
  vsaMap(vsaMap),
  blockAllocator(blkAllocator),
  logData(dat)
{
}

ReplayBlockMapUpdate::~ReplayBlockMapUpdate(void)
{
}

void
ReplayBlockMapUpdate::_ReadBlockMap(void)
{
    readMap.resize(logData.numBlks);

    for (uint32_t offset = 0; offset < logData.numBlks; offset++)
    {
        int shouldRetry = CALLER_NOT_EVENT;
        readMap[offset] = vsaMap->GetVSAInternal(logData.volId,
            logData.startRba + offset, shouldRetry);
        assert(shouldRetry == OK_READY);
    }
}

int
ReplayBlockMapUpdate::Replay(void)
{
    _ReadBlockMap();

    int result = 0;
    for (uint32_t offset = 0; offset < logData.numBlks; offset++)
    {
        VirtualBlkAddr currentVsa = _GetVsa(offset);
        VirtualBlkAddr read = readMap[offset];

        if (IsSameVsa(read, currentVsa) == false)
        {
            _InvalidateOldBlock(offset);
            result = _UpdateMap(offset);
        }
    }

    return result;
}

void
ReplayBlockMapUpdate::_InvalidateOldBlock(uint32_t offset)
{
    VirtualBlkAddr read = readMap[offset];

    // TODO (huijeong.kim) : remove isGC, oldVsa from the log
    assert(logData.isGC == false);

    if (read.stripeId != UNMAP_STRIPE)
    {
        VirtualBlks blksToInvalidate = {
            .startVsa = read,
            .numBlks = 1};
        blockAllocator->InvalidateBlks(blksToInvalidate);
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

    int result = vsaMap->SetVSAsInternal(logData.volId, rba, virtualBlks);
    blockAllocator->ValidateBlks(virtualBlks);
    status->BlockWritten(virtualBlks.startVsa.offset, virtualBlks.numBlks);

    return result;
}

ReplayStripeMapUpdate::ReplayStripeMapUpdate(IStripeMap* stripeMap,
    StripeReplayStatus* status, StripeId vsid, StripeAddr dest)
: ReplayEvent(status),
  stripeMap(stripeMap),
  vsid(vsid),
  dest(dest)
{
}

ReplayStripeMapUpdate::~ReplayStripeMapUpdate(void)
{
}

int
ReplayStripeMapUpdate::Replay(void)
{
    int ret = stripeMap->SetLSA(vsid, dest.stripeId, dest.stripeLoc);
    // TODO(huijeong.kim) notify replay status that stripe map is updated
    return ret;
}

ReplayStripeAllocation::ReplayStripeAllocation(IStripeMap* stripeMap,
    IWBStripeCtx* wbStripeCtx, StripeReplayStatus* status,
    StripeId vsid, StripeId wbLsid)
: ReplayEvent(status),
  stripeMap(stripeMap),
  wbStripeCtx(wbStripeCtx),
  vsid(vsid),
  wbLsid(wbLsid)
{
}

ReplayStripeAllocation::~ReplayStripeAllocation(void)
{
}

int
ReplayStripeAllocation::Replay(void)
{
    int result = 0;

    result = stripeMap->SetLSA(vsid, wbLsid, IN_WRITE_BUFFER_AREA);

    if (result != 0)
    {
        return result;
    }

    wbStripeCtx->ReplayStripeAllocation(vsid, wbLsid);
    status->StripeAllocated();
    return result;
}

ReplaySegmentAllocation::ReplaySegmentAllocation(ISegmentCtx* isegCtx,
    IArrayInfo* arrayInfo, StripeReplayStatus* status, StripeId stripeId)
: ReplayEvent(status),
  segmentCtx(isegCtx),
  arrayInfo(arrayInfo),
  userLsid(stripeId)
{
}

ReplaySegmentAllocation::~ReplaySegmentAllocation(void)
{
}

int
ReplaySegmentAllocation::Replay(void)
{
    uint32_t numStripesPerSegment = arrayInfo->GetSizeInfo(PartitionType::USER_DATA)->stripesPerSegment;
    SegmentId segmentId = userLsid / numStripesPerSegment;
    StripeId firstStripe = segmentId * numStripesPerSegment;

    segmentCtx->ReplaySegmentAllocation(firstStripe);
    status->SegmentAllocated();
    return 0;
}

ReplayStripeFlush::ReplayStripeFlush(IWBStripeCtx* wbStripeCtx, ISegmentCtx* segCtx,
    StripeReplayStatus* status, StripeId vsid, StripeId wbLsid, StripeId userLsid)
: ReplayEvent(status),
  wbStripeCtx(wbStripeCtx),
  segmentCtx(segCtx),
  vsid(vsid),
  wbLsid(wbLsid),
  userLsid(userLsid)
{
}

ReplayStripeFlush::~ReplayStripeFlush(void)
{
}

int
ReplayStripeFlush::Replay(void)
{
    wbStripeCtx->ReplayStripeFlushed(wbLsid);
    segmentCtx->UpdateOccupiedStripeCount(userLsid);

    status->StripeFlushed();

    return 0;
}

} // namespace pos
