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

#include <string>

#include "map.h"
#include "mpage_info.h"

class MapperTest;

namespace ibofos
{
const int CREATE_FILE_IF_NOT_EXIST = 1;

using AsyncLoadCallBack = std::function<void(int)>;

class MapIoHandler;

class MapContent
{
    friend class MapperTest;

public:
    explicit MapContent(int mapId);
    virtual ~MapContent(void);

    virtual int Prepare(uint64_t numEntries, int64_t opt = 0) = 0;
    virtual MpageList GetDirtyPages(BlkAddr start, uint64_t numEntries) = 0;

    int Init(uint64_t numPages);
    void InitHeaderInfo(uint64_t numPages);

    int CreateMapFile(void);
    int FileOpen(void);
    bool IsFileOpened(void);
    int LoadSync(int opt = 0);
    int LoadAsync(AsyncLoadCallBack& cb);
    int LoadAsyncEvent(AsyncLoadCallBack& cb);
    bool IsLoaded();

    int Unload(void);
    int StoreSync(void);
    int StoreAsync(void);
    int FileClose(void);
    int DeleteMapFile(void);
    bool DoesFileExist(void);

    // CheckPoint
    int StartDirtyPageFlush(MpageList dirtyPages, EventSmartPtr callback);
    // NVMe FLUSH command
    int Flush(EventSmartPtr callback);

    int Dump(std::string fileName);
    int DumpLoad(std::string fileName);

    int GetId(void);
    uint32_t GetPageSize(void);
    int SetPageSize(StorageOpt storageOpt = StorageOpt::DEFAULT);
    uint32_t GetEntriesPerPage(void);
    int GetNumMpageByLoadHeader(int volID, uint64_t& volSizeBytePageSizeAligned);
    std::string GetFileName(void);

protected:
    MapHeader header;           // by InitHeaderInfo()
    Map* map;                   // by Init()
    MapIoHandler* mapIoHandler; // by Init()
    MetaFileIntf* metaFile;     // assigned by CreateMapFile(), LoadSync(), LoadAsync()
    std::string filename;       // by Ctor() of derived class
    bool loaded;
};

} // namespace ibofos
