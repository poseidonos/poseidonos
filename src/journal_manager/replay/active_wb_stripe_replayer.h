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

#include <map>
#include <vector>

#include "active_stripe_address.h"
#include "pending_stripe.h"
#include "src/allocator/i_context_replayer.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/include/address_type.h"
#include "src/journal_manager/statistics/stripe_info.h"
#include "src/mapper/i_stripemap.h"

namespace pos
{
class Array;
class ReplayStripe;

class ActiveWBStripeReplayer
{
public:
    explicit ActiveWBStripeReplayer(PendingStripeList& pendingStripeList);
    ActiveWBStripeReplayer(IContextReplayer* contextReplayer, IWBStripeAllocator* iwbstripeAllocator,
        IStripeMap* stripeMap, PendingStripeList& pendingStripeList, IArrayInfo* aInfo);
    virtual ~ActiveWBStripeReplayer(void);

    virtual int Replay(void);
    virtual void Update(StripeInfo info);
    virtual void UpdateRevMaps(int volId, StripeId vsid, uint64_t offset, BlkAddr startRba);

protected:
    ActiveStripeAddr* _FindStripe(int index, StripeId vsid);

private:
    int _FindWbufIndex(StripeInfo stripeInfo);
    void _AddResetTailToFoundList(int index);
    void _AddWbufTailToFoundList(int index, ActiveStripeAddr addr);
    bool _IsFlushedStripe(StripeInfo stripeInfo);

    ActiveStripeAddr _FindTargetActiveStripeAndRestore(int index);
    int _RestoreActiveStripes(void);
    int _RestorePendingStripes(void);

    void _AddActiveStripeToRestore(int index);
    void _SetActiveStripeTail(int index, ActiveStripeAddr addr);
    void _ResetActiveStripeTail(int index);

    // TODO(meta): This is same with BlockManager::_IsStripeFull, call block manager instead of this function
    bool _IsStripeFull(VirtualBlkAddr vsa);
    uint32_t _GetNumBlksPerStripe(void);

    const int INDEX_NOT_FOUND = -1;

    using PendingActiveStripeList = std::vector<ActiveStripeAddr>;

    std::vector<VirtualBlkAddr> readTails;

    std::vector<PendingActiveStripeList> foundActiveStripes;
    PendingActiveStripeList pendingActiveStripes;

    PendingStripeList& pendingStripes;
    IContextReplayer* contextReplayer;
    IWBStripeAllocator* wbStripeAllocator;
    IStripeMap* stripeMap;

    IArrayInfo* arrayInfo;
};

} // namespace pos
