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

#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include <list>
#include <mutex>

#include "buffer_info.h"
#include "src/lib/singleton.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/dpdk_wrapper/hugepage_allocator.h"

namespace pos
{
class BufferPool;
class BufferPoolFactory;

const uint32_t USE_DEFAULT_SOCKET = -1;
class MemoryManager
{
public:
    MemoryManager(BufferPoolFactory* bufferPoolFactory = nullptr,
        AffinityManager* affinityManager = AffinityManagerSingleton::Instance());
    virtual ~MemoryManager(void);
    virtual BufferPool* CreateBufferPool(BufferInfo& info,
        uint32_t socket = USE_DEFAULT_SOCKET);
    virtual bool DeleteBufferPool(BufferPool* pool);

private:
    bool _CheckBufferPolicy(const BufferInfo& info, uint32_t& socket);

    std::mutex bufferPoolsLock;
    std::list<BufferPool*> bufferPools;

    BufferPoolFactory* bufferPoolFactory;
    AffinityManager* affinityManager;
};

using MemoryManagerSingleton = Singleton<MemoryManager>;

} // namespace pos

#endif // MEMORY_MANAGER_H_
