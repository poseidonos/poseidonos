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

#include <utility>
#include <vector>

#include "src/include/address_type.h"
#include "test_info.h"

using namespace ibofos;

inline uint32_t
GetWbLsid(int vsid)
{
    return (uint32_t)(vsid);
}

inline StripeId
GetUserLsid(int vsid, TestInfo* testInfo)
{
    return vsid + testInfo->numWbStripes;
}

static const StripeAddr unmapAddr = {
    .stripeLoc = IN_WRITE_BUFFER_AREA,
    .stripeId = UNMAP_STRIPE};

struct StripeWriteInfo
{
    StripeId vsid;

    StripeAddr wbAddr;
    StripeAddr userAddr;

    int volId;

    StripeWriteInfo(void)
    {
        vsid = UNMAP_STRIPE;
        volId = -1;

        wbAddr = unmapAddr;
        userAddr = unmapAddr;
    }
    StripeWriteInfo(int _vsid, int _volId)
    {
        vsid = _vsid;
        volId = _volId;

        wbAddr.stripeId = GetWbLsid(vsid);
        wbAddr.stripeLoc = IN_WRITE_BUFFER_AREA;

        userAddr.stripeId = _vsid;
        userAddr.stripeLoc = IN_USER_AREA;
    }
};

using BlockMapList = std::vector<std::pair<BlkAddr, VirtualBlks>>;

struct StripeLog
{
    StripeWriteInfo stripe;
    BlockMapList blks;

    StripeLog(void)
    {
    }

    StripeLog(StripeWriteInfo s, BlockMapList b)
    {
        stripe = s;
        blks = b;
    }
};
