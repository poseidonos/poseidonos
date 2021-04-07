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

#include "src/gc/copier_meta.h"

#include "src/include/meta_const.h"

namespace ibofos
{
CopierMeta::CopierMeta(uint64_t maxBufferCount, uint32_t bufferSize)
{
    InitProgressCount();
    _CreateBufferPool(maxBufferCount, bufferSize);
}

CopierMeta::~CopierMeta(void)
{
    for (uint32_t index = 0; index < GC_BUFFER_COUNT; index++)
    {
        delete gcBufferPool[index];
    }
}

void*
CopierMeta::GetBuffer(StripeId stripeId)
{
    return gcBufferPool[stripeId % GC_BUFFER_COUNT]->GetBuffer();
}

void
CopierMeta::ReturnBuffer(StripeId stripeId, void* buffer)
{
    gcBufferPool[stripeId % GC_BUFFER_COUNT]->ReturnBuffer(buffer);
}

void
CopierMeta::SetStartCopyStripes(void)
{
    requestCopyStripes++;
}

void
CopierMeta::SetStartCopyBlks(uint32_t blocks)
{
    startCount.fetch_add(blocks);
}

void
CopierMeta::SetDoneCopyBlks(uint32_t blocks)
{
    doneCount.fetch_add(blocks);
}

uint32_t
CopierMeta::GetStartCopyBlks(void)
{
    return startCount;
}

uint32_t
CopierMeta::GetDoneCopyBlks(void)
{
    return doneCount;
}

void
CopierMeta::InitProgressCount(void)
{
    requestCopyStripes = 0;
    startCount = 0;
    doneCount = 0;
}

bool
CopierMeta::IsSync(void)
{
    if (STRIPES_PER_SEGMENT != requestCopyStripes)
    {
        return false;
    }

    if (startCount != doneCount)
    {
        return false;
    }

    return true;
}

void
CopierMeta::_CreateBufferPool(uint64_t maxBufferCount, uint32_t bufferSize)
{
    for (uint32_t index = 0; index < GC_BUFFER_COUNT; index++)
    {
        gcBufferPool[index] = new FreeBufferPool(maxBufferCount, bufferSize);
    }
}

} // namespace ibofos
