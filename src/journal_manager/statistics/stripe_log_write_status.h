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

#include <mutex>
#include <utility>

#include "src/include/address_type.h"
#include "src/include/pos_event_id.h"
#include "src/journal_manager/log/gc_map_update_list.h"
#include "src/journal_manager/log/log_event.h"
#include "src/journal_manager/statistics/stripe_info.h"
#include "src/logger/logger.h"

namespace pos
{
class StripeLogWriteStatus : public StripeInfo
{
public:
    StripeLogWriteStatus(void) = delete;
    explicit StripeLogWriteStatus(StripeId vsid);
    virtual ~StripeLogWriteStatus(void);

    virtual void BlockLogFound(BlockWriteDoneLog dat);
    virtual void StripeLogFound(StripeMapUpdatedLog dat);

    virtual void GcBlockLogFound(GcBlockMapUpdate* mapUpdate, uint32_t numBlks);
    virtual void GcStripeLogFound(GcStripeFlushedLog dat);

    virtual void Print(void);

    virtual bool
    IsFlushed(void)
    {
        return stripeFlushed == true;
    }

    // For unit test
    void SetFirstBlockOffset(BlkOffset offset);
    void SetLastBlockOffset(BlkOffset offset);

    std::pair<BlkOffset, BlkOffset> GetBlockOffsetRange(void);
    std::pair<BlkAddr, BlkAddr> GetRbaRange(void);
    uint32_t GetNumFoundBlocks(void);
    StripeAddr GetFinalStripeAddr(void);

protected:
    uint32_t numFoundBlockMaps;
    BlkAddr smallestRba;
    BlkAddr largestRba;
    BlkOffset firstBlockOffset;
    BlkOffset lastBlockOffset;
    StripeAddr finalStripeAddr;
    bool stripeFlushed;

    std::mutex statusLock;

    static const BlkOffset INVALID_OFFSET = UINT64_MAX;

private:
    void _UpdateOffset(BlkOffset startOffset, uint32_t numBlks);
    void _UpdateRba(BlkAddr rba, uint32_t numBlks);
};

} // namespace pos
