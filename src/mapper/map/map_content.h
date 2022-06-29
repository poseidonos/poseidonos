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

#include "src/mapper/address/mapper_address_info.h"
#include "src/mapper/include/mpage_info.h"
#include "src/mapper/map/map.h"
#include "src/mapper/map/map_header.h"
#include "src/metafs/include/metafs_service.h"
#include "src/metafs/include/mf_property.h"

#include <string>

namespace pos
{
const int CREATE_FILE_IF_NOT_EXIST = 1;

using AsyncLoadCallBack = std::function<void(int)>;

class MapIoHandler;

class MapContent
{
public:
    MapContent(void) = default;
    MapContent(int mapId_, MapperAddressInfo* addrInfo, MetaFileType type = MetaFileType::Map);
    virtual ~MapContent(void);

    virtual MpageList GetDirtyPages(uint64_t start, uint64_t numEntries) = 0;

    virtual int Init(uint64_t numEntries, uint64_t entrySize, uint64_t mpageSize);
    virtual void Dispose(void);
    virtual int Load(AsyncLoadCallBack& cb);

    virtual int OpenMapFile(void);
    virtual int DeleteMapFile(void);
    virtual bool DoesFileExist(void);

    virtual int FlushDirtyPagesGiven(MpageList dirtyPages, EventSmartPtr callback);
    virtual int FlushTouchedPages(EventSmartPtr callback);
    virtual int FlushHeader(EventSmartPtr callback);

    virtual int Dump(std::string fileName);
    virtual int DumpLoad(std::string fileName);

    virtual uint64_t GetEntriesPerPage(void);

protected:
    MapHeader* mapHeader;
    Map* map;
    MapIoHandler* mapIoHandler;
    std::string fileName;
    MetaFileType fileType;
    int mapId;
    uint64_t entriesPerMpage;
    MapperAddressInfo* addrInfo;
    bool isInitialized;
};

} // namespace pos
