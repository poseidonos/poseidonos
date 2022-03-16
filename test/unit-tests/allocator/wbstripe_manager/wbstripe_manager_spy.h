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
#include <set>
#include <string>
#include <vector>

#include "src/allocator/wbstripe_manager/wbstripe_manager.h"
#include "src/io/backend_io/flush_submission.h"

namespace pos
{
using StripeVec = std::vector<Stripe*>;
class IVolumeManager;
class WBStripeManagerSpy : public WBStripeManager
{
public:
    using WBStripeManager::WBStripeManager;
    virtual ~WBStripeManagerSpy(void) = default;

    int
    _ReconstructAS(StripeId vsid, StripeId wbLsid, uint64_t blockCount, ASTailArrayIdx tailarrayidx, Stripe*& stripe)
    {
        return WBStripeManager::_ReconstructAS(vsid, wbLsid, blockCount, tailarrayidx, stripe);
    }
    VirtualBlks
    _GetRemainingBlocks(VirtualBlkAddr tail)
    {
        return WBStripeManager::_GetRemainingBlocks(tail);
    }
    bool
    _FillBlocksToStripe(Stripe* stripe, StripeId wbLsid, BlkOffset startOffset, uint32_t numBlks)
    {
        return WBStripeManager::_FillBlocksToStripe(stripe, wbLsid, startOffset, numBlks);
    }
    Stripe*
    _FinishActiveStripe(ASTailArrayIdx index)
    {
        return WBStripeManager::_FinishActiveStripe(index);
    }
};

} // namespace pos
