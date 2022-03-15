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

#include <string>
#include <vector>

#include "src/allocator/i_context_replayer.h"

namespace pos
{
class AllocatorAddressInfo;
class AllocatorCtx;
class SegmentCtx;

class ContextReplayer : public IContextReplayer
{
public:
    ContextReplayer(void) = default;
    ContextReplayer(AllocatorCtx* allocatorCtx, SegmentCtx* segmentCtx, AllocatorAddressInfo* info);
    virtual ~ContextReplayer(void);

    virtual void ResetDirtyContextVersion(int owner);
    virtual void ReplaySsdLsid(StripeId currentSsdLsid);
    virtual void ReplaySegmentAllocation(StripeId userLsid);
    virtual void ReplayStripeAllocation(StripeId wbLsid, StripeId userLsid);
    virtual void ReplayStripeRelease(StripeId wbLsid);
    virtual void ReplayStripeFlushed(StripeId userLsid);
    virtual void SetActiveStripeTail(int index, VirtualBlkAddr tail, StripeId wbLsid);
    virtual void ResetActiveStripeTail(int index);
    virtual std::vector<VirtualBlkAddr> GetAllActiveStripeTail(void);
    virtual void ResetSegmentsStates(void);

private:
    AllocatorCtx* allocatorCtx;
    SegmentCtx* segmentCtx;

    AllocatorAddressInfo* addrInfo;
};

} // namespace pos
