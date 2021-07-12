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

#include "vsamap_content.h"

#include "src/allocator_service/allocator_service.h"
#include "src/allocator/i_block_allocator.h"
#include "src/include/branch_prediction.h"

#include <string>

namespace pos
{

VSAMapContent::VSAMapContent(void)
: MapContent()
{
}

VSAMapContent::VSAMapContent(int mapId, std::string arrayName)
: MapContent(mapId)
{
    filename = "VSAMap." + std::to_string(mapId) + ".bin";
    this->arrayName = arrayName;
    totalBlks = 0;
}

int
VSAMapContent::Prepare(uint64_t blkCnt, int64_t volid)
{
    SetPageSize(arrayName);
    totalBlks = blkCnt;

    header.entriesPerMpage = header.mpageSize / sizeof(VirtualBlkAddr);
    uint64_t mpagesNeeded = DivideUp(totalBlks, header.entriesPerMpage);
    InitHeaderInfo(mpagesNeeded);

    return 0;
}

int
VSAMapContent::InMemoryInit(uint64_t blkCnt, uint64_t volid)
{
    totalBlks = blkCnt;
    uint64_t pages = DivideUp(totalBlks, header.entriesPerMpage);
    int ret = Init(pages);
    return ret;
}

VirtualBlkAddr
VSAMapContent::GetEntry(BlkAddr rba)
{
    uint64_t pageNr = rba / header.entriesPerMpage;

    char* mpage = map->GetMpage(pageNr);

    if (unlikely(nullptr == mpage))
    {
        return UNMAP_VSA;
    }
    else
    {
        uint64_t entNr = rba % header.entriesPerMpage;
        return ((VirtualBlkAddr*)mpage)[entNr];
    }
}

int
VSAMapContent::SetEntry(BlkAddr rba, VirtualBlkAddr vsa)
{
    uint64_t pageNr = rba / header.entriesPerMpage;

    map->GetMpageLock(pageNr);
    char* mpage = map->GetMpage(pageNr);

    if (mpage == nullptr)
    {
        mpage = map->AllocateMpage(pageNr);
        if (unlikely(mpage == nullptr))
        {
            map->ReleaseMpageLock(pageNr);
            return -(int)POS_EVENT_ID::VSAMAP_SET_FAILURE;
        }
        header.SetMapAllocated(pageNr);
    }

    VirtualBlkAddr* mpageMap = (VirtualBlkAddr*)mpage;

    uint64_t entNr = rba % header.entriesPerMpage;
    mpageMap[entNr] = vsa;

    header.touchedPages->SetBit(pageNr);
    _UpdateUsedBlkCnt(vsa);
    map->ReleaseMpageLock(pageNr);

    return 0;
}

void
VSAMapContent::ResetEntries(BlkAddr rba, uint64_t cnt)
{
    // TODO(jk.man.kim) to check length of page offset variables (rba is in 64 bits)
    uint64_t i;
    uint64_t startPageNr = rba / header.entriesPerMpage;
    uint64_t startOffset = rba % header.entriesPerMpage;
    uint64_t endPageNr = (rba + cnt) / header.entriesPerMpage;
    uint64_t endOffset = (rba + cnt) % header.entriesPerMpage;

    if (startOffset)
    {
        char* mpage = map->GetMpageWithLock(startPageNr);
        if (mpage == nullptr)
        {
            goto next;
        }
        for (i = startOffset; i < header.entriesPerMpage; i++)
        {
            ((VirtualBlkAddr*)mpage)[i] = UNMAP_VSA;
        }
    next:
        startPageNr++;
    }

    for (i = startPageNr; i < endPageNr; i++)
    {
        char* mpage = map->GetMpageWithLock(i);
        if (mpage == nullptr)
            continue;

        memset(mpage, 0xFF, header.mpageSize);
    }

    if (endOffset)
    {
        char* mpage = map->GetMpageWithLock(endPageNr);
        if (mpage == nullptr)
        {
            goto out;
        }
        for (i = 0; i < endOffset; i++)
        {
            ((VirtualBlkAddr*)mpage)[i] = UNMAP_VSA;
        }
    }
out:
    return;
}

MpageList
VSAMapContent::GetDirtyPages(BlkAddr start, uint64_t numEntries)
{
    uint64_t startPageNr = start / header.entriesPerMpage;
    uint64_t endPageNr = (start + numEntries) / header.entriesPerMpage;
    uint64_t endOffset = (start + numEntries) % header.entriesPerMpage;

    MpageList dirtyList;

    for (uint64_t pageNum = startPageNr; pageNum < endPageNr; ++pageNum)
    {
        dirtyList.insert(pageNum);
    }

    if (endOffset != 0)
    {
        dirtyList.insert(endPageNr);
    }

    return dirtyList;
}

int64_t
VSAMapContent::GetNumUsedBlocks(void)
{
    return header.usedBlkCnt;
}

int
VSAMapContent::InvalidateAllBlocks(void)
{
    uint32_t mpageId = 0;
    IBlockAllocator* iBlockAllocator = AllocatorServiceSingleton::Instance()->GetIBlockAllocator(arrayName);

    while ((mpageId = header.bitmap->FindFirstSet(mpageId)) != header.bitmap->GetNumBits())
    {
        map->GetMpageLock(mpageId);

        char* mpage = map->GetMpage(mpageId);

        for (uint32_t entryIdx = 0; entryIdx < header.entriesPerMpage; ++entryIdx)
        {
            VirtualBlkAddr vsa = ((VirtualBlkAddr*)mpage)[entryIdx];
            if (IsUnMapVsa(vsa) == false)
            {
                VirtualBlks vBlks = {.startVsa = vsa, .numBlks = 1};
                iBlockAllocator->InvalidateBlks(vBlks);
            }
        }

        map->ReleaseMpageLock(mpageId);
        mpageId++;
    }

    return 0;
}

void
VSAMapContent::_UpdateUsedBlkCnt(VirtualBlkAddr vsa)
{
    if (IsUnMapVsa(vsa))
    {
        header.usedBlkCnt--;
        if (header.usedBlkCnt < 0)
        {
            header.usedBlkCnt = 0;
        }
    }
    else
    {
        header.usedBlkCnt++;
    }
}

} // namespace pos
