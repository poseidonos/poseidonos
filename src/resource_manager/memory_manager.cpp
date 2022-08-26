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

#include "memory_manager.h"

#include "buffer_pool.h"
#include "buffer_pool_factory.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

using namespace pos;
using namespace std;

MemoryManager::MemoryManager(BufferPoolFactory* bufferPoolFactory,
    AffinityManager* affinityManager)
: bufferPoolFactory(bufferPoolFactory),
  affinityManager(affinityManager)
{
    if (this->bufferPoolFactory == nullptr)
    {
        this->bufferPoolFactory = new BufferPoolFactory();
    }
}

MemoryManager::~MemoryManager(void)
{
    for (BufferPool* pool : bufferPools)
    {
        delete pool;
    }
    bufferPools.clear();

    if (bufferPoolFactory != nullptr)
    {
        delete bufferPoolFactory;
    }
}

BufferPool*
MemoryManager::CreateBufferPool(BufferInfo& info, uint32_t socket)
{
    if (_CheckBufferPolicy(info, socket) == false)
    {
        return nullptr;
    }

    BufferPool* pool = bufferPoolFactory->Create(info, socket);
    if (pool != nullptr)
    {
        unique_lock<mutex> lock(bufferPoolsLock);
        bufferPools.push_back(pool);
    }
    else
    {
        POS_TRACE_WARN(EID(RESOURCE_MANAGER_DEBUG_MSG),
            "Failed to create BufferPool, owner={}, size={}, count={}",
            info.owner, info.size, info.count);
    }

    return pool;
}

bool
MemoryManager::DeleteBufferPool(BufferPool* poolToDelete)
{
    unique_lock<mutex> lock(bufferPoolsLock);
    uint32_t sizeAfterRemove = bufferPools.size() - 1;
    bufferPools.remove(poolToDelete);
    if (sizeAfterRemove != bufferPools.size())
    {
        return false;
    }
    delete poolToDelete;
    return true;
}

bool
MemoryManager::_CheckBufferPolicy(const BufferInfo& info, uint32_t& socket)
{
    if (info.owner == "")
    {
        POS_TRACE_WARN(EID(RESOURCE_MANAGER_DEBUG_MSG),
            "Illegal buffer policy. Owner is empty");
        return false;
    }

    if (info.size == 0)
    {
        POS_TRACE_WARN(EID(RESOURCE_MANAGER_DEBUG_MSG),
            "Illegal buffer policy. Buffer size is zero");
        return false;
    }

    const uint32_t MEMORY_ALIGN_SIZE_BYTE = 4096;
    if (info.size % MEMORY_ALIGN_SIZE_BYTE != 0)
    {
        POS_TRACE_WARN(EID(RESOURCE_MANAGER_DEBUG_MSG),
            "Illegal buffer policy. Buffer size is not aligned");
        return false;
    }

    if (socket == USE_DEFAULT_SOCKET)
    {
        socket = affinityManager->GetNumaIdFromCurrentThread();
    }
    else if (socket > affinityManager->GetNumaCount())
    {
        POS_TRACE_WARN(EID(RESOURCE_MANAGER_DEBUG_MSG),
            "Illegal buffer policy. Invalid socket");
        return false;
    }

    return true;
}
