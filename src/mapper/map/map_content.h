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

#include "src/mapper/include/mpage_info.h"
#include "src/mapper/map/map.h"
#include "src/mapper/map/map_header.h"
#include "src/metafs/include/metafs_service.h"

#include <string>

namespace pos
{
const int CREATE_FILE_IF_NOT_EXIST = 1;

using AsyncLoadCallBack = std::function<void(int)>;

class MapIoHandler;

class MapContent
{
public:
    MapContent(int mapId_, int arrayId_);
    virtual ~MapContent(void);

    virtual MpageList GetDirtyPages(BlkAddr start, uint64_t numEntries) = 0;

    int Init(uint64_t numEntries, int entrySize, int mpageSize);
    void Dispose(void);
    int Load(AsyncLoadCallBack& cb);

    int OpenMapFile(void);
    int DeleteMapFile(void);
    bool DoesFileExist(void);

    int FlushDirtyPagesGiven(MpageList dirtyPages, EventSmartPtr callback);
    int FlushTouchedPages(EventSmartPtr callback);
    int FlushHeader(EventSmartPtr callback);

    int Dump(std::string fileName);
    int DumpLoad(std::string fileName);

    int GetId(void);
    uint32_t GetEntriesPerPage(void);
    void SetMapHeader(MapHeader* mapHeader_) { mapHeader = mapHeader_; }
    void SetMap(Map* map_) { map = map_; }

protected:
    MapHeader* mapHeader;
    Map* map;
    MapIoHandler* mapIoHandler;
    std::string fileName;
    int mapId;
    uint32_t entriesPerMpage;
    int arrayId;
    bool isInitialized;
};

} // namespace pos
