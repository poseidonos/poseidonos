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
#include "src/metafs/lib/metafs_pool.h"
#include "src/metafs/common/fifo_cache.h"

namespace pos
{
class MetaFsConfigManager;

class MpioAllocator
{
public:
    explicit MpioAllocator(MetaFsConfigManager* configManager);
    virtual ~MpioAllocator(void);

    virtual Mpio* TryAlloc(const MpioType mpioType, const MetaStorageType storageType,
        const MetaLpnType lpn, const bool partialIO, const int arrayId);
    virtual void Release(Mpio* item);
    virtual size_t GetFreeCount(void) const
    {
        size_t total = 0;
        for (uint32_t type = 0; type < (uint32_t)MpioType::Last; ++type)
            pool_[(uint32_t)type]->GetFreeCount();
        return total;
    }
    virtual size_t GetFreeCount(const MpioType type) const
    {
        return pool_[(uint32_t)type]->GetFreeCount();
    }
    virtual size_t GetUsedCount(const MpioType type) const
    {
        return pool_[(uint32_t)type]->GetUsedCount();
    }
    virtual size_t GetCapacity(const MpioType type) const
    {
        return pool_[(uint32_t)type]->GetCapacity();
    }
    virtual bool IsEmpty(const MpioType type) const
    {
        return (0 == pool_[(uint32_t)type]->GetFreeCount());
    }
    virtual void TryReleaseTheOldestCache(void);
    virtual void ReleaseAllCache(void);

private:
    Mpio* _CreateMpio(const MpioType type, const bool directAccessEnabled);
    Mpio* _TryAlloc(const MpioType type);
    void _ReleaseCache(void);

    const size_t WRITE_CACHE_CAPACITY;
    std::shared_ptr<MDPageBufPool> mdPageBufPool;
    std::array<std::shared_ptr<MetaFsPool<Mpio*>>, (uint32_t)MpioType::Max> pool_;
    std::shared_ptr<FifoCache<int, MetaLpnType, Mpio*>> writeCache_;
};
} // namespace pos
