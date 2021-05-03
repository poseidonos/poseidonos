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

#include "mpio_pool.h"

#include <vector>

#include "enum_iterator.h"
#include "metafs_common.h"
#include "read_mpio.h"
#include "write_mpio.h"

namespace pos
{
MpioPool::MpioPool(uint32_t poolSize)
{
    assert(poolSize != 0);
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "MpioPool Construct : poolsize={}", poolSize);

    this->poolSize = poolSize;

    mdPageBufPool = new MDPageBufPool(poolSize * (uint32_t)MpioType::Max);
    mdPageBufPool->Init();

    for (auto type : Enum<MpioType>())
    {
        int numMpio = poolSize;
        while (numMpio-- != 0)
        {
            Mpio* mpio;
            void* mdPageBuf = mdPageBufPool->PopNewBuf();
            switch (type)
            {
                case MpioType::Read:
                {
                    mpio = new ReadMpio(mdPageBuf);
                }
                break;
                case MpioType::Write:
                {
                    mpio = new WriteMpio(mdPageBuf);
                }
                break;
                default:
                    assert(false);
            }
            mpio->InitStateHandler();
            mpioList[(uint32_t)type].push_back(mpio);
        }
    }

#if MPIO_CACHE_EN
    _InitCache(poolSize);
#endif
}

MpioPool::~MpioPool(void)
{
#if MPIO_CACHE_EN
    _InitCache(poolSize);
#endif

    delete mdPageBufPool;
    for (auto type : Enum<MpioType>())
    {
        _FreeAllMpioinPool(type);
        mpioList[(uint32_t)type].clear();
    }
}

Mpio*
MpioPool::Alloc(MpioType mpioType, MetaStorageType storageType, MetaLpnType lpn, bool partialIO, std::string arrayName)
{
#if RANGE_OVERLAP_CHECK_EN
    Mpio* mpio = nullptr;

#if MPIO_CACHE_EN
    if (!((MetaStorageType::NVRAM == storageType) &&
            (true == partialIO) &&
            (MpioType::Write == mpioType)))
    {
        mpio = _AllocMpio(mpioType);

        if (nullptr == mpio)
        {
            _CacheRemove(mpioType);
        }
    }
    else
    {
        // find mpio
        mpio = _CacheHit(mpioType, lpn, arrayName);
        if (nullptr != mpio)
            return mpio;

        // delete the oldest
        if (true == _IsFullyCached())
            _CacheRemove(mpioType);

        // add new
        return _CacheAlloc(mpioType, lpn, arrayName);
    }
#else
    mpio = _AllocMpio(mpioType);
#endif
#else
    Mpio* mpio = _AllocMpio(mpioType);
#endif

    return mpio;
}

void
MpioPool::Release(Mpio* mpio)
{
    const uint32_t type = (uint32_t)mpio->GetType();

#if MPIO_CACHE_EN
    if (MpioCacheState::Init != mpio->GetCacheState())
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "[Mpio][Release    ] cached mpio, not released. type={}, req.tagId={}, mpio_id={}, fileOffset={}, buffer={}",
            (int)mpio->GetType(), mpio->io.tagId, mpio->GetId(),
            mpio->io.startByteOffset, mpio->GetMDPageDataBuf());

        mpio->Reset();

        return;
    }
#endif

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Mpio][Release    ] type={}, req.tagId={}, mpio_id={}, fileOffset={}, buffer={}",
        (int)mpio->GetType(), mpio->io.tagId, mpio->GetId(),
        mpio->io.startByteOffset, mpio->GetMDPageDataBuf());

    mpio->Reset();

    mpioList[type].push_back(mpio);
}

size_t
MpioPool::GetPoolSize(void)
{
    return poolSize;
}

void
MpioPool::_FreeAllMpioinPool(MpioType type)
{
    std::vector<Mpio*>& pool = mpioList[(uint32_t)type];
    for (std::vector<Mpio*>::iterator itr = pool.begin(); itr != pool.end(); ++itr)
    {
        delete *itr;
        *itr = nullptr;
    }
}

bool
MpioPool::IsEmpty(void)
{
    for (auto type : Enum<MpioType>())
    {
        if (mpioList[(uint32_t)type].size() == 0)
        {
            return true;
        }
    }
    return false;
}

bool
MpioPool::IsEmpty(MpioType type)
{
    if (mpioList[(uint32_t)type].size() == 0)
    {
        return true;
    }
    return false;
}

Mpio*
MpioPool::_AllocMpio(MpioType mpioType)
{
    uint32_t type = (uint32_t)mpioType;
    if (0 == mpioList[type].size())
        return nullptr;

    Mpio* mpio = mpioList[type].back();
    mpioList[type].pop_back();

    return mpio;
}

#if MPIO_CACHE_EN
void
MpioPool::_InitCache(uint32_t poolSize)
{
    maxCacheCount = MetaFsConfig::DEFAULT_MAX_MPIO_CACHE_COUNT;
    currentCacheCount = 0;

    for (auto& it : cachedMpio)
    {
        mpioList[(int)MpioType::Write].push_back(it.second);
    }

    cachedMpio.clear();
    cachedList.clear();
}

bool
MpioPool::_IsFullyCached(void)
{
    return (maxCacheCount == currentCacheCount);
}

bool
MpioPool::_IsEmptyCached(void)
{
    return (0 == currentCacheCount);
}

Mpio*
MpioPool::_CacheHit(MpioType mpioType, MetaLpnType lpn, std::string arrayName)
{
    auto range = cachedMpio.equal_range(lpn);
    for (multimap<MetaLpnType, Mpio*>::iterator iter = range.first; iter != range.second; ++iter)
    {
        if (arrayName == iter->second->io.arrayName)
        {
            return iter->second;
        }
    }

    return nullptr;
}

Mpio*
MpioPool::_CacheAlloc(MpioType mpioType, MetaLpnType lpn, std::string arrayName)
{
    uint32_t type = (uint32_t)mpioType;
    if (0 == mpioList[type].size())
        return nullptr;

    Mpio* mpio = mpioList[type].back();
    mpioList[type].pop_back();
    mpio->SetCacheState(MpioCacheState::FirstRead);
    cachedMpio.insert(make_pair(lpn, mpio));
    cachedList.push_back(mpio);
    currentCacheCount++;

    return mpio;
}

void
MpioPool::_CacheRemove(MpioType mpioType)
{
    if (0 == cachedList.size())
        return;

    Mpio* mpio = cachedList.front();
    cachedList.pop_front();

    auto range = cachedMpio.equal_range(mpio->io.metaLpn);
    for (multimap<MetaLpnType, Mpio*>::iterator iter = range.first; iter != range.second; ++iter)
    {
        if (mpio == iter->second)
        {
            cachedMpio.erase(iter);
        }
    }

    mpio->SetCacheState(MpioCacheState::Init);
    if (mpio->GetCurrState() == MpAioState::First)
    {
        Release(mpio);
    }

    currentCacheCount--;
}

void
MpioPool::ReleaseCache(void)
{
    if (0 == mpioList[(uint32_t)MpioType::Write].size())
        _CacheRemove(MpioType::Write);
}
#endif
} // namespace pos
