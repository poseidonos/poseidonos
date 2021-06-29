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

#include "src/include/pos_event_id.h"
#include "src/lib/bitmap.h"
#include "src/logger/logger.h"
#include "src/meta_file_intf/mock_file_intf.h"
#ifndef IBOF_CONFIG_USE_MOCK_FS
#include "src/metafs/metafs_file_intf.h"
#endif

#include <vector>

namespace pos
{
// Header data to be stored in the map header
class MpageValidInfo
{
public:
    MpageValidInfo(void) : numValidMpages(UINT64_MAX), numTotalMpages(UINT64_MAX) {}
    uint64_t numValidMpages;
    uint64_t numTotalMpages;
};

class Mpage
{
public:
    Mpage(void) : data(nullptr), mpageNr(-1)
    {
    }

    char* data;
    int mpageNr;
    std::mutex lock;
};

class MapHeader
{
public:
    MapHeader(void);
    virtual ~MapHeader(void);

    void UpdateNumValidMpages(void);
    bool ApplyNumValidMpages(void);

    int CopyToBuffer(char* buffer);
    BitMap* GetBitmapFromTempBuffer(char* buffer);

    int SetSize(void);
    void SetMapAllocated(int pageNr);
    void SetMpageValidInfo(uint64_t numPages, uint64_t ValidPages);

    MpageValidInfo mpageData;
    BitMap* bitmap;

    std::mutex mpageHeaderLock;

    BitMap* touchedPages;

    bool isInitialized;
    uint32_t size;      // header size(MpageValidInfo + BitMap), aligned by mpageSize
    uint32_t mpageSize; // by MapContent::SetPageSize()
    int mapId;          // by MapContent::Ctor()
    uint32_t entriesPerMpage;
};

class Map
{
public:
    Map(int numPages, int pageSize);
    virtual ~Map(void);

    char* GetMpage(int pageNr);
    char* GetMpageWithLock(int pageNr);
    char* AllocateMpage(int pageNr);

    void GetMpageLock(int pageNr);
    void ReleaseMpageLock(int pageNr);

    Mpage* mPageArr;
    uint32_t pageSize;
    int numPages;
};

} // namespace pos
