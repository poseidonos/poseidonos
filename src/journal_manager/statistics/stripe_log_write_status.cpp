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

#include "src/journal_manager/statistics/stripe_log_write_status.h"

#include <iomanip>
#include <iostream>

#include "src/include/pos_event_id.h"
#include "src/journal_manager/log/gc_map_update_list.h"
#include "src/logger/logger.h"

namespace pos
{
StripeLogWriteStatus::StripeLogWriteStatus(StripeId vsid)
: StripeInfo(vsid),
  numFoundBlockMaps(0),
  smallestRba(UINT64_MAX),
  largestRba(0),
  firstBlockOffset(INVALID_OFFSET),
  lastBlockOffset(INVALID_OFFSET),
  stripeFlushed(false)
{
    finalStripeAddr.stripeId = UNMAP_STRIPE;
    finalStripeAddr.stripeLoc = IN_WRITE_BUFFER_AREA;
}

StripeLogWriteStatus::~StripeLogWriteStatus(void)
{
}

void
StripeLogWriteStatus::BlockLogFound(BlockWriteDoneLog dat)
{
    std::lock_guard<std::mutex> lock(statusLock);

    numFoundBlockMaps += dat.numBlks;

    _UpdateOffset(dat.startVsa.offset, dat.numBlks);
    _UpdateRba(dat.startRba, dat.numBlks);

    finalStripeAddr.stripeId = (dat.writeBufferStripeAddress).stripeId;
    finalStripeAddr.stripeLoc = IN_WRITE_BUFFER_AREA;

    BlkOffset curEndOffset = dat.startVsa.offset + dat.numBlks - 1;
    _UpdateLastOffset(curEndOffset);
    _UpdateVolumeId(dat.volId);
    _UpdateWbLsid((dat.writeBufferStripeAddress).stripeId);
    _UpdateWbIndex(dat.wbIndex);
}

void
StripeLogWriteStatus::_UpdateOffset(BlkOffset startOffset, uint32_t numBlks)
{
    BlkOffset endOffset = startOffset + numBlks - 1;
    if ((firstBlockOffset == INVALID_OFFSET) ||
        (startOffset < firstBlockOffset))
    {
        firstBlockOffset = startOffset;
    }

    if ((lastBlockOffset == INVALID_OFFSET) ||
        (lastBlockOffset < endOffset))
    {
        lastBlockOffset = endOffset;
    }
}

void
StripeLogWriteStatus::_UpdateRba(BlkAddr rba, uint32_t numBlks)
{
    if (smallestRba > rba)
    {
        smallestRba = rba;
    }
    if (largestRba < rba + numBlks - 1)
    {
        largestRba = rba + numBlks - 1;
    }
}

void
StripeLogWriteStatus::StripeLogFound(StripeMapUpdatedLog dat)
{
    assert(dat.oldMap.stripeLoc == IN_WRITE_BUFFER_AREA);
    assert(dat.newMap.stripeLoc == IN_USER_AREA);

    std::lock_guard<std::mutex> lock(statusLock);

    stripeFlushed = true;

    finalStripeAddr.stripeId = dat.newMap.stripeId;
    finalStripeAddr.stripeLoc = IN_USER_AREA;

    _UpdateWbLsid(dat.oldMap.stripeId);
    _UpdateUserLsid(dat.newMap.stripeId);

    ResetOffset();
}

void
StripeLogWriteStatus::GcBlockLogFound(GcBlockMapUpdate* mapUpdate, uint32_t numBlks)
{
    std::lock_guard<std::mutex> lock(statusLock);

    numFoundBlockMaps += numBlks;

    for (uint32_t count = 0; count < numBlks; count++)
    {
        _UpdateOffset(mapUpdate[count].vsa.offset, 1);
        _UpdateRba(mapUpdate[count].rba, 1);
    }

    _UpdateLastOffset(lastBlockOffset);
}

void
StripeLogWriteStatus::GcStripeLogFound(GcStripeFlushedLog dat)
{
    std::lock_guard<std::mutex> lock(statusLock);

    stripeFlushed = true;

    finalStripeAddr.stripeId = dat.userLsid;
    finalStripeAddr.stripeLoc = IN_USER_AREA;

    _UpdateVolumeId(dat.volId);
    _UpdateWbLsid(dat.wbLsid);
    _UpdateUserLsid(dat.userLsid);
}

// TODO (cheolho.kang) Will be updated after introducing LogWriteStatistics
void
StripeLogWriteStatus::Print(void)
{
    int eventId = static_cast<int>(EID(JOURNAL_DEBUG));
    std::ostringstream os;

    os << "Stripe written(vsid " << GetVsid() << "), ";
    if (GetVolumeId() != INT32_MAX)
    {
        os << "vol " << GetVolumeId() << " ";
    }
    if (IsWbIndexValid() == true)
    {
        os << "wbIndex " << GetWbIndex() << " ";
    }
    if (IsLastOffsetValid() == true)
    {
        os << "last offset " << GetLastOffset() << " ";
        os << "(rba " << smallestRba << " to " << largestRba << " )";
    }
    if (stripeFlushed == true)
    {
        os << "stripe flushed ";
    }
    os << numFoundBlockMaps << "blks written ";
    os << "userLsid " << GetUserLsid() << " ";
    POS_TRACE_DEBUG(eventId, os.str());
}

// For unit test
void
StripeLogWriteStatus::SetFirstBlockOffset(BlkOffset offset)
{
    firstBlockOffset = offset;
}

void
StripeLogWriteStatus::SetLastBlockOffset(BlkOffset offset)
{
    lastBlockOffset = offset;
}

std::pair<BlkOffset, BlkOffset>
StripeLogWriteStatus::GetBlockOffsetRange(void)
{
    return std::make_pair(firstBlockOffset, lastBlockOffset);
}

std::pair<BlkAddr, BlkAddr>
StripeLogWriteStatus::GetRbaRange(void)
{
    return std::make_pair(smallestRba, largestRba);
}

uint32_t
StripeLogWriteStatus::GetNumFoundBlocks(void)
{
    return numFoundBlockMaps;
}

StripeAddr
StripeLogWriteStatus::GetFinalStripeAddr(void)
{
    return finalStripeAddr;
}

} // namespace pos
