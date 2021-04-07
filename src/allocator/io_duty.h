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

#include <atomic>

#include "allocator_meta_archive.h"
#include "segment_info.h"

namespace ibofos
{
class CommonDuty;
class GcDuty;

class IoDuty
{
public:
    IoDuty(AllocatorAddressInfo& addrInfoI, AllocatorMetaArchive* metaI,
        GcDuty* gcDutyI, CommonDuty* commonDutyI);
    virtual ~IoDuty(void);

    VirtualBlks AllocateWriteBufferBlks(VolumeId volumeId, uint32_t numBlks);
    VirtualBlks AllocateGcBlk(VolumeId volumeId, uint32_t numBlks);
    StripeId AllocateUserDataStripeId(StripeId vsid);
    VirtualBlks AllocateWriteBufferBlksFromNewStripe(ASTailArrayIdx asTailArrayIdx,
        StripeId vsid, int numBlks);

    SegmentId AllocateUserDataSegmentId(void);
    void UsedSegmentStateChange(SegmentId segmentId, SegmentState to);
    void TurnOffBlkAllocation(void);
    void TurnOnBlkAllocation(void);

private:
    VirtualBlks _AllocateBlks(ASTailArrayIdx asTailArrayIdx, int numBlks);
    int _AllocateStripe(ASTailArrayIdx asTailArrayIdx, StripeId& vsid);
    StripeId _AllocateWriteBufferStripeId(void);
    StripeId _AllocateUserDataStripeIdInternal(bool stopAtUrgent);
    void _RollBackStripeIdAllocation(StripeId wbLsid = UINT32_MAX,
        StripeId arrayLsid = UINT32_MAX);

    bool
    _IsValidOffset(uint64_t stripeOffset)
    {
        return stripeOffset < addrInfo.GetblksPerStripe();
    }
    bool
    _IsStripeFull(VirtualBlkAddr addr)
    {
        return addr.offset == addrInfo.GetblksPerStripe();
    }
    bool
    _IsSegmentFull(StripeId stripeId)
    {
        return stripeId % addrInfo.GetstripesPerSegment() == 0;
    }
    bool
    _IsUserStripeAllocation(ASTailArrayIdx asTailArrayIdx)
    {
        return (asTailArrayIdx < MAX_VOLUME_COUNT);
    }

    AllocatorAddressInfo& addrInfo;
    AllocatorMetaArchive* meta;
    CommonDuty* commonDuty;
    GcDuty* gcDuty;

    std::atomic<bool> blockWholeBlockAllocation;
    std::mutex lockBlockAlloc;
};

} // namespace ibofos
