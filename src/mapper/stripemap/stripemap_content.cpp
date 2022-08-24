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


#include "src/include/branch_prediction.h"
#include "src/mapper/stripemap/stripemap_content.h"
#include "src/mapper/map/map_io_handler.h"
#include <string>

namespace pos
{
StripeMapContent::StripeMapContent(Map* m, int mapId, MapperAddressInfo* addrInfo)
: MapContent(mapId, addrInfo, MetaFileType::SpecialPurposeMap)
{
    // only for UT
    fileName = "StripeMap.bin";
    map = m;
}

StripeMapContent::StripeMapContent(int mapId, MapperAddressInfo* addrInfo)
: MapContent(mapId, addrInfo, MetaFileType::SpecialPurposeMap)
{
    fileName = "StripeMap.bin";
}

int
StripeMapContent::InMemoryInit(uint64_t numEntries, uint64_t mpageSize)
{
    return Init(numEntries, sizeof(StripeAddr), mpageSize);
}

StripeAddr
StripeMapContent::GetEntry(StripeId vsid)
{
    uint32_t pageNr = vsid / entriesPerMpage;

    char* mpage = map->GetMpage(pageNr);

    if (unlikely(mpage == nullptr))
    {
        StripeAddr unmapped = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = UNMAP_STRIPE};
        return unmapped;
    }
    else
    {
        uint32_t entNr = vsid % entriesPerMpage;
        return ((StripeAddr*)mpage)[entNr];
    }
}

int
StripeMapContent::SetEntry(StripeId vsid, StripeAddr entry)
{
    uint32_t pageNr = vsid / entriesPerMpage;

    map->GetMpageLock(pageNr);
    char* mpage = map->GetMpage(pageNr);

    if (mpage == nullptr)
    {
        mpage = map->AllocateMpage(pageNr);
        if (unlikely(mpage == nullptr))
        {
            map->ReleaseMpageLock(pageNr);
            return ERRID(STRIPEMAP_SET_FAILURE);
        }
        mapHeader->SetMapAllocated(pageNr);
    }

    StripeAddr* mpageMap = (StripeAddr*)mpage;
    uint32_t entNr = vsid % entriesPerMpage;
    mpageMap[entNr] = entry;

    mapHeader->GetTouchedMpages()->SetBit(pageNr);

    map->ReleaseMpageLock(pageNr);
    return 0;
}

MpageList
StripeMapContent::GetDirtyPages(uint64_t start, uint64_t numEntries)
{
    assert(numEntries == 1);

    MpageList dirtyList;
    dirtyList.insert(start / entriesPerMpage);
    return dirtyList;
}

} // namespace pos
