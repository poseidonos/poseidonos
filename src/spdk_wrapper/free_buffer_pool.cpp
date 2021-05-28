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

#include "free_buffer_pool.h"

#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

namespace pos
{

FreeBufferPool::FreeBufferPool(uint64_t maxBufferCount, uint32_t bufferSize, AffinityManager* affinityManager)
{
    uint32_t remainBufferCount = 0;
    uint8_t* buffer = 0;
    uint32_t socket = affinityManager->GetEventWorkerSocket();
    for (uint32_t i = 0; i < maxBufferCount; i++)
    {
        if (remainBufferCount == 0)
        {
            // 2MB allocation for avoiding buddy allocation overhead
            buffer =
                static_cast<uint8_t*>(Memory<ALLOCATION_SIZE_BYTE>::
                        AllocFromSocket(1, socket));
            if (unlikely(buffer == nullptr))
            {
                POS_EVENT_ID eventId =
                    POS_EVENT_ID::FREEBUFPOOL_FAIL_TO_ALLOCATE_MEMORY;
                POS_TRACE_ERROR(static_cast<uint32_t>(eventId),
                    PosEventId::GetString(eventId));
                return;
            }
            remainBufferCount = ALLOCATION_SIZE_BYTE / bufferSize;
            bufferHeadList.push_back(buffer);
        }
        freeList.push_back(buffer);
        buffer += bufferSize;
        remainBufferCount--;
    }
}

FreeBufferPool::~FreeBufferPool(void)
{
    for (auto buffer : bufferHeadList)
    {
        Memory<ALLOCATION_SIZE_BYTE>::Free(buffer);
    }
}

void*
FreeBufferPool::GetBuffer(void)
{
    std::unique_lock<std::mutex> lock(freeListLock);
    if (freeList.empty())
    {
        return nullptr;
    }
    else
    {
        void* buffer = freeList.front();
        freeList.pop_front();
        return buffer;
    }
}

void
FreeBufferPool::ReturnBuffer(void* buffer)
{
    if (unlikely(nullptr == buffer))
    {
        return;
    }
    std::unique_lock<std::mutex> lock(freeListLock);

    freeList.push_back(buffer);
}

} // namespace pos
