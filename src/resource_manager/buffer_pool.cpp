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

#include "buffer_pool.h"

#include "src/dpdk_wrapper/hugepage_allocator.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

using namespace pos;
using namespace std;

BufferPool::BufferPool(const BufferInfo info,
    HugepageAllocator* hugepageAllocator)
: BUFFER_INFO(info),
  hugepageAllocator(hugepageAllocator)
{
    if (_Alloc() == false)
    {
        _Clear();
    }
}

BufferPool::~BufferPool(void)
{
    _Clear();
}

void
BufferPool::_Clear()
{
    for (void* mem : allocatedMemories)
    {
        hugepageAllocator->Free(mem);
    }
}

bool
BufferPool::_Alloc(void)
{
    const uint32_t ALLOCATION_SIZE_BYTE = 2 * 1024 * 1024; // 2MB

    uint32_t remainBufferCount = 0;
    uint8_t* buffer = 0;
    for (uint32_t i = 0; i < BUFFER_INFO.bufferCount; i++)
    {
        if (remainBufferCount == 0)
        {
            // 2MB allocation for avoiding buddy allocation overhead
            buffer = static_cast<uint8_t*>(hugepageAllocator->AllocFromSocket(
                ALLOCATION_SIZE_BYTE,
                1,
                BUFFER_INFO.socket));
            if (buffer == nullptr)
            {
                POS_EVENT_ID eventId =
                    POS_EVENT_ID::FREEBUFPOOL_FAIL_TO_ALLOCATE_MEMORY;
                POS_TRACE_ERROR(static_cast<uint32_t>(eventId),
                    PosEventId::GetString(eventId));
                return false;
            }
            remainBufferCount = ALLOCATION_SIZE_BYTE / BUFFER_INFO.bufferSize;
            allocatedMemories.push_back(buffer);
        }

        freeBuffers.push_back(buffer);
        totalBuffers.push_back(buffer);
        buffer += BUFFER_INFO.bufferSize;
        remainBufferCount--;
    }

    return true;
}

void*
BufferPool::GetBuffer(void)
{
    unique_lock<mutex> lock(freeBufferLock);

    if (freeBuffers.empty())
    {
        return nullptr;
    }

    void* buffer = nullptr;
    buffer = freeBuffers.front();
    freeBuffers.pop_front();

    return buffer;
}

void
BufferPool::ReturnBuffer(void* buffer)
{
    if (buffer == nullptr)
    {
        return;
    }

    unique_lock<mutex> lock(freeBufferLock);
    freeBuffers.push_back(buffer);
}
