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

#include <vector>

#include "mdpage_buf_pool.h"
#include "mpio.h"
#include "os_header.h"
#include <list>
#include <map>
#include <string>

namespace pos
{
class MpioPool
{
public:
    explicit MpioPool(uint32_t poolSize);
    virtual ~MpioPool(void);

    Mpio* Alloc(MpioType mpioType, MetaStorageType storageType, MetaLpnType lpn,
                bool partialIO, int arrayId);
    virtual void Release(Mpio* mpio);
    virtual size_t GetPoolSize(void);

    bool IsEmpty(MpioType type);

#if MPIO_CACHE_EN
    virtual void ReleaseCache(void);
#endif

private:
    void _FreeAllMpioinPool(MpioType type);
    Mpio* _AllocMpio(MpioType mpioType);

#if MPIO_CACHE_EN
    void _InitCache(uint32_t poolSize);
    bool _IsFullyCached(void);
    bool _IsEmptyCached(void);
    Mpio* _CacheHit(MpioType mpioType, MetaLpnType lpn, int arrayId);
    Mpio* _CacheAlloc(MpioType mpioType, MetaLpnType lpn);
    void _CacheRemove(MpioType mpioType);
#endif

    MDPageBufPool* mdPageBufPool;
    std::vector<Mpio*> mpioList[(uint32_t)MpioType::Max];
    size_t poolSize;

#if MPIO_CACHE_EN
    size_t maxCacheCount;
    size_t currentCacheCount;
    std::list<Mpio*> cachedList;
    std::multimap<MetaLpnType, Mpio*> cachedMpio;
#endif
};
} // namespace pos
