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

#include "src/allocator/i_segment_ctx.h"
#include "src/include/address_type.h"
#include "src/journal_manager/replay/replay_event.h"
#include "src/mapper/i_vsamap.h"
#include "src/mapper/include/mapper_const.h"

namespace pos
{
class ActiveWBStripeReplayer;
class ReplayBlockMapUpdate : public ReplayEvent
{
public:
    ReplayBlockMapUpdate(IVSAMap* ivsaMa, ISegmentCtx* segmentCtx,
        StripeReplayStatus* status, ActiveWBStripeReplayer* wbReplayer,
        int volId, BlkAddr startRba, VirtualBlkAddr startVsa, uint64_t numBlks,
        bool replaySegmentInfo);
    virtual ~ReplayBlockMapUpdate(void);

    virtual int Replay(void) override;

    inline ReplayEventType
    GetType(void)
    {
        return ReplayEventType::BLOCK_MAP_UPDATE;
    }

private:
    void _ReadBlockMap(void);

    void _InvalidateOldBlock(uint32_t offset);
    int _UpdateMap(uint32_t offset);
    void _UpdateReverseMap(uint32_t offset);

    inline VirtualBlkAddr
    _GetVsa(uint32_t offset)
    {
        VirtualBlkAddr vsa = startVsa;
        vsa.offset += offset;
        return vsa;
    }

    inline BlkAddr
    _GetRba(uint32_t offset)
    {
        return startRba + offset;
    }

    IVSAMap* vsaMap;
    ISegmentCtx* segmentCtx;

    int volId;
    BlkAddr startRba;
    VirtualBlkAddr startVsa;
    uint64_t numBlks;

    std::vector<VirtualBlkAddr> readMap;

    bool replaySegmentInfo;
    ActiveWBStripeReplayer* wbStripeReplayer;
};
} // namespace pos
