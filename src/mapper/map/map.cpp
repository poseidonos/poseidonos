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

#include "src/include/memory.h"
#include "src/mapper/map/map.h"

namespace pos
{
Map::Map(void)
: memPool(nullptr),
  mPageArr(nullptr),
  pageSize(0),
  numPages(0)
{
}

Map::Map(uint64_t numMpages, uint64_t mpageSize)
{
    mPageArr = new Mpage[numMpages]();
    for (uint64_t mpage = 0; mpage < numMpages; ++mpage)
    {
        mPageArr[mpage].mpageNr = mpage;
    }

    pageSize = mpageSize;
    numPages = numMpages;
    uint64_t totalSize = pageSize * numPages;
    int ret = posix_memalign((void**)&memPool, pos::SECTOR_SIZE, totalSize);
    if (ret != 0)
    {
        POS_TRACE_ERROR(EID(MPAGE_MEMORY_ALLOC_FAILURE), "[Mapper Map] Failed to Allocate memPool size:{}, posix_memalign() failure, ret:{}", totalSize, ret);
        assert(false);
    }
}

Map::~Map(void)
{
    if (memPool != nullptr)
    {
        free(memPool);
        memPool = nullptr;
    }
    if (mPageArr != nullptr)
    {
        delete[] mPageArr;
        mPageArr = nullptr;
    }
}

char*
Map::AllocateMpage(uint64_t pageNr)
{
    assert(pageNr < numPages);
    if (mPageArr[pageNr].data != nullptr)
    {
        POS_TRACE_ERROR(EID(MPAGE_ALREADY_EXIST),
            "mpage exists but tried to allocate, pageNr:{} Mpage.data:{0:x}", pageNr,
            mPageArr[pageNr].data);
        return nullptr;
    }

    uint64_t memOffset = pageSize * pageNr;
    char* mpage = memPool + memOffset;
    memset(mpage, 0xFF, pageSize);
    mPageArr[pageNr].data = mpage;

    return mPageArr[pageNr].data;
}

void
Map::GetMpageLock(uint64_t pageNr)
{
    mPageArr[pageNr].lock.lock();
}

void
Map::ReleaseMpageLock(uint64_t pageNr)
{
    mPageArr[pageNr].lock.unlock();
}

char*
Map::GetMpage(uint64_t pageNr)
{
    if (pageNr >= numPages)
    {
        return nullptr;
    }
    return mPageArr[pageNr].data;
}

char*
Map::GetMpageWithLock(uint64_t pageNr)
{
    if (pageNr >= numPages)
    {
        return nullptr;
    }
    std::unique_lock<std::mutex> lock(mPageArr[pageNr].lock);
    return mPageArr[pageNr].data;
}

uint64_t
Map::GetSize(void)
{
    return pageSize;
}

uint64_t
Map::GetNumMpages(void)
{
    return numPages;
}

} // namespace pos
