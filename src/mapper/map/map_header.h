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

#include "src/lib/bitmap.h"

namespace pos
{

class MpageValidInfo
{
public:
    MpageValidInfo(void) : numValidMpages(UINT64_MAX), numTotalMpages(UINT64_MAX) {}
    uint64_t numValidMpages;
    uint64_t numTotalMpages;
};

class MapHeader
{
public:
    MapHeader(BitMap* mPageMap_, BitMap* touchedMpages_);   // Ctor for IT
    MapHeader(void);    // Ctor for Production
    virtual ~MapHeader(void);
    void Init(uint64_t numMpages);

    int CopyToBuffer(char* buffer);
    BitMap* GetBitmapFromTempBuffer(char* buffer);

    void UpdateNumValidMpages(void);
    bool ApplyNumValidMpages(void);
    void* GetMpageDataAddr(void) { return &mpageData; }
    int GetSizeofMpageData(void) { return sizeof(mpageData); }
    uint64_t GetNumValidMpages(void) { return mpageData.numValidMpages; }
    uint64_t GetNumTotalMpages(void) { return mpageData.numTotalMpages; }
    void SetMpageValidInfo(uint64_t numPages, uint64_t ValidPages);

    virtual BitMap* GetMpageMap(void) { return mPageMap; }
    virtual void SetMapAllocated(int pageNr);
    virtual BitMap* GetTouchedMpages(void) { return touchedMpages; }

    int GetMapId(void) { return mapId; }
    void SetMapId(int mapId_) { mapId = mapId_; }

    uint32_t GetSize(void) { return size; }
    int SetSize(void);

    virtual uint32_t GetMpageSize(void) { return mpageSize; }
    virtual void SetMpageSize(uint32_t mpageSize_) { mpageSize = mpageSize_; }

    virtual uint32_t GetEntriesPerMpage(void) { return entriesPerMpage; }
    virtual void SetEntriesPerMpage(uint32_t entriesPerMpage_) { entriesPerMpage = entriesPerMpage_; }

    bool IsInitialized(void) { return isInitialized; }

private:
    std::mutex mpageHeaderLock;
    MpageValidInfo mpageData;
    BitMap* mPageMap;
    BitMap* touchedMpages;

    int mapId;              // by MapContent::Ctor()
    uint32_t size;          // header size(MpageValidInfo + BitMap), aligned by mpageSize
    uint32_t mpageSize;     // by MapContent::SetPageSize()
    uint32_t entriesPerMpage;
    bool isInitialized;
};

}   // namespace pos
