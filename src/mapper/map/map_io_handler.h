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

#include "src/journal_manager/checkpoint/checkpoint_handler.h"
#include "src/mapper/map/map_content.h"
#include "src/mapper/map/sequential_page_finder.h"
#include "src/meta_file_intf/async_context.h"
#include "src/meta_file_intf/meta_file_intf.h"

#include <string>

namespace pos
{
enum MapIoStatus
{
    IDLE,
    FLUSHING_STARTED,
    FLUSHING_HEADER,
    FLUSHING_DONE,
    FLUSHING_ERROR,

    LOADING_HEADER_STARTED,
    LOADING_HEADER_DONE,
    LOADING_DONE,
    LOADING_ERROR,
};

class MapFlushIoContext : public AsyncMetaFileIoCtx
{
public:
    MpageNum startMpage;
    int numMpages;
};

class MapIoHandler
{
public:
    MapIoHandler(MetaFileIntf* file, Map* mapData, MapHeader* mapHeaderData, int mapId, MapperAddressInfo* addrInfo_);
    MapIoHandler(Map* mapData, MapHeader* mapHeaderData, int mapId, MapperAddressInfo* addrInfo_);
    ~MapIoHandler(void);

    void Dispose(void);
    int OpenFile(std::string fileName, uint64_t fileSize);
    int DeleteFile(void);
    int Load(AsyncLoadCallBack& cb);
    bool DoesFileExist(void);

    int FlushDirtyPagesGiven(MpageList dirtyPages, EventSmartPtr callback);
    int FlushTouchedPages(EventSmartPtr callback);
    int FlushHeader(EventSmartPtr callback);
    int LoadForWBT(MetaFileIntf* fileFromLoad);
    int StoreForWBT(MetaFileIntf* fileToStore);

private:
    int _MakeFileReady(void);
    bool _IncreaseAsyncIoDonePageNum(int numPagesDone);
    void _ResetAsyncIoPageNum(void);
    void _HeaderFlushed(AsyncMetaFileIoCtx* ctx);
    void _MpageFlushed(AsyncMetaFileIoCtx* ctx);
    void _HeaderAsyncLoaded(AsyncMetaFileIoCtx* ctx);
    void _MpageAsyncLoaded(AsyncMetaFileIoCtx* ctx);
    bool _PrepareFlush(EventSmartPtr callback);
    void _CompleteFlush(void);
    void _RemoveCleanPages(MpageList& pages);
    void _GetTouchedPages(BitMap* validPages);
    int _FlushMpages(MpageNum startPage, int numPages);
    int _IssueFlush(char* buffer, MpageNum startMpage, int numMpages);
    int _IssueFlushHeader(void);
    int _Flush(SequentialPageFinder& sequentialPages);
    int _IssueHeaderIoByMockFs(MetaFsIoOpcode opType, MetaFileIntf* fileToIo, char* headerBuf);
    int _IssueMpageIoByMockFs(MetaFsIoOpcode opType, MetaFileIntf* fileToIo);

    BitMap* touchedPages;
    int mapId;
    Map* map;
    MapHeader* mapHeader;
    MetaFileIntf* file;
    char* mapHeaderTempBuffer;
    MapIoStatus status;
    std::atomic<int> numPagesToAsyncIo;
    std::atomic<int> numPagesAsyncIoDone;
    std::atomic<bool> flushInProgress;

    AsyncLoadCallBack loadFinishedCallBack;
    EventSmartPtr flushDoneCallBack;
    int ioError;
    MapperAddressInfo* addrInfo;
};

} // namespace pos
