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

#include "src/bio/data_buffer.h"

#include "src/include/pos_event_id.hpp"
#include "src/include/branch_prediction.h"
#include "src/include/memory.h"
#include "src/logger/logger.h"

namespace pos
{
DataBuffer::DataBuffer(uint64_t size, void* buffer)
: baseAddress(buffer),
  currentAddress(static_cast<uint8_t*>(buffer)),
  size(size),
  originalSize(size),
  ownership(false)
{
    if (size == 0)
    {
        return;
    }

    if (nullptr == baseAddress)
    {
        ownership = true;
        baseAddress = Memory<ALIGNMENT>::Alloc(size / ALIGNMENT);
        currentAddress = static_cast<uint8_t*>(baseAddress);
    }
}

// When dataBuffer is copied, ownership should not be copied.
// to avoid double free for base address.
DataBuffer::DataBuffer(const DataBuffer& dataBuffer)
: baseAddress(dataBuffer.baseAddress),
  currentAddress(dataBuffer.currentAddress),
  size(dataBuffer.size),
  originalSize(dataBuffer.size),
  ownership(false)
{
}

DataBuffer::~DataBuffer(void)
{
    if (ownership)
    {
        Memory<ALIGNMENT>::Free(baseAddress);
    }
}

void
DataBuffer::Free(void)
{
    if (unlikely(ownership))
    {
        POS_EVENT_ID eventId = EID(UBIO_FREE_UNALLOWED_BUFFER);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Cannot free unallowed data buffer");
        return;
    }

    Memory<ALIGNMENT>::Free(baseAddress);
}

void*
DataBuffer::GetAddress(uint64_t blockIndex, uint64_t sectorOffset) const
{
    if (unlikely(nullptr == baseAddress))
    {
        POS_TRACE_ERROR(EID(UBIO_REQUEST_NULL_BUFFER),
            "Requested buffer of Ubio is null");

        return baseAddress;
    }

    uint64_t shiftSize = ChangeBlockToByte(blockIndex) +
        ChangeSectorToByte(sectorOffset);

    if (unlikely(size <= shiftSize))
    {
        POS_TRACE_ERROR(EID(UBIO_REQUEST_OUT_RANGE),
            "Requested buffer of Ubio is out of range, size:{}, shift:{}, blkIdx:{}, offset:{}",
            size, shiftSize, blockIndex, sectorOffset);
        return baseAddress;
    }

    uint8_t* addressForReturn = currentAddress;
    addressForReturn += shiftSize;

    return addressForReturn;
}

void*
DataBuffer::GetBaseAddress(void) const
{
    return baseAddress;
}

uint64_t
DataBuffer::GetSize(void)
{
    return size;
}

uint64_t
DataBuffer::GetOriginalSize(void)
{
    return originalSize;
}

void
DataBuffer::Remove(uint64_t removalSize, bool removalFromTail)
{
    bool isOutRangeSize = (removalSize > size);

    if (unlikely(isOutRangeSize))
    {
        assert(false);
        POS_EVENT_ID eventId = EID(UBIO_WRONG_SPLIT_SIZE);
        POS_TRACE_ERROR(eventId, "Invalid size of Ubio split request");

        throw eventId;
    }

    if (removalFromTail == false)
    {
        currentAddress += removalSize;
    }

    size -= removalSize;
}
} // namespace pos
