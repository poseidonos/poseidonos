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

#include <atomic>
#include <vector>

#include "src/mapper/reverse_map.h"

namespace ibofos
{
using ASTailArrayIdx = uint32_t;
using VolumeId = uint32_t;
using DataBufferIter = std::vector<void*>::iterator;

class Stripe
{
public:
    Stripe(void);
    void Assign(StripeId vsid, StripeId lsid);

    uint32_t GetAsTailArrayIdx(void);
    void SetAsTailArrayIdx(ASTailArrayIdx idx);

    StripeId GetVsid(void);
    void SetVsid(StripeId virtsid);

    StripeId GetWbLsid(void);
    void SetWbLsid(StripeId wbAreaLsid);

    StripeId GetUserLsid(void);
    void SetUserLsid(StripeId userAreaLsid);

    int Flush(EventSmartPtr callback);
    void UpdateReverseMap(uint32_t offset, BlkAddr rba, VolumeId volumeId);
    int ReconstructReverseMap(VolumeId volumeId, uint64_t blockCount);
    int LinkReverseMap(ReverseMapPack* revMapPackToLink);
    int UnLinkReverseMap(void);

    bool IsFinished(void);
    void SetFinished(bool state);

    uint32_t GetBlksRemaining(void);
    uint32_t DecreseBlksRemaining(uint32_t amount);

    void Refer(void);
    void Derefer(uint32_t blockCount);
    bool IsOkToFree(void);

    void AddDataBuffer(void* buf);
    DataBufferIter DataBufferBegin(void);
    DataBufferIter DataBufferEnd(void);

private:
    ASTailArrayIdx asTailArrayIdx;
    StripeId vsid; // SSD LSID, Actually User Area LSID
    StripeId wbLsid;
    StripeId userLsid;
    ReverseMapPack* revMapPack;

    std::atomic<bool> finished;
    std::atomic<uint32_t> remaining; // #empty block(s) left, on this stripe
    std::atomic<uint32_t> referenceCount;
    std::vector<void*> dataBuffer;
};

} // namespace ibofos
