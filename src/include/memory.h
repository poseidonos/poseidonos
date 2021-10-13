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

#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#if defined UNVME_BUILD
#include <rte_config.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_malloc.h>
#include <rte_mempool.h>
#endif

#define SZ_1KB (1 << 10)
#define SZ_1MB (1 << 20)
#define SZ_10MB (10 * SZ_1MB)
#define SZ_1GB (1 << 30)

namespace pos
{
static const std::size_t BLOCK_SIZE_SHIFT = 12;
static const std::size_t BLOCK_SIZE = 1 << BLOCK_SIZE_SHIFT;
static const std::size_t SECTOR_SIZE_SHIFT = 9;
static const std::size_t SECTOR_SIZE = 1 << SECTOR_SIZE_SHIFT;
static const std::size_t SECTORS_PER_BLOCK_SHIFT =
    BLOCK_SIZE_SHIFT - SECTOR_SIZE_SHIFT;
static const std::size_t SECTORS_PER_BLOCK = 1 << SECTORS_PER_BLOCK_SHIFT;

constexpr uint64_t
DivideUp(uint64_t v, uint64_t a)
{
    return (v + a - 1) / a;
}

constexpr uint64_t
Align(uint64_t v, uint64_t u)
{
    return u * DivideUp(v, u);
}

constexpr uint64_t
AlignDown(uint64_t v, uint64_t u)
{
    return (v / u) * u;
}

constexpr uint64_t
ChangeBlockToByte(uint64_t b)
{
    return b << BLOCK_SIZE_SHIFT;
}

constexpr uint64_t
ChangeByteToBlock(uint64_t b)
{
    return b >> BLOCK_SIZE_SHIFT;
}

constexpr uint64_t
ChangeBlockToSector(uint64_t b)
{
    return b << SECTORS_PER_BLOCK_SHIFT;
}

constexpr uint64_t
ChangeSectorToBlock(uint64_t s)
{
    return s >> SECTORS_PER_BLOCK_SHIFT;
}

constexpr uint64_t
ChangeByteToSector(uint64_t b)
{
    return b >> SECTOR_SIZE_SHIFT;
}

constexpr uint64_t
ChangeSectorToByte(uint64_t s)
{
    return s << SECTOR_SIZE_SHIFT;
}

constexpr uint64_t
GetSectorOffsetInBlock(uint64_t address)
{
    return address & (SECTORS_PER_BLOCK - 1);
}

constexpr uint64_t
GetByteOffsetInBlock(uint64_t address)
{
    return address & (BLOCK_SIZE - 1);
}

template<const std::size_t N = BLOCK_SIZE>
class Memory
{
public:
    static void*
    Alloc(const std::size_t cnt = 1)
    {
        void* ret = nullptr;
#if defined UNVME_BUILD
        ret = rte_malloc(nullptr, N * cnt, N);
#else
        int error = posix_memalign(&ret, N, N * cnt);
        if (0 != error)
        {
            ret = nullptr;
        }
        if (nullptr != ret)
        {
            std::memset(ret, 0, N * cnt);
        }
#endif
        return ret;
    }

    static void*
    AllocFromSocket(const std::size_t cnt, uint32_t socket)
    {
#if defined UNVME_BUILD
        void* ret = rte_malloc_socket(nullptr, N * cnt, N, socket);
        // best effort for another socket id to avoid memory allocation fail
        if (ret == nullptr)
        {
            ret = Alloc(cnt);
        }
        return ret;
#else
        return Alloc(cnt);
#endif
    }

    static void
    Free(void* addr)
    {
#if defined UNVME_BUILD
        rte_free(addr);
#else
        std::free(addr);
#endif
    }
};

} // namespace pos
