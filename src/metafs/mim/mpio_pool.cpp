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
#include "mfs_common.h"
#include "read_mpio.h"
#include "write_mpio.h"

MpioPool::MpioPool(uint32_t poolSize)
{
    assert(poolSize != 0);
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "MpioPool Construct : poolsize={}", poolSize);

    this->poolSize = poolSize;

    mdPageBufPool = new MDPageBufPool(poolSize * (uint32_t)MpioType::Max);
    mdPageBufPool->Init();

    for (auto type : Enum<MpioType>())
    {
        int numMpio = poolSize;
        while (--numMpio > 0)
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
}
MpioPool::~MpioPool(void)
{
    delete mdPageBufPool;
    for (auto type : Enum<MpioType>())
    {
        _FreeAllMpioinPool(type);
        mpioList[(uint32_t)type].clear();
    }
}

Mpio*
MpioPool::Alloc(MpioType mpioType)
{
    const uint32_t type = (uint32_t)mpioType;
    Mpio* mpio = mpioList[type].back();
    mpioList[type].pop_back();
    assert(mpio != nullptr);

    return mpio;
}
void
MpioPool::Release(Mpio* mpio)
{
    const uint32_t type = (uint32_t)mpio->GetType();
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
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
