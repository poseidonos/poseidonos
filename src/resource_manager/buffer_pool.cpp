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
 *   OWNE   R OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
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
    uint32_t socket,
    HugepageAllocator* hugepageAllocator)
: BUFFER_INFO(info),
  SOCKET(socket),
  hugepageAllocator(hugepageAllocator)
{
    if (hugepageAllocator == nullptr)
    {
        POS_TRACE_ERROR(EID(RESOURCE_MANAGER_DEBUG_MSG),
            "Faild to get hugepageAllocator");
        return;
    }

    if (_Alloc() == false)
    {
        _Clear();
    }
    else
    {
        isAllocated = true;
    }
}

BufferPool::~BufferPool(void)
{
    _Clear();
}

void
BufferPool::_Clear()
{
    unique_lock<mutex> lock(freeBufferLock);
    freeBuffers.clear();
    freeBufferSize = 0;
    initSize = 0;
    while (allocatedHugepages.size() != 0)
    {
        void* mem = allocatedHugepages.front();
        if(mem != nullptr)
        {
            hugepageAllocator->Free(mem);
        }
        allocatedHugepages.pop_front();
    }
    isAllocated = false;
}

bool
BufferPool::_Alloc(void)
{
    unique_lock<mutex> lock(freeBufferLock);
    uint32_t remainBufferCount = 0;
    uint8_t* buffer = 0;
    // 2MB allocation for avoiding buddy allocation overhead
    uint64_t allocSize = hugepageAllocator->GetDefaultPageSize();
    uint32_t allocCount = 1;
    if (allocSize < BUFFER_INFO.size)
    {
        allocCount = BUFFER_INFO.size / allocSize + 1;
    }
    for (uint32_t i = 0; i < BUFFER_INFO.count; i++)
    {
        if (remainBufferCount == 0)
        {
            buffer = static_cast<uint8_t*>(
                hugepageAllocator->AllocFromSocket(allocSize, allocCount, this->SOCKET));

            if (buffer == nullptr)
            {
                POS_TRACE_WARN(EID(RESOURCE_MANAGER_DEBUG_MSG),
                    "Failed to allocated buffer for {}", BUFFER_INFO.owner);
                return false;
            }
            remainBufferCount = allocSize * allocCount / BUFFER_INFO.size;
            allocatedHugepages.push_back(buffer);
        }

        freeBuffers.push_back(buffer);
        buffer += BUFFER_INFO.size;
        remainBufferCount--;
    }
    freeBufferSize = initSize = freeBuffers.size();
    return true;
}

void*
BufferPool::TryGetBuffer(void)
{
    unique_lock<mutex> lock(freeBufferLock);

    if (freeBuffers.empty())
    {
        return nullptr;
    }

    void* buffer = nullptr;
    buffer = freeBuffers.front();
    freeBuffers.pop_front();
    freeBufferSize--;
    return buffer;
}

void
BufferPool::ReturnBuffer(void* buffer)
{
    if (buffer == nullptr)
    {
        POS_TRACE_WARN(EID(RESOURCE_MANAGER_DEBUG_MSG),
            "Failed to return buffer. Buffer is Null");
        return;
    }

    unique_lock<mutex> lock(freeBufferLock);
    freeBuffers.push_back(buffer);
    freeBufferSize++;
}
