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

#include "src/include/address_type.h"
#include "src/lib/bitmap.h"

#include <atomic>

namespace pos
{

class MpageInfo
{
public:
    MpageInfo(void) : numValidMpages(UINT64_MAX), numTotalMpages(UINT64_MAX), numUsedBlks(0), age(0) {}
    uint64_t numValidMpages;
    uint64_t numTotalMpages;
    uint64_t numUsedBlks;
    uint64_t age;
};

class MapHeader
{
public:
    explicit MapHeader(int mapId);
    MapHeader(BitMap* mPageMap_, BitMap* touchedMpages_, int mapId_);
    virtual ~MapHeader(void);
    virtual void Init(uint64_t numMpages, uint64_t mpageSize);

    virtual int CopyToBuffer(char* buffer);
    virtual BitMap* GetBitmapFromTempBuffer(char* buffer);

    virtual uint64_t GetSize(void) { return size; }
    virtual void ApplyHeader(char* srcBuf);
    virtual uint64_t GetNumValidMpages(void) { return mPageMap->GetNumBitsSet(); }

    virtual BitMap* GetMpageMap(void) { return mPageMap; }
    virtual void SetMapAllocated(int pageNr);
    virtual BitMap* GetTouchedMpages(void) { return touchedMpages; }

    virtual void UpdateNumUsedBlks(VirtualBlkAddr vsa);
    virtual uint64_t GetNumUsedBlks(void) { return numUsedBlks; }

    virtual int GetMapId(void) { return mapId; }
    virtual uint32_t GetNumTouchedMpagesSet(void);
    virtual uint32_t GetNumTotalTouchedMpages(void);
    virtual void SetTouchedMpageBit(uint64_t pageNr);

private:
    std::mutex mpageHeaderLock;

    uint64_t age;
    uint64_t size;
    std::atomic<uint64_t> numUsedBlks;
    BitMap* mPageMap;
    BitMap* touchedMpages;

    int mapId;
};

}   // namespace pos
