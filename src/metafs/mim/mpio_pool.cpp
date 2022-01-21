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

#include "mpio_pool.h"

#include <vector>

#include "metafs_common.h"
#include "read_mpio.h"
#include "write_mpio.h"

namespace pos
{
MpioPool::MpioPool(const size_t poolSize)
: capacity_(poolSize)
{
    if (capacity_ == 0)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "Pool size requested is {}", capacity_);
        assert(false);
    }

    all_.reserve(capacity_ * (int)MpioType::Last);

    mdPageBufPool = std::make_shared<MDPageBufPool>(capacity_ * (uint32_t)MpioType::Max);
    mdPageBufPool->Init();

    for (int idx = (int)MpioType::First; idx <= (int)MpioType::Last; ++idx)
    {
        int numMpio = capacity_;
        while (numMpio-- != 0)
        {
            Mpio* mpio = nullptr;
            auto mdPageBuf = mdPageBufPool->PopNewBuf();
            assert(nullptr != mdPageBuf);

            if (MpioType::Read == (MpioType)idx)
                mpio = new ReadMpio(mdPageBuf);
            else
                mpio = new WriteMpio(mdPageBuf);

            all_.emplace_back(mpio);
            free_[idx].emplace_back(mpio);
        }
    }

#if MPIO_CACHE_EN
    _InitCache(capacity_);
#endif
}

MpioPool::~MpioPool(void)
{
#if MPIO_CACHE_EN
    _InitCache(capacity_);
#endif

    all_.clear();
    for (int idx = (int)MpioType::First; idx <= (int)MpioType::Last; ++idx)
    {
        _FreeAllMpioinPool((MpioType)idx);
        free_[idx].clear();
    }
}

Mpio*
MpioPool::TryAlloc(const MpioType mpioType, const MetaStorageType storageType,
        const MetaLpnType lpn, const bool partialIO, const int arrayId)
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
        mpio = _CacheHit(mpioType, lpn, arrayId);
        if (nullptr != mpio)
            return mpio;

        // delete the oldest
        if (true == _IsFullyCached())
            _CacheRemove(mpioType);

        // add new
        return _TryCacheAlloc(mpioType, lpn);
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
    mpio->StoreTimestamp(MpioTimestampStage::Release);

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
    free_[(uint32_t)mpio->GetType()].emplace_back(mpio);
}

void
MpioPool::_FreeAllMpioinPool(const MpioType type)
{
    for (auto itr : free_[(uint32_t)type])
    {
        delete itr;
    }
}

bool
MpioPool::IsEmpty(MpioType type)
{
    if (free_[(uint32_t)type].size() == 0)
    {
        return true;
    }
    return false;
}

Mpio*
MpioPool::_AllocMpio(const MpioType mpioType)
{
    const uint32_t type = (uint32_t)mpioType;
    if (0 == free_[type].size())
        return nullptr;

    auto mpio = free_[type].front();
    free_[type].pop_front();
    mpio->StoreTimestamp(MpioTimestampStage::Allocate);

    return mpio;
}

#if MPIO_CACHE_EN
void
MpioPool::_InitCache(uint32_t poolSize)
{
    maxCacheCount = MetaFsConfig::DEFAULT_MAX_MPIO_CACHE_COUNT;
    currentCacheCount = 0;

    for (auto& it : cachedMpio)
        free_[(int)MpioType::Write].push_back(it.second);

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
MpioPool::_CacheHit(MpioType mpioType, MetaLpnType lpn, int arrayId)
{
    auto range = cachedMpio.equal_range(lpn);
    for (multimap<MetaLpnType, Mpio*>::iterator iter = range.first; iter != range.second; ++iter)
    {
        if (arrayId == iter->second->io.arrayId)
        {
            return iter->second;
        }
    }

    return nullptr;
}

Mpio*
MpioPool::_TryCacheAlloc(MpioType mpioType, MetaLpnType lpn)
{
    const uint32_t type = (uint32_t)mpioType;
    if (0 == free_[type].size())
        return nullptr;

    Mpio* mpio = free_[type].back();
    free_[type].pop_back();
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
            break;
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
    if (0 == free_[(uint32_t)MpioType::Write].size())
        _CacheRemove(MpioType::Write);
}
#endif
} // namespace pos
