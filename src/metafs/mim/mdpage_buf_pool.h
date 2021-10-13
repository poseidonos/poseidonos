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

#include "metafs_config.h"
#include "metafs_type.h"
#include "src/include/memory.h"

namespace pos
{
class MDPageBufPool
{
public:
    explicit MDPageBufPool(uint32_t numBuf)
    : totalNumBuf(numBuf)
    {
        base = nullptr;
    }

    ~MDPageBufPool(void)
    {
        pos::Memory<MDPAGE_BUF_SIZE>::Free(base);
        mdPageBufList.clear();
    }

    void
    Init(void)
    {
        base = pos::Memory<MDPAGE_BUF_SIZE>::Alloc(totalNumBuf);
        assert(base != nullptr); // please check hugepage preallocation
        for (uint32_t bufIdx = 0; bufIdx < totalNumBuf; ++bufIdx)
        {
            mdPageBufList.push_back((uint8_t*)base + (MDPAGE_BUF_SIZE * bufIdx));
        }
    }

    void*
    PopNewBuf(void)
    {
        while (mdPageBufList.empty())
        {
            usleep(1);
        }
        void* newBuf = mdPageBufList.back();
        mdPageBufList.pop_back();

        return newBuf;
    }

    void
    FreeBuf(void* buf)
    {
        mdPageBufList.push_back(buf);
    }

    bool
    IsEmpty(void)
    {
        return mdPageBufList.empty();
    }

    size_t
    GetCount(void)
    {
        return mdPageBufList.size();
    }

private:
    uint32_t totalNumBuf;
    static const FileSizeType MDPAGE_BUF_SIZE = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
    void* base;
    std::vector<void*> mdPageBufList;
};
} // namespace pos
