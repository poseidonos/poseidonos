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
#include "src/metafs/include/metafs_service.h"
#include "src/mapper/map/map_content.h"
#include "src/mapper/map/map_io_handler.h"

#include <string>

namespace pos
{
MapContent::MapContent(int mapId_, int arrayId_)
: mapHeader(nullptr),
  map(nullptr),
  mapIoHandler(nullptr),
  mapId(mapId_),
  arrayId(arrayId_),
  isInitialized(false)
{
}

MapContent::~MapContent(void)
{
    Dispose();
}

int
MapContent::Init(uint64_t numEntries, int entrySize, int mpageSize)
{
    if (isInitialized == false)
    {
        isInitialized = true;
        int entriesPerPage = mpageSize / entrySize;
        int numMpages = DivideUp(numEntries, entriesPerPage);
        if (mapHeader == nullptr)
        {
            mapHeader = new MapHeader(mapId, numMpages, mpageSize, entriesPerPage);
            mapHeader->Init(numMpages);
        }
        if (map == nullptr)
        {
            map = new Map(numMpages, mpageSize);
        }
        if (mapIoHandler == nullptr)
        {
            mapIoHandler = new MapIoHandler(map, mapHeader, arrayId);
        }
    }
    return 0;
}

int
MapContent::OpenMapFile(void)
{
    uint64_t fileSize = mapHeader->GetSize() + mapHeader->GetMpageSize() * mapHeader->GetNumTotalMpages();
    int ret = mapIoHandler->OpenFile(fileName, fileSize);
    if (ret == EID(NEED_TO_INITIAL_STORE))
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "[Mapper] Need to Initial Store fileName:{}, arrayId:{}", fileName, arrayId);
    }
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MAPPER_SUCCESS), "[Mapper] failed to save Header fileName:{}, arrayId:{}", fileName, arrayId);
    }
    return ret;
}

void
MapContent::Dispose(void)
{
    if (isInitialized == true)
    {
        if (mapIoHandler != nullptr)
        {
            delete mapIoHandler;
            mapIoHandler = nullptr;
        }
        if (mapHeader != nullptr)
        {
            delete mapHeader;
            mapHeader = nullptr;
        }
        if (map != nullptr)
        {
            delete map;
            map = nullptr;
        }
        isInitialized = false;
    }
}

int
MapContent::Load(AsyncLoadCallBack& cb)
{
    int ret = mapIoHandler->Load(cb);
    return ret;
}

int
MapContent::FlushDirtyPagesGiven(MpageList dirtyPages, EventSmartPtr callback)
{
    return mapIoHandler->FlushDirtyPagesGiven(dirtyPages, callback);
}

int
MapContent::FlushTouchedPages(EventSmartPtr callback)
{
    return mapIoHandler->FlushTouchedPages(callback);
}

int
MapContent::FlushHeader(EventSmartPtr callback)
{
    return mapIoHandler->FlushHeader(callback);
}

int
MapContent::DeleteMapFile(void)
{
    return mapIoHandler->DeleteFile();
}

bool
MapContent::DoesFileExist(void)
{
    return mapIoHandler->DoesFileExist();
}

int
MapContent::Dump(std::string fname)
{
    MetaFileIntf* linuxFileToStore = new MockFileIntf(fname, arrayId);
    int ret = linuxFileToStore->Create(0);
    linuxFileToStore->Open();

    if (ret == 0)
    {
        ret = mapIoHandler->StoreForWBT(linuxFileToStore);
    }

    linuxFileToStore->Close();
    delete linuxFileToStore;
    return ret;
}

int
MapContent::DumpLoad(std::string fname)
{
    MetaFileIntf* linuxFileFromLoad = new MockFileIntf(fname, arrayId);
    int ret = linuxFileFromLoad->Open();

    if (ret == 0)
    {
        ret = mapIoHandler->LoadForWBT(linuxFileFromLoad);
        ret = mapIoHandler->StoreForWBT(nullptr);
    }

    linuxFileFromLoad->Close();
    delete linuxFileFromLoad;
    return ret;
}

int
MapContent::GetId(void)
{
    return mapHeader->GetMapId();
}

uint32_t
MapContent::GetEntriesPerPage(void)
{
    return mapHeader->GetEntriesPerMpage();
}

} // namespace pos
