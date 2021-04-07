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

#include <thread>

#include "allocator.h"

namespace ibofos
{
Allocator::Allocator(void)
: VolumeEvent("Allocator")
{
}

Allocator::~Allocator(void)
{
}

void
Allocator::FreeWriteBufferStripeId(StripeId lsid)
{
}

void
Allocator::TryToUpdateSegmentValidBlks(StripeId lsid)
{
}

VirtualBlks
Allocator::AllocateWriteBufferBlks(uint32_t volumeId, uint32_t numBlks)
{
    VirtualBlks blks = {.startVsa = UNMAP_VSA, .numBlks = 0};
    return blks;
}

VirtualBlks
Allocator::AllocateGcBlk(uint32_t volumeId, uint32_t numBlks)
{
    VirtualBlks blks = {.startVsa = UNMAP_VSA, .numBlks = 0};
    return blks;
}

StripeId
Allocator::AllocateUserDataStripeId(StripeId vsid)
{
    return vsid;
}

uint32_t
Allocator::GetNumOfFreeUserDataSegment(void)
{
    return 0;
}

SegmentId
Allocator::GetMostInvalidatedSegment(void)
{
    return 0;
}

void
Allocator::SetGcThreshold(uint32_t inputThreshold)
{
}

void
Allocator::SetUrgentThreshold(uint32_t inputThreshold)
{
}

uint32_t
Allocator::GetGcThreshold()
{
    return 0;
}

uint32_t
Allocator::GetUrgentThreshold()
{
    return 0;
}

void
Allocator::SetBlockingSegmentAllocation(bool isBlock)
{
}

void
Allocator::FreeUserDataSegmentId(SegmentId segId)
{
}

int
Allocator::PrepareRebuild()
{
    return 0;
}

int
Allocator::StopRebuilding()
{
    return 0;
}

bool
Allocator::NeedRebuildAgain()
{
    return false;
}

SegmentId
Allocator::GetRebuildTargetSegment()
{
    return 0;
}

int
Allocator::ReleaseRebuildSegment(SegmentId segmentId)
{
    return 0;
}

int
Allocator::FlushMetadata(EventSmartPtr callbackEvent)
{
    bool result = callbackEvent->Execute();
    if (result == true)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

void
Allocator::ReplaySsdLsid(StripeId currentSsdLsid)
{
}

int
Allocator::FlushStripe(VolumeId volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa)
{
    return 0;
}

void
Allocator::FlushFullActiveStripes(void)
{
}

void
Allocator::ReplaySegmentAllocation(StripeId userLsid)
{
}

void
Allocator::ReplayStripeAllocation(StripeId vsid, StripeId wbLsid)
{
}

void
Allocator::ReplayStripeFlushed(StripeId wbLsid)
{
}

std::vector<VirtualBlkAddr>
Allocator::GetActiveStripeTail(void)
{
    std::vector<VirtualBlkAddr> ret(ACTIVE_STRIPE_TAIL_ARRAYLEN, UNMAP_VSA);
    return ret;
}

void
Allocator::ResetActiveStripeTail(int index)
{
}

int
Allocator::RestoreActiveStripeTail(int index, VirtualBlkAddr tail, StripeId wbLsid)
{
    return 0;
}

void
Allocator::FlushAllUserdata()
{
}

int
Allocator::Store()
{
    return 0;
}

Stripe*
Allocator::GetStripe(StripeAddr& lsidEntry)
{
    return nullptr;
}

void
Allocator::InvalidateBlks(VirtualBlks blks)
{
}

void
Allocator::TryToResetSegmentState(StripeId lsid, bool replay)
{
}

int
Allocator::GetMeta(AllocatorMetaType type, std::string fname)
{
    return 0;
}

int
Allocator::GetBitmapLayout(std::string fname)
{
    return 0;
}

void
Allocator::FreeUserDataStripeId(StripeId lsid)
{
}

bool
Allocator::IsValidWriteBufferStripeId(StripeId lsid)
{
    return true;
}

bool
Allocator::IsValidUserDataSegmentId(SegmentId segId)
{
    return true;
}

bool
Allocator::VolumeCreated(std::string volName, int volID, uint64_t volSizeBytem,
    uint64_t maxiops, uint64_t maxbw)
{
    return true;
}

bool
Allocator::VolumeUpdated(std::string volName, int volID, uint64_t maxiops,
    uint64_t maxbw)
{
    return true;
}

bool
Allocator::VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte)
{
    return true;
}

bool
Allocator::VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte,
    uint64_t maxiops, uint64_t maxbw)
{
    return true;
}

bool
Allocator::VolumeUnmounted(std::string volName, int volID)
{
    return true;
}

bool
Allocator::VolumeLoaded(std::string name, int id, uint64_t totalSize,
    uint64_t maxiops, uint64_t maxbw)
{
    return true;
}

void
Allocator::VolumeDetached(vector<int> volList)
{
}

} // namespace ibofos
