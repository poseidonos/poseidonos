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

#include <array>
#include <list>
#include <string>
#include <vector>
#include <memory>

#include "mdpage_buf_pool.h"
#include "mpio.h"
#include "os_header.h"
#include "src/metafs/common/fifo_cache.h"

namespace pos
{
class MpioPool
{
public:
    explicit MpioPool(const size_t poolSize);
    virtual ~MpioPool(void);

    virtual Mpio* TryAlloc(const MpioType mpioType, const MetaStorageType storageType,
        const MetaLpnType lpn, const bool partialIO, const int arrayId);
    virtual void Release(Mpio* item);
    virtual size_t GetCapacity(void)
    {
        return capacity_;
    }
    virtual size_t GetFreeCount(void)
    {
        size_t total = 0;
        for (uint32_t mpioType = 0; mpioType < (uint32_t)MpioType::Max; ++mpioType)
            total += free_[(uint32_t)mpioType].size();
        return total;
    }
    virtual size_t GetFreeCount(const MpioType mpioType)
    {
        return free_[(uint32_t)mpioType].size();
    }
    virtual size_t GetUsedCount(const MpioType mpioType)
    {
        return capacity_ - GetFreeCount(mpioType);
    }

    bool IsEmpty(const MpioType type);

#if MPIO_CACHE_EN
    virtual void ReleaseCache(void);
#endif

private:
    void _FreeAllMpioinPool(const MpioType type);
    Mpio* _TryAllocMpio(const MpioType mpioType);
    void _ReleaseCache(void);

    std::shared_ptr<MDPageBufPool> mdPageBufPool;
    std::array<std::list<Mpio*>, (uint32_t)MpioType::Max> free_;
    std::vector<Mpio*> all_;
    size_t capacity_;
    const size_t WRITE_CACHE_CAPACITY;
    std::shared_ptr<FifoCache<int, MetaLpnType, Mpio*>> writeCache_;
};
} // namespace pos
