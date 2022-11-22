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

#include "src/include/memory.h"
#include "src/mapper/map/map_header.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{

MapHeader::MapHeader(BitMap* mPageMap_, BitMap* touchedMpages_, int mapId_)
: age(0),
  size(0),
  numUsedBlks(0),
  mPageMap(mPageMap_),
  touchedMpages(touchedMpages_),
  mapId(mapId_)
{
}

MapHeader::MapHeader(int mapId_)
: MapHeader(nullptr, nullptr, mapId_)
{
}
// LCOV_EXCL_START
MapHeader::~MapHeader(void)
{
    if (mPageMap != nullptr)
    {
        delete mPageMap;
        mPageMap = nullptr;
    }
    if (touchedMpages != nullptr)
    {
        delete touchedMpages;
        touchedMpages = nullptr;
    }
}
// LCOV_EXCL_STOP
void
MapHeader::Init(uint64_t numMpages, uint64_t mpageSize)
{
    mPageMap = new BitMap(numMpages);
    mPageMap->ResetBitmap();
    touchedMpages = new BitMap(numMpages);
    touchedMpages->ResetBitmap();

    size = sizeof(MpageInfo) + (mPageMap->GetNumEntry() * BITMAP_ENTRY_SIZE);
    size = Align(size, mpageSize);
}

int
MapHeader::CopyToBuffer(char* buffer)
{
    std::unique_lock<std::mutex> lock(mpageHeaderLock);
    MpageInfo* header = reinterpret_cast<MpageInfo*>(buffer);

    header->numValidMpages = mPageMap->GetNumBitsSet();
    header->numTotalMpages = mPageMap->GetNumBits();
    header->numUsedBlks = numUsedBlks;
    header->age = ++age;
    POS_TRACE_INFO(EID(COPY_MAP_HEADER), "mapId:{}, age:{}, numValidPgs:{}, numUsedBlks:{}", mapId, header->age, header->numValidMpages, header->numUsedBlks);
    memcpy((buffer + sizeof(MpageInfo)), (void*)mPageMap->GetMapAddr(), mPageMap->GetNumEntry() * BITMAP_ENTRY_SIZE);
    return 0;
}

BitMap*
MapHeader::GetBitmapFromTempBuffer(char* buffer)
{
    MpageInfo* info = reinterpret_cast<MpageInfo*>(buffer);
    int bitmapOffset = sizeof(MpageInfo);

    BitMap* copiedBitmap = new BitMap(info->numTotalMpages);
    copiedBitmap->SetNumBitsSet(info->numValidMpages);
    memcpy(copiedBitmap->GetMapAddr(), buffer + bitmapOffset, copiedBitmap->GetNumEntry() * BITMAP_ENTRY_SIZE);
    return copiedBitmap;
}

void
MapHeader::ApplyHeader(char* buffer)
{
    MpageInfo* header = reinterpret_cast<MpageInfo*>(buffer);
    numUsedBlks = header->numUsedBlks;
    age = header->age;
    mPageMap->SetNumBitsSet(header->numValidMpages);
    POS_TRACE_INFO(EID(MAPPER_INFO), "[Mapper MapHeader] Load, age:{}, numValidPgs:{}, numUsedBlks:{}", header->age, header->numUsedBlks, header->numValidMpages);
    memcpy(mPageMap->GetMapAddr(), buffer + sizeof(MpageInfo), mPageMap->GetNumEntry() * BITMAP_ENTRY_SIZE);
}

void
MapHeader::SetMapAllocated(int pageNr)
{
    std::unique_lock<std::mutex> lock(mpageHeaderLock);
    mPageMap->SetBit(pageNr);
}

void
MapHeader::UpdateNumUsedBlks(VirtualBlkAddr oldVsa)
{
    if (IsUnMapVsa(oldVsa) == true)
    {
        numUsedBlks++;
    }
}

uint32_t
MapHeader::GetNumTouchedMpagesSet(void)
{
    return touchedMpages->GetNumBitsSet();
}

uint32_t
MapHeader::GetNumTotalTouchedMpages(void)
{
    return touchedMpages->GetNumBits();
}

void
MapHeader::SetTouchedMpageBit(uint64_t pageNr)
{
    touchedMpages->SetBit(pageNr);
}

}   // namespace pos
