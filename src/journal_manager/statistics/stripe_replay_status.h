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

#include "src/journal_manager/statistics/stripe_log_write_status.h"

#include "src/include/address_type.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
class LogHandlerInterface;

class StripeReplayStatus : public StripeLogWriteStatus
{
public:
    StripeReplayStatus(void) = delete;
    explicit StripeReplayStatus(StripeId vsid);
    virtual ~StripeReplayStatus(void);

    virtual void Print(void) override;

    virtual void BlockWritten(BlkOffset startOffset, uint32_t numBlks);
    virtual void StripeFlushed(void);
    virtual void BlockInvalidated(uint32_t numBlks);

    virtual void SegmentAllocated(void);
    virtual void StripeAllocated(void);

    virtual void RecordLogFoundTime(uint64_t time);

    // For unit test
    inline uint32_t
    GetNumUpdatedBlockMaps(void)
    {
        return numUpdatedBlockMaps;
    }

    inline bool
    GetStripeMapReplayed(void)
    {
        return stripeMapReplayed;
    }

    inline uint32_t
    GetNumInvalidatedBlocks(void)
    {
        return numInvalidatedBlocks;
    }

    inline bool
    GetSegmentAllocated(void)
    {
        return segmentAllocated;
    }

    inline bool
    GetStripeAllocated(void)
    {
        return stripeAllocated;
    }

private:
    std::string _BoolToString(bool val);

    static const BlkOffset INVALID_OFFSET = UINT64_MAX;
    uint32_t numUpdatedBlockMaps;
    uint32_t numInvalidatedBlocks;

    bool segmentAllocated;
    bool stripeAllocated;
    bool stripeMapReplayed;

    uint64_t minTime;
    uint64_t maxTime;
};

} // namespace pos
