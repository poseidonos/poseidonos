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

#ifndef BUFFER_POOL_H_
#define BUFFER_POOL_H_

#include <list>
#include <mutex>
#include <string>

#include "buffer_info.h"
#include "src/dpdk_wrapper/hugepage_allocator.h"

namespace pos
{

class HugepageAllocator;

class BufferPool
{
public:
    BufferPool(const BufferInfo info,
        const uint32_t socket,
        HugepageAllocator* hugepageAllocator =
            HugepageAllocatorSingleton::Instance());
    virtual ~BufferPool(void);

    virtual void* TryGetBuffer(void);
    virtual void ReturnBuffer(void*);
    virtual bool IsFull(void) { return freeBufferSize == initSize; }
    virtual bool IsAllocated(void) { return isAllocated; }
    std::string GetOwner(void) { return BUFFER_INFO.owner; }
private:
    bool _Alloc(void);
    void _Clear(void);

    const BufferInfo BUFFER_INFO;
    const uint32_t SOCKET;

    std::mutex freeBufferLock;
    std::list<void*> freeBuffers;
    std::list<void*> allocatedHugepages;
    uint32_t freeBufferSize = 0;
    uint32_t initSize = 0;
    bool isAllocated = false;
    HugepageAllocator* hugepageAllocator;
};

} // namespace pos

#endif // BUFFER_POOL_H_
