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
#include "src/metafs/config/metafs_config_manager.h"
#include "src/metafs/include/metafs_service.h"

namespace pos
{
MpioAllocator::MpioAllocator(MetaFsConfigManager* configManager)
: WRITE_CACHE_CAPACITY(configManager->GetWriteMpioCacheCapacity())
{
    const size_t poolSize = configManager->GetMpioPoolCapacity();
    const bool directAccessEnabled = configManager->IsDirectAccessEnabled();

    if (poolSize == 0)
    {
        POS_TRACE_ERROR(EID(MFS_INVALID_PARAMETER),
            "Pool size requested is {}", poolSize);
        assert(false);
    }

    // tuple of array id, meta lpn, and mpio
    writeCache_ = std::make_shared<FifoCache<int, MetaLpnType, Mpio*>>(WRITE_CACHE_CAPACITY);

    mdPageBufPool = std::make_shared<MDPageBufPool>(poolSize * (uint32_t)MpioType::Max);
    mdPageBufPool->Init();

    for (int idx = (int)MpioType::First; idx <= (int)MpioType::Last; ++idx)
    {
        int numMpio = poolSize;
        pool_[idx] = std::make_shared<MetaFsPool<Mpio*>>(poolSize);
        while (numMpio-- != 0)
            pool_[idx]->AddToPool(_CreateMpio((MpioType)idx, directAccessEnabled));
    }

    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "Mpio allocator constructed. mpio pool size: {}, write cache size: {}",
        poolSize, WRITE_CACHE_CAPACITY);
}

MpioAllocator::~MpioAllocator(void)
{
}

Mpio*
MpioAllocator::TryAlloc(const MpioType mpioType, const MetaStorageType storageType,
        const MetaLpnType lpn, const bool partialIO, const int arrayId)
{
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
        // At this point, all IOs must be "partial writes to NVRAM or Journal_SSD".
        // We check the corresponding cache entry first in the hope of reusing it.
        mpio = writeCache_->Find({arrayId, lpn});
        if (nullptr != mpio)
        {
            mpio->PrintLog("[alloc-   hit]", arrayId, lpn);
            return mpio;
        }

        // alloc new mpio, not cached
        mpio = _TryAlloc(mpioType);
        if (mpio)
        {
            if (writeCache_->IsFull())
                _ReleaseCache();

            // cached now
            mpio->ChangeCacheStateTo(MpioCacheState::Read);
            auto victim = writeCache_->Push({arrayId, lpn}, mpio);
            assert(victim == nullptr);
            mpio->PrintLog("[alloc-   new]", arrayId, lpn);
        }
        return mpio;
    }

    return mpio;
}

void
MpioAllocator::Release(Mpio* mpio)
{
    if (mpio->IsCached())
    {
        MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
            "[Mpio][Release    ] cached mpio, not released. type:{}, req.tagId:{}, mpio_id:{}, fileOffset:{}, buffer:{}",
            (int)mpio->GetType(), mpio->io.tagId, mpio->GetId(),
            mpio->io.startByteOffset, mpio->GetMDPageDataBuf());

        mpio->PrintLog("[release- cac]", mpio->io.arrayId, mpio->io.metaLpn);
    }
    else
    {
        MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
            "[Mpio][Release    ] type:{}, req.tagId:{}, mpio_id:{}, fileOffset:{}, buffer:{}",
            (int)mpio->GetType(), mpio->io.tagId, mpio->GetId(),
            mpio->io.startByteOffset, mpio->GetMDPageDataBuf());

        mpio->PrintLog("[release- del]", mpio->io.arrayId, mpio->io.metaLpn);
        pool_[(uint32_t)mpio->GetType()]->Release(mpio);
    }

    mpio->Reset();
}

Mpio*
MpioAllocator::_CreateMpio(const MpioType type, const bool directAccessEnabled)
{
    auto mdPageBuf = mdPageBufPool->PopNewBuf();
    assert(nullptr != mdPageBuf);
    if (MpioType::Read == type)
        return new ReadMpio(mdPageBuf, directAccessEnabled);
    return new WriteMpio(mdPageBuf, directAccessEnabled);
}

Mpio*
MpioAllocator::_TryAlloc(const MpioType type)
{
    const uint32_t mpioType = (uint32_t)type;
    if (0 == pool_[mpioType]->GetFreeCount())
        return nullptr;

    auto mpio = pool_[mpioType]->TryAlloc();
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

    victim->ChangeCacheStateTo(MpioCacheState::Init);
    if (victim->GetCurrState() == MpAioState::First)
    {
        Release(victim);
    }
}
} // namespace pos
