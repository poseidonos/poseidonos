/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "src/mapper/i_reversemap.h"
#include "src/mapper/i_stripemap.h"
#include "src/include/smart_ptr_type.h"

namespace pos
{
class ContextManager;
class IWBStripeAllocator;
class AllocatorAddressInfo;
class AllocatorCtx;
class BlockAllocationStatus;

class StripeManager
{
public:
    StripeManager(void) = default;
    StripeManager(ContextManager* ctxMgr, AllocatorAddressInfo* addrInfo_, int arrayId);
    StripeManager(ContextManager* ctxMgr, IReverseMap* iReverseMap_, IStripeMap* stripeMap_, AllocatorAddressInfo* addrInfo_, int arrayId);
    virtual ~StripeManager(void) = default;

    virtual void Init(IWBStripeAllocator* wbStripeManager);

    virtual std::pair<StripeId, StripeId> AllocateStripesForUser(uint32_t volumeId);
    virtual StripeSmartPtr AllocateGcDestStripe(uint32_t volumeId);

protected:
    bool _IsLastStripesWithinSegment(StripeId stripeId);

private:
    StripeId _AllocateSsdStripe(void);
    StripeId _AllocateSegmentAndStripe(void);
    StripeId _AllocateWbStripe(void);
    void _RollBackWbStripeIdAllocation(StripeId wbLsid = UINT32_MAX);

    StripeId _AllocateUserSsdStripe(int volumeId);
    StripeSmartPtr _AllocateStripe(StripeId vsid, StripeId wbLsid, uint32_t volumeId);

    IWBStripeAllocator* wbStripeManager;
    ContextManager* contextManager;
    AllocatorCtx* allocCtx;
    BlockAllocationStatus* allocStatus;
    AllocatorAddressInfo* addrInfo;
    int arrayId;

    IReverseMap* reverseMap;
    IStripeMap* stripeMap;
};

} // namespace pos
