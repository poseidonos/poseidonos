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

#include <vector>

#include "allocator_meta_archive.h"

namespace ibofos
{
using WbStripeIter = std::vector<Stripe*>::iterator;

class GcDuty;

class CommonDuty
{
public:
    CommonDuty(AllocatorAddressInfo& addrInfoI, AllocatorMetaArchive* metaI,
        GcDuty* gcDutyI);
    virtual ~CommonDuty(void);

    int Store(void);
    Stripe* GetStripe(StripeAddr& lsidEntry);
    void InvalidateBlks(VirtualBlks blks);

    void TryToResetSegmentState(StripeId lsid, bool replay = false);
    void FreeUserDataStripeId(StripeId lsid);
    bool IsValidWriteBufferStripeId(StripeId lsid);
    bool IsValidUserDataSegmentId(SegmentId segId);

    WbStripeIter GetwbStripeArrayBegin(void);
    WbStripeIter GetwbStripeArrayEnd(void);
    Stripe* GetStripe(StripeId wbLsid);
    int CheckAllActiveStripes(std::vector<Stripe*>& stripesToFlush,
        std::vector<StripeId>& vsidToCheckFlushDone);
    void PickActiveStripe(VolumeId volumeId, std::vector<Stripe*>& stripesToFlush,
        std::vector<StripeId>& vsidToCheckFlushDone);
    Stripe* FinishStripe(StripeId wbLsid, VirtualBlkAddr tail);

private:
    void _IncreaseInvCount(SegmentId segId, int count = 1);
    Stripe* _FinishActiveStripe(ASTailArrayIdx index);
    Stripe* _FinishRemainingBlocks(VirtualBlks remainingVsaRange);

    VirtualBlks _AllocateRemainingBlocks(ASTailArrayIdx index);
    VirtualBlks _AllocateRemainingBlocks(VirtualBlkAddr tail);

    AllocatorAddressInfo& addrInfo;
    AllocatorMetaArchive* meta;
    GcDuty* gcDuty;

    FreeBufferPool* stripeBufferPool;
    std::vector<Stripe*> wbStripeArray;
};

} // namespace ibofos
