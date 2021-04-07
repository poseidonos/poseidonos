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

#include "allocator_address_info.h"
#include "allocator_meta_archive.h"
#include "io_duty.h"
#include "src/scheduler/event.h"
#include "stripe.h"

namespace ibofos
{
using StripeVec = std::vector<Stripe*>;

class CommonDuty;
class MainDuty;

class JournalManagerDuty
{
public:
    JournalManagerDuty(AllocatorAddressInfo& addrInfoI, AllocatorMetaArchive* metaI,
        CommonDuty* commonDutyI, MainDuty* mainDutyI, IoDuty* ioDutyI);
    virtual ~JournalManagerDuty(void);

    int FlushMetadata(EventSmartPtr callback);
    void ReplaySsdLsid(StripeId currentSsdLsid);
    int FlushStripe(VolumeId volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa);
    int FlushFullActiveStripes(void);
    void ReplaySegmentAllocation(StripeId userLsid);
    void ReplayStripeAllocation(StripeId vsid, StripeId wbLsid);
    void ReplayStripeFlushed(StripeId wbLsid);
    std::vector<VirtualBlkAddr> GetActiveStripeTail(void);
    void ResetActiveStripeTail(int index);
    int RestoreActiveStripeTail(int index, VirtualBlkAddr tail, StripeId wbLsid);

private:
    int _ReconstructActiveStripe(StripeId vsid, StripeId wbLsid, uint64_t blockCount, Stripe*& stripe);
    int _ReconstructReverseMap(VolumeId volumeId, Stripe* stripe, uint64_t blockCount);
    bool
    _IsFirstStripeOfSegment(StripeId stripeId)
    {
        return (stripeId % addrInfo.GetstripesPerSegment() == 0);
    }

    StripeVec* pendingFullStripes;
    AllocatorAddressInfo& addrInfo;
    AllocatorMetaArchive* meta;
    CommonDuty* commonDuty;
    IoDuty* ioDuty;
    MainDuty* mainDuty;
};

} // namespace ibofos
