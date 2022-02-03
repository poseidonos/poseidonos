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
: capacity_(poolSize),
  WRITE_CACHE_CAPACITY(MetaFsConfig::DEFAULT_MAX_MPIO_CACHE_COUNT)
{
    if (capacity_ == 0)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "Pool size requested is {}", capacity_);
        assert(false);
    }

    all_.reserve(capacity_ * (int)MpioType::Last);

    // tuple of array id, meta lpn, and mpio
    writeCache_ = std::make_shared<FifoCache<int, MetaLpnType, Mpio*>>(WRITE_CACHE_CAPACITY);

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
}

MpioPool::~MpioPool(void)
{
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
        mpio = _TryAllocMpio(mpioType);

        if (nullptr == mpio)
            _ReleaseCache();
    }
    else
    {
        // At this point, all IOs must be "partial writes to NVRAM".
        // We check the corresponding cache entry first in the hope of reusing it.
        mpio = writeCache_->Find({arrayId, lpn});
        if (nullptr != mpio)
            return mpio;

        mpio = _TryAllocMpio(mpioType);
        if (mpio)
        {
            if (writeCache_->IsFull())
                _ReleaseCache();

            mpio->SetCacheState(MpioCacheState::FirstRead);
            assert(writeCache_->Push({arrayId, lpn}, mpio) == nullptr);
        }
        return mpio;
    }
#else
    mpio = _TryAllocMpio(mpioType);
#endif
#else
    Mpio* mpio = _TryAllocMpio(mpioType);
#endif

    return mpio;
}

void
MpioPool::Release(Mpio* mpio)
{
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
MpioPool::_TryAllocMpio(const MpioType mpioType)
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
MpioPool::ReleaseCache(void)
{
    if (0 == free_[(uint32_t)MpioType::Write].size())
        _ReleaseCache();
}

void
MpioPool::_ReleaseCache(void)
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
#endif
} // namespace pos
