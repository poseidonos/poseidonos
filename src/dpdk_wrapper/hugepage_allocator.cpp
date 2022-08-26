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

#include "hugepage_allocator.h"

#include <rte_malloc.h>

#include "src/logger/logger.h"
#include "src/include/pos_event_id.hpp"

using namespace pos;

void*
HugepageAllocator::AllocFromSocket(const uint32_t size,
    const uint32_t count,
    const uint32_t socket)
{
    void* ret = rte_malloc_socket(nullptr, size * count, size, socket);
    // best effort for another socket id to avoid memory allocation fail
    if (ret == nullptr)
    {
        ret = rte_malloc(nullptr, size * count, size);
    }
    if (ret == nullptr)
    {
        POS_TRACE_WARN(EID(RESOURCE_HUGEPAGE_ALLOCATION_FAIL),
            "Failed to allocate Hugepages, size:{}, count:{}", size, count);
    }
    return ret;
}

void
HugepageAllocator::Free(void* addr)
{
    rte_free(addr);
    addr = nullptr;
}

uint32_t
HugepageAllocator::GetDefaultPageSize(void)
{
    return DEFAULT_PAGE_SIZE;
}

HugepageAllocator::~HugepageAllocator(void)
{
}
