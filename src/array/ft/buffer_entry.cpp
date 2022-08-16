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

#include "buffer_entry.h"

#include <cassert>

#include "src/include/array_config.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/resource_manager/buffer_pool.h"

namespace pos
{
void
BufferEntry::Reset(void)
{
    buffer = nullptr;
    blkCnt = 0;
    isParity = false;
    bufferPool = nullptr;
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
BufferEntry::SetBufferPool(BufferPool* inputBufferPool)
{
    bufferPool = inputBufferPool;
}

void
BufferEntry::ReturnBuffer(void)
{
    if (nullptr != bufferPool)
    {
        bufferPool->ReturnBuffer(buffer);
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
BufferEntry::GetBufferPtr(void) const
{
    return buffer;
}

uint32_t
BufferEntry::GetBlkCnt(void)
{
    return blkCnt;
}

void*
BufferEntry::GetBlock(uint32_t blockIndex)
{
    if (unlikely(blockIndex >= blkCnt))
    {
        POS_TRACE_ERROR(EID(BUFFER_ENTRY_OUT_OF_RANGE),
            "Block / Chunk Index excceds block count of BufferEntry");
        return nullptr;
    }

    uint8_t(*block)[ArrayConfig::BLOCK_SIZE_BYTE] =
        static_cast<uint8_t(*)[ArrayConfig::BLOCK_SIZE_BYTE]>(buffer);
    return block[blockIndex];
}

} // namespace pos
