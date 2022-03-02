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

#include "src/mapper/vsamap/vsamap_content.h"

#include <string>

#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_segment_ctx.h"
#include "src/allocator_service/allocator_service.h"
#include "src/include/branch_prediction.h"
#include "src/io/frontend_io/flush_command_manager.h"

namespace pos
{
VSAMapContent::VSAMapContent(int mapId, MapperAddressInfo* addrInfo, FlushCmdManager* flm_, Map* map_, MapHeader* mapHeader_)
: MapContent(mapId, addrInfo),
  flushThreshold(0),
  internalFlushEnabled(false)
{
    fileName = "VSAMap." + std::to_string(mapId) + ".bin";
    totalBlks = 0;
    this->arrayId = addrInfo->GetArrayId();
    callback = nullptr;

    flushCmdManager = flm_;
    if (flushCmdManager == nullptr)
    {
        flushCmdManager = FlushCmdManagerSingleton::Instance();
    }
    flushThreshold = flushCmdManager->GetInternalFlushThreshold();
    internalFlushEnabled = flushCmdManager->IsInternalFlushEnabled();
    map = map_;
    mapHeader = mapHeader_;
}

VSAMapContent::VSAMapContent(int mapId, MapperAddressInfo* addrInfo)
: VSAMapContent(mapId, addrInfo, nullptr, nullptr, nullptr)
{
}

int
VSAMapContent::InMemoryInit(uint64_t volId, uint64_t blkCnt, uint64_t mpageSize)
{
    totalBlks = blkCnt;
    return Init(totalBlks, sizeof(VirtualBlkAddr), mpageSize);
}

VirtualBlkAddr
VSAMapContent::GetEntry(BlkAddr rba)
{
    uint64_t pageNr = rba / entriesPerMpage;

    char* mpage = map->GetMpage(pageNr);

    if (unlikely(nullptr == mpage))
    {
        return UNMAP_VSA;
    }
    else
    {
        uint64_t entNr = rba % entriesPerMpage;
        return ((VirtualBlkAddr*)mpage)[entNr];
    }
}

int
VSAMapContent::SetEntry(BlkAddr rba, VirtualBlkAddr vsa)
{
    uint64_t pageNr = rba / entriesPerMpage;

    map->GetMpageLock(pageNr);
    char* mpage = map->GetMpage(pageNr);

    if (mpage == nullptr)
    {
        mpage = map->AllocateMpage(pageNr);
        if (unlikely(mpage == nullptr))
        {
            map->ReleaseMpageLock(pageNr);
            return -EID(VSAMAP_SET_FAILURE);
        }
        mapHeader->SetMapAllocated(pageNr);
    }

    VirtualBlkAddr* mpageMap = (VirtualBlkAddr*)mpage;
    uint64_t entNr = rba % entriesPerMpage;

    VirtualBlkAddr oldVsa = mpageMap[entNr];
    mapHeader->UpdateNumUsedBlks(oldVsa);

    mpageMap[entNr] = vsa;

    mapHeader->SetTouchedMpageBit(pageNr);

    if (internalFlushEnabled == true)
    {
        uint32_t numBitsSet = mapHeader->GetNumTouchedMpagesSet();
        uint32_t totalNumBits =  mapHeader->GetNumTotalTouchedMpages();

        if (((HUNDRED_PERCENT * numBitsSet) / totalNumBits) > flushThreshold)
        {
            flushCmdManager->UpdateVSANewEntries(mapHeader->GetMapId(), arrayId);
        }
    }

    map->ReleaseMpageLock(pageNr);

    return 0;
}

MpageList
VSAMapContent::GetDirtyPages(uint64_t start, uint64_t numEntries)
{
    uint64_t startPageNr = start / entriesPerMpage;
    uint64_t endPageNr = (start + numEntries) / entriesPerMpage;
    uint64_t endOffset = (start + numEntries) % entriesPerMpage;

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
VSAMapContent::GetNumUsedBlks(void)
{
    return mapHeader->GetNumUsedBlks();
}

void
VSAMapContent::SetCallback(EventSmartPtr cb)
{
    callback = cb;
}

EventSmartPtr
VSAMapContent::GetCallback(void)
{
    return callback;
}

int
VSAMapContent::InvalidateAllBlocks(ISegmentCtx* segmentCtx)
{
    uint32_t mpageId = 0;

    while ((mpageId = mapHeader->GetMpageMap()->FindFirstSet(mpageId)) != mapHeader->GetMpageMap()->GetNumBits())
    {
        map->GetMpageLock(mpageId);

        char* mpage = map->GetMpage(mpageId);

        for (uint32_t entryIdx = 0; entryIdx < entriesPerMpage; ++entryIdx)
        {
            VirtualBlkAddr vsa = ((VirtualBlkAddr*)mpage)[entryIdx];
            if (IsUnMapVsa(vsa) == false)
            {
                VirtualBlks vBlks = {.startVsa = vsa, .numBlks = 1};
                segmentCtx->InvalidateBlks(vBlks);
            }
        }

        map->ReleaseMpageLock(mpageId);
        mpageId++;
    }

    return 0;
}

} // namespace pos
