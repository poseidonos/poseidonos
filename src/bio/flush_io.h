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

#include <memory>
#include <string>

#include "src/bio/ubio.h"
#include "src/bio/flush_states.h"

namespace pos
{

class FlushIo : public Ubio
{
public:
    explicit FlushIo(int arrayId);
    ~FlushIo(void) override;

    virtual uint32_t GetVolumeId(void);
    void SetVolumeId(uint32_t volumeId);

    uint32_t GetOriginCore(void) override;

    FlushState GetState(void);
    void SetState(FlushState state);

    bool IsVsaMapFlushComplete(void);
    void SetVsaMapFlushComplete(bool vsaMapFlushComplete);

    bool IsStripeMapFlushComplete(void);
    void SetStripeMapFlushComplete(bool stripeMapFlushComplete);

    bool IsAllocatorFlushComplete(void);
    void SetAllocatorFlushComplete(bool allocatorFlushComplete);

    bool IsStripeMapAllocatorFlushComplete(void);
    void SetStripeMapAllocatorFlushComplete(bool metaFlushComplete);

    void IncreaseStripeCnt(void);
    void DecreaseStripeCnt(void);

    void StripesScanComplete(void);

    bool IsInternalFlush(void);
    void SetInternalFlush(bool isInternalFlush);

    bool IsStripesFlushComplete(void);

private:
    uint32_t volumeId;
    uint32_t originCore;
    FlushState state;
    bool isInternalFlush;
    std::atomic<int32_t> stripeCnt;
    std::atomic<bool> stripesScanComplete;
    std::atomic<bool> stripesFlushComplete;
    std::atomic<bool> vsaMapFlushComplete;
    std::atomic<bool> stripeMapFlushComplete;
    std::atomic<bool> allocatorFlushComplete;
    std::atomic<bool> metaFlushComplete;
};

using FlushIoSmartPtr = std::shared_ptr<FlushIo>;

} // namespace pos
