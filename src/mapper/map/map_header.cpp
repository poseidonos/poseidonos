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
#include "src/mapper/map/map_header.h"

namespace pos
{

MapHeader::MapHeader(BitMap* mPageMap_, BitMap* touchedMpages_, int mapId_, int numMpages_, int mpageSize_, int entriesPerPage_)
: mPageMap(mPageMap_),
  touchedMpages(touchedMpages_),
  mapId(mapId_),
  size(0),
  mpageSize(mpageSize_),
  entriesPerMpage(entriesPerPage_),
  usedBlkCnt(0),
  isInitialized(false)
{
    mpageData.numTotalMpages = numMpages_;
    mpageData.numValidMpages = 0;
}

MapHeader::MapHeader(int mapId_, int numMpages_, int mpageSize_, int entriesPerPage_)
: MapHeader(nullptr, nullptr, mapId_, numMpages_, mpageSize_, entriesPerPage_)
{
}

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

void
MapHeader::Init(uint64_t numMpages)
{
    mPageMap = new BitMap(numMpages);
    mPageMap->ResetBitmap();
    touchedMpages = new BitMap(numMpages);
    touchedMpages->ResetBitmap();
    SetSize();
    isInitialized = true;
}

int
MapHeader::SetSize(void)
{
    uint32_t curOffset = 0;

    curOffset += sizeof(mpageData);
    curOffset += sizeof(usedBlkCnt);
    curOffset += (mPageMap->GetNumEntry() * BITMAP_ENTRY_SIZE);

    size = Align(curOffset, mpageSize);

    return 0;
}

int
MapHeader::CopyToBuffer(char* buffer)
{
    int curOffset = 0;
    std::unique_lock<std::mutex> lock(mpageHeaderLock);

    mpageData.numValidMpages = mPageMap->GetNumBitsSet();
    mpageData.numTotalMpages = mPageMap->GetNumBits();

    memcpy(buffer, (void*)(&mpageData), sizeof(mpageData));
    curOffset += sizeof(mpageData);
    memcpy(buffer + curOffset, (void*)(&usedBlkCnt), sizeof(usedBlkCnt));
    curOffset += sizeof(usedBlkCnt);
    memcpy(buffer + curOffset, (void*)mPageMap->GetMapAddr(), mPageMap->GetNumEntry() * BITMAP_ENTRY_SIZE);

    return 0;
}

BitMap*
MapHeader::GetBitmapFromTempBuffer(char* buffer)
{
    MpageValidInfo* validInfo = reinterpret_cast<MpageValidInfo*>(buffer);
    int bitmapOffset = sizeof(mpageData) + sizeof(usedBlkCnt);

    BitMap* copiedBitmap = new BitMap(validInfo->numTotalMpages);
    copiedBitmap->SetNumBitsSet(validInfo->numValidMpages);
    memcpy(copiedBitmap->GetMapAddr(), buffer + bitmapOffset, copiedBitmap->GetNumEntry() * BITMAP_ENTRY_SIZE);

    return copiedBitmap;
}

void
MapHeader::UpdateNumValidMpages(void)
{
    mpageData.numValidMpages = mPageMap->GetNumBitsSet();
    mpageData.numTotalMpages = mPageMap->GetNumBits();
}

bool
MapHeader::ApplyNumValidMpages(void)
{
    return mPageMap->SetNumBitsSet(mpageData.numValidMpages);
}

void
MapHeader::SetMapAllocated(int pageNr)
{
    std::unique_lock<std::mutex> lock(mpageHeaderLock);
    mPageMap->SetBit(pageNr);
}

void
MapHeader::UpdateUsedBlkCnt(VirtualBlkAddr vsa)
{
    if (IsUnMapVsa(vsa))
    {
        usedBlkCnt--;
        if (usedBlkCnt < 0)
        {
            usedBlkCnt = 0;
        }
    }
    else
    {
        usedBlkCnt++;
    }
}

}   // namespace pos
