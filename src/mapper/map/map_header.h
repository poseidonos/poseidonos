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

#pragma once

#include "src/include/address_type.h"
#include "src/lib/bitmap.h"

#include <atomic>

namespace pos
{

class MpageInfo
{
public:
    MpageInfo(void) : numValidMpages(UINT64_MAX), numTotalMpages(UINT64_MAX), numUsedBlks(0), reserved(0) {}
    uint64_t numValidMpages;
    uint64_t numTotalMpages;
    uint64_t numUsedBlks;
    uint64_t reserved;
};

class MapHeader
{
public:
    MapHeader(void);
    MapHeader(BitMap* mPageMap_, BitMap* touchedMpages_);
    virtual ~MapHeader(void);
    void Init(uint64_t numMpages, uint64_t mpageSize);

    int CopyToBuffer(char* buffer);
    BitMap* GetBitmapFromTempBuffer(char* buffer);

    uint64_t GetSize(void) { return size; }
    void ApplyHeader(char* srcBuf);
    uint64_t GetNumValidMpages(void) { return mPageMap->GetNumBitsSet(); }

    virtual BitMap* GetMpageMap(void) { return mPageMap; }
    virtual void SetMapAllocated(int pageNr);
    virtual BitMap* GetTouchedMpages(void) { return touchedMpages; }

    void UpdateNumUsedBlks(VirtualBlkAddr vsa);
    uint64_t GetNumUsedBlks(void) { return numUsedBlks; }

private:
    std::mutex mpageHeaderLock;

    int age;
    int size;
    std::atomic<uint64_t> numUsedBlks;
    BitMap* mPageMap;
    BitMap* touchedMpages;
};

}   // namespace pos
