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

#include <list>
#include <map>
#include <unordered_map>

#include "src/include/address_type.h"
#include "src/array_models/interface/i_array_info.h"

namespace pos
{
class IContextReplayer;
class ReplayStripe;

class ActiveUserStripeReplayer
{
public:
    ActiveUserStripeReplayer(void) = default;
    ActiveUserStripeReplayer(IContextReplayer* ictxReplayer, IArrayInfo* array);
    virtual ~ActiveUserStripeReplayer(void);

    virtual int Replay(void);
    virtual void Update(StripeId userLsid);

private:
    void _Reset(void);
    void _UpdateLastLsidOfSegment(uint32_t stripesPerSegment);
    void _EraseFullSegmentLsid(uint32_t stripesPerSegment);

    inline bool
    _IsLastStripe(StripeId lsid, uint32_t stripesPerSegment)
    {
        return ((lsid + 1) % stripesPerSegment == 0);
    }
    StripeId _FindLastLsid(uint32_t stripesPerSegment);

    std::list<StripeId> userLsids;
    std::unordered_map<SegmentId, StripeId> lastLsid;

    IContextReplayer* contextReplayer;
    IArrayInfo* arrayInfo;
};

} // namespace pos
