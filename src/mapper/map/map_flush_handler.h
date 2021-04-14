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

#include "map_content.h"
#include "sequential_page_finder.h"
#include "src/journal_manager/checkpoint/checkpoint_handler.h"
#include "src/meta_file_intf/async_context.h"

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
    friend class MapIoHandlerTest;

public:
    MapIoHandler(Map* mapData, MapHeader* mapHeaderData);
    virtual ~MapIoHandler(void) = default;

    int LoadFromMFS(void);
    int SyncLoad(MetaFileIntf* fileFromLoad);
    int AsyncLoad(AsyncLoadCallBack& cb);
    int AsyncLoadEvent(AsyncLoadCallBack& cb);
    int StoreToMFS();
    int SyncStore(MetaFileIntf* fileToStore);

    void RegisterFile(MetaFileIntf* mapFile);
    int SaveHeader(void);

    int FlushDirtyPagesGiven(MpageList dirtyPages, EventSmartPtr callback);
    int FlushTouchedPages(EventSmartPtr callback);

protected:
    BitMap* touchedPages;

private:
    bool _IncreaseAsyncIoDonePageNum(int numPagesDone);
    void _ResetAsyncIoPageNum();
    int _IssueHeaderIO(MetaFsIoOpcode opType);
    int _IssueHeaderIO(MetaFsIoOpcode opType, MetaFileIntf* file);
    int _IssueMpageIO(MetaFsIoOpcode opType);
    int _IssueMpageIO(MetaFsIoOpcode opType, MetaFileIntf* fileToIo);
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
    int _Flush(SequentialPageFinder& sequentialPages);

    Map* map;                  // by Ctor()
    MapHeader* mapHeader;      // by Ctor()
    MetaFileIntf* file;        // by RegisterFile()
    char* mapHeaderTempBuffer; // by FlushDirtyPages(), AsyncLoad()
    MapIoStatus status;
    int numPagesToAsyncIo;
    int numPagesAsyncIoDone;
    int ioError; // indicates if there is an Async-IO error among mpages

    std::atomic<bool> flushInProgress; // under CheckPoint or FLUSH command

    std::mutex mpageAsyncIoCountLock;
    AsyncLoadCallBack loadFinishedCallBack;
    EventSmartPtr flushDoneCallBack;
};

} // namespace pos
