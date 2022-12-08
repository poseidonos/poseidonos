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
#include <vector>
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
    virtual bool TryGetBuffers(uint32_t reqCnt, std::vector<void*>* retBuffers, uint32_t minAcqCnt);
    virtual void ReturnBuffer(void*);
    virtual void ReturnBuffers(std::vector<void*>* buffers);
    virtual bool IsAllocated(void) { return isAllocated; }
    std::string GetOwner(void) { return BUFFER_INFO.owner; }

private:
    bool _Init(void);
    void _Clear(void);
    void _TrySwapWhenConsumerPoolEmpty(void);
    void _TrySwapWhenProducerPoolIsNotEmpty(void);
    void _Swap(void);

    const BufferInfo BUFFER_INFO;
    const uint32_t SOCKET;

    HugepageAllocator* hugepageAllocator = nullptr;
    std::list<void*> allocatedHugepages;
    std::list<void*>* consumerPool = nullptr;
    std::list<void*>* producerPool = nullptr;
    std::list<void*> bufferList1;
    std::list<void*> bufferList2;

    std::mutex consumerLock;
    std::mutex producerLock;
    bool isAllocated = false;
    size_t swapThreshold = 0;
    const static size_t SWAP_THRESHOLD_PERCENT = 25;
};

} // namespace pos

#endif // BUFFER_POOL_H_
