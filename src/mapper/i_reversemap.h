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
#include "src/include/address_type.h"
#include <tuple>
#include <map>
namespace pos
{
class ReverseMapPack;
class Stripe;

class IReverseMap
{
public:
    virtual int Load(ReverseMapPack* rev, StripeId wblsid, StripeId vsid, EventSmartPtr cb) = 0;
    virtual int UpdateReverseMapEntry(ReverseMapPack* rev, StripeId wblsid, uint64_t offset, BlkAddr rba, uint32_t volumeId) = 0;
    virtual std::tuple<BlkAddr, uint32_t> GetReverseMapEntry(ReverseMapPack* rev, StripeId wblsid, uint64_t offset) = 0;
    virtual ReverseMapPack* Assign(StripeId wblsid, StripeId vsid) = 0;
    virtual ReverseMapPack* AllocReverseMapPack(uint32_t vsid) = 0;
    virtual int ReconstructReverseMap(uint32_t volumeId, uint64_t totalRba, uint32_t wblsid, uint32_t vsid, uint64_t blockCount, std::map<uint64_t, BlkAddr> revMapInfos) = 0;
    virtual int Flush(ReverseMapPack* rev, StripeId wblsid, Stripe* stripe, StripeId vsid, EventSmartPtr cb) = 0;
};

} // namespace pos
