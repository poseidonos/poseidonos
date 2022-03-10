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

#include "mpio_allocator.h"

#include <vector>

#include "metafs_common.h"
#include "read_mpio.h"
#include "write_mpio.h"

namespace pos
{
MpioAllocator::MpioAllocator(const size_t eachPoolSize)
: WRITE_CACHE_CAPACITY(MetaFsConfig::DEFAULT_MAX_MPIO_CACHE_COUNT)
{
    if (eachPoolSize == 0)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "Pool size requested is {}", eachPoolSize);
        assert(false);
    }

    // tuple of array id, meta lpn, and mpio
    writeCache_ = std::make_shared<FifoCache<int, MetaLpnType, Mpio*>>(WRITE_CACHE_CAPACITY);

    mdPageBufPool = std::make_shared<MDPageBufPool>(eachPoolSize * (uint32_t)MpioType::Max);
    mdPageBufPool->Init();

    for (int idx = (int)MpioType::First; idx <= (int)MpioType::Last; ++idx)
    {
        int numMpio = eachPoolSize;
        pool_[idx] = std::make_shared<MetaFsPool<Mpio*>>(eachPoolSize);
        while (numMpio-- != 0)
            pool_[idx]->AddToPool(_CreateMpio((MpioType)idx));
    }
}

MpioAllocator::~MpioAllocator(void)
{
}

Mpio*
MpioAllocator::TryAlloc(const MpioType mpioType, const MetaStorageType storageType,
        const MetaLpnType lpn, const bool partialIO, const int arrayId)
{
#if RANGE_OVERLAP_CHECK_EN
    Mpio* mpio = nullptr;

    if (!((MetaStorageType::SSD != storageType) &&
            (true == partialIO) &&
            (MpioType::Write == mpioType)))
    {
        mpio = _TryAlloc(mpioType);

        if (nullptr == mpio)
            _ReleaseCache();
    }
    else
    {
        // At this point, all IOs must be "partial writes to NVRAM".
        // We check the corresponding cache entry first in the hope of reusing it.
        mpio = writeCache_->Find({arrayId, lpn});
        if (nullptr != mpio)
        {
            mpio->PrintLog("[alloc-   hit]", arrayId, lpn);
            return mpio;
        }

        mpio = _TryAlloc(mpioType);
        if (mpio)
        {
            if (writeCache_->IsFull())
                _ReleaseCache();

            mpio->SetCacheState(MpioCacheState::FirstRead);
            assert(writeCache_->Push({arrayId, lpn}, mpio) == nullptr);
            mpio->PrintLog("[alloc-   new]", arrayId, lpn);
        }
        return mpio;
    }
#else
    Mpio* mpio = _TryAlloc(mpioType);
#endif

    return mpio;
}

void
MpioAllocator::Release(Mpio* mpio)
{
    if (MpioCacheState::Init != mpio->GetCacheState())
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "[Mpio][Release    ] cached mpio, not released. type={}, req.tagId={}, mpio_id={}, fileOffset={}, buffer={}",
            (int)mpio->GetType(), mpio->io.tagId, mpio->GetId(),
            mpio->io.startByteOffset, mpio->GetMDPageDataBuf());

        mpio->PrintLog("[release- cac]", mpio->io.arrayId, mpio->io.metaLpn);
        mpio->Reset();

        return;
    }

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Mpio][Release    ] type={}, req.tagId={}, mpio_id={}, fileOffset={}, buffer={}",
        (int)mpio->GetType(), mpio->io.tagId, mpio->GetId(),
        mpio->io.startByteOffset, mpio->GetMDPageDataBuf());

    mpio->PrintLog("[release- del]", mpio->io.arrayId, mpio->io.metaLpn);
    mpio->Reset();
    pool_[(uint32_t)mpio->GetType()]->Release(mpio);
}

Mpio*
MpioAllocator::_CreateMpio(MpioType type)
{
    auto mdPageBuf = mdPageBufPool->PopNewBuf();
    assert(nullptr != mdPageBuf);
    if (MpioType::Read == type)
        return new ReadMpio(mdPageBuf);
    return new WriteMpio(mdPageBuf);
}

Mpio*
MpioAllocator::_TryAlloc(const MpioType mpioType)
{
    const uint32_t type = (uint32_t)mpioType;
    if (0 == pool_[type]->GetFreeCount())
        return nullptr;

    auto mpio = pool_[type]->TryAlloc();
    return mpio;
}

void
MpioAllocator::TryReleaseTheOldestCache(void)
{
    if (0 == pool_[(uint32_t)MpioType::Write]->GetFreeCount())
        _ReleaseCache();
}

void
MpioAllocator::ReleaseAllCache(void)
{
    while (writeCache_->GetSize())
        _ReleaseCache();
}

void
MpioAllocator::_ReleaseCache(void)
{
    auto victim = writeCache_->PopTheOldest();
    if (!victim)
        return;

    victim->SetCacheState(MpioCacheState::Init);
    if (victim->GetCurrState() == MpAioState::First)
    {
        Release(victim);
    }
}
} // namespace pos
