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

#include "buffer_entry.h"

#include <cassert>

#include "src/array/config/array_config.h"
#include "src/array/free_buffer_pool.h"
#include "src/include/branch_prediction.h"
#include "src/include/ibof_event_id.hpp"
#include "src/logger/logger.h"

namespace ibofos
{
BufferEntry::BufferEntry(void)
{
    Reset();
}

void
BufferEntry::Reset(void)
{
    buffer = nullptr;
    blkCnt = 0;
    isParity = false;
    freeBufferPool = nullptr;
}

void
BufferEntry::SetBuffer(void* inputBuffer)
{
    buffer = inputBuffer;
}

void
BufferEntry::SetBlkCnt(uint32_t inputBlkCnt)
{
    blkCnt = inputBlkCnt;
}

void
BufferEntry::SetFreeBufferPool(FreeBufferPool* inputFreeBufferPool)
{
    freeBufferPool = inputFreeBufferPool;
}

void
BufferEntry::ReturnBuffer(void)
{
    if (nullptr != freeBufferPool)
    {
        freeBufferPool->ReturnBuffer(buffer);
        Reset();
    }
}

BufferEntry::BufferEntry(void* inputBuffer, uint32_t inputBlkCnt, bool inputIsParity)
{
    Reset();
    SetBuffer(inputBuffer);
    SetBlkCnt(inputBlkCnt);
    isParity = inputIsParity;
}
BufferEntry::~BufferEntry(void)
{
}

void*
BufferEntry::GetBufferEntry(void) const
{
    return buffer;
}

uint32_t
BufferEntry::GetBlkCnt(void)
{
    return blkCnt;
}
bool
BufferEntry::IsParity(void)
{
    return isParity;
}

void*
BufferEntry::GetBlock(uint32_t blockIndex)
{
    if (unlikely(blockIndex >= blkCnt))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::BUFFER_ENTRY_OUT_OF_RANGE,
            IbofEventId::GetString(IBOF_EVENT_ID::BUFFER_ENTRY_OUT_OF_RANGE));
        return nullptr;
    }

    uint8_t(*block)[ArrayConfig::BLOCK_SIZE_BYTE] =
        static_cast<uint8_t(*)[ArrayConfig::BLOCK_SIZE_BYTE]>(buffer);
    return block[blockIndex];
}

void*
BufferEntry::GetChunk(uint32_t chunkIndex, uint32_t blksPerChunk)
{
    if (unlikely(chunkIndex * blksPerChunk >= blkCnt))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::BUFFER_ENTRY_OUT_OF_RANGE,
            IbofEventId::GetString(IBOF_EVENT_ID::BUFFER_ENTRY_OUT_OF_RANGE));
        return nullptr;
    }

    assert(blksPerChunk * chunkIndex < blkCnt);
    uint8_t(*chunk)[blksPerChunk][ArrayConfig::BLOCK_SIZE_BYTE] =
        static_cast<uint8_t(*)[blksPerChunk][ArrayConfig::BLOCK_SIZE_BYTE]>(buffer);
    return chunk[chunkIndex];
}

} // namespace ibofos
