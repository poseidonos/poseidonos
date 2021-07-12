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

#include "src/event_scheduler/event_scheduler.h"
#include "src/logger/logger.h"
#include "src/mapper/i_map_flush.h"
#include "src/mapper/map/event_mpage_async_io.h"
#include "src/mapper/map/map_flush_handler.h"

namespace pos
{
MapIoHandler::MapIoHandler(Map* mapData, MapHeader* mapHeaderData)
: touchedPages(nullptr),
  map(mapData),
  mapHeader(mapHeaderData),
  file(nullptr),
  mapHeaderTempBuffer(nullptr),
  status(IDLE),
  numPagesToAsyncIo(0),
  numPagesAsyncIoDone(0),
  ioError(0),
  flushInProgress(false)
{
}

int
MapIoHandler::LoadFromMFS(void)
{
    return SyncLoad(file);
}

int
MapIoHandler::SyncLoad(MetaFileIntf* fileFromLoad)
{
    int ret = _IssueHeaderIO(MetaFsIoOpcode::Read, fileFromLoad);
    if (ret == 0)
    {
        mapHeader->ApplyNumValidMpages();
        POS_TRACE_INFO(EID(MAP_LOAD_ONGOING),
            "fileName:{} header load success, valid:{} / total:{}",
            fileFromLoad->GetFileName(), mapHeader->mpageData.numValidMpages, mapHeader->mpageData.numTotalMpages);
        ret = _IssueMpageIO(MetaFsIoOpcode::Read, fileFromLoad);
    }
    return ret;
}

// Executed by CLI thread
int
MapIoHandler::AsyncLoad(AsyncLoadCallBack& cb)
{
    // MapHeader Sync-Load
    status = LOADING_HEADER_STARTED;
    POS_TRACE_INFO(EID(MAP_LOAD_STARTED), "Header Async Loading Started, mapId:{}", mapHeader->mapId);
    int ret = _IssueHeaderIO(MetaFsIoOpcode::Read);
    if (ret < 0)
    {
        return ret;
    }
    mapHeader->ApplyNumValidMpages();
    status = LOADING_HEADER_DONE;
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "fileName:{} Header Sync Load Success, Valid:{} / Total:{}",
                    file->GetFileName(), mapHeader->mpageData.numValidMpages, mapHeader->mpageData.numTotalMpages);

    loadFinishedCallBack = cb;
    if (0 == mapHeader->mpageData.numValidMpages)
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "There is no mpage to load, 'numValidMpage == 0'");
        return -EID(MAP_LOAD_COMPLETED);
    }

    // Mpages Async-Load
    uint64_t fileOffset = mapHeader->size;
    MpageNum mpageNum = 0;
    uint64_t numBitsSet = mapHeader->bitmap->GetNumBitsSet();
    numPagesToAsyncIo = numBitsSet;
    numPagesAsyncIoDone = 0;

    for (uint64_t cnt = 0; cnt < numBitsSet; ++cnt)
    {
        mpageNum = mapHeader->bitmap->FindFirstSet(mpageNum);
        char* mpage = map->AllocateMpage(mpageNum);

        MapFlushIoContext* mPageLoadRequest = new MapFlushIoContext();
        mPageLoadRequest->opcode = MetaFsIoOpcode::Read;
        mPageLoadRequest->fd = file->GetFd();
        mPageLoadRequest->fileOffset = fileOffset + (mpageNum * mapHeader->mpageSize);
        mPageLoadRequest->length = mapHeader->mpageSize;
        mPageLoadRequest->buffer = mpage;
        mPageLoadRequest->callback = std::bind(&MapIoHandler::_MpageAsyncLoaded, this, std::placeholders::_1);
        mPageLoadRequest->startMpage = mpageNum;
        mPageLoadRequest->numMpages = 1;

        ret = file->AsyncIO(mPageLoadRequest);
        if (ret < 0)
        {
            return ret;
        }
        mpageNum++;
    }

    return ret;
}

// Executed by EventWorker thread
int
MapIoHandler::AsyncLoadEvent(AsyncLoadCallBack& cb)
{
    status = LOADING_HEADER_STARTED;
    POS_TRACE_INFO(EID(MAP_LOAD_STARTED), "Header Async Loading Started, mapId:{}", mapHeader->mapId);

    loadFinishedCallBack = cb;
    uint64_t lenToRead = mapHeader->size;
    mapHeaderTempBuffer = new char[lenToRead]();

    MapFlushIoContext* headerLoadRequest = new MapFlushIoContext();
    headerLoadRequest->opcode = MetaFsIoOpcode::Read;
    headerLoadRequest->fd = file->GetFd();
    headerLoadRequest->fileOffset = 0;
    headerLoadRequest->length = lenToRead;
    headerLoadRequest->buffer = mapHeaderTempBuffer;
    headerLoadRequest->callback = std::bind(&MapIoHandler::_HeaderAsyncLoaded, this, std::placeholders::_1);

    ioError = 0;
    int ret = file->AsyncIO(headerLoadRequest);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS returned error on Header async loading, mapId:{}", mapHeader->mapId);
    }

    return ret;
}

int
MapIoHandler::StoreToMFS(void)
{
    return SyncStore(file);
}

int
MapIoHandler::SyncStore(MetaFileIntf* fileToStore)
{
    // mpage store
    mapHeader->UpdateNumValidMpages();
    int ret = _IssueMpageIO(MetaFsIoOpcode::Write, fileToStore);
    if (ret == 0)
    {
        // header store
        ret = _IssueHeaderIO(MetaFsIoOpcode::Write, fileToStore);
    }

    return ret;
}

void
MapIoHandler::RegisterFile(MetaFileIntf* mapFile)
{
    file = mapFile;
}

int
MapIoHandler::SaveHeader(void)
{
    return _IssueHeaderIO(MetaFsIoOpcode::Write);
}

int
MapIoHandler::FlushDirtyPagesGiven(MpageList dirtyPages, EventSmartPtr callback)
{
    if (_PrepareFlush(callback) == false)
    {
        return -EID(MAP_FLUSH_IN_PROGRESS);
    }

    int result = EID(SUCCESS);
    _RemoveCleanPages(dirtyPages);
    numPagesToAsyncIo = dirtyPages.size();

    if (numPagesToAsyncIo != 0)
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_STARTED), "FlushDirtyPagesGiven mapId:{} started", mapHeader->mapId);

        SequentialPageFinder sequentialPages(dirtyPages);
        result = _Flush(sequentialPages);
    }
    else
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_STARTED), "FlushDirtyPagesGiven mapId:{} completed, W/O any pages to flush", mapHeader->mapId);
        _CompleteFlush();
    }
    return result;
}

int
MapIoHandler::FlushTouchedPages(EventSmartPtr callback)
{
    if (_PrepareFlush(callback) == false)
    {
        return -EID(MAP_FLUSH_IN_PROGRESS);
    }

    int result = EID(SUCCESS);
    BitMap* copiedBitmap = mapHeader->GetBitmapFromTempBuffer(mapHeaderTempBuffer);
    _GetTouchedPages(copiedBitmap);
    delete copiedBitmap;

    if (touchedPages->GetNumBitsSet() != 0)
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_STARTED), "FlushTouchedPages mapId:{} started", mapHeader->mapId);
        numPagesToAsyncIo = touchedPages->GetNumBitsSet();
        SequentialPageFinder finder(touchedPages);
        result = _Flush(finder);
    }
    else
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_COMPLETED), "mapId:{} FlushTouchedPages completed, W/O any pages to flush", mapHeader->mapId);
        _CompleteFlush();
        // TODO(r.saraf) Handle callback returning false value (callback executed in _CompleteFlush)
    }

    return result;
}


// Executed by meta-fs thread
void
MapIoHandler::_HeaderAsyncLoaded(AsyncMetaFileIoCtx* ctx)
{
    MapFlushIoContext* headerLoadReqCtx = static_cast<MapFlushIoContext*>(ctx);
    if (headerLoadReqCtx->error < 0)
    {
        ioError = headerLoadReqCtx->error;
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO error, ioError:{}", ioError);
    }

    int bufferOffset = 0;
    memcpy(&mapHeader->mpageData, mapHeaderTempBuffer + bufferOffset, sizeof(mapHeader->mpageData));
    bufferOffset += sizeof(mapHeader->mpageData);
    memcpy(&mapHeader->usedBlkCnt, mapHeaderTempBuffer + bufferOffset, sizeof(mapHeader->usedBlkCnt));
    bufferOffset += sizeof(mapHeader->usedBlkCnt);
    memcpy(mapHeader->bitmap->GetMapAddr(), mapHeaderTempBuffer + bufferOffset, mapHeader->bitmap->GetNumEntry() * BITMAP_ENTRY_SIZE);

    mapHeader->ApplyNumValidMpages();
    status = LOADING_HEADER_DONE;
    POS_TRACE_INFO(EID(MAPPER_SUCCESS),
        "fileName:{} Header Sync Load Success, Valid:{} / Total:{}",
        file->GetFileName(), mapHeader->mpageData.numValidMpages, mapHeader->mpageData.numTotalMpages);

    if (0 == mapHeader->mpageData.numValidMpages)
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "There is no mpage to load, 'numValidMpage == 0'");
        loadFinishedCallBack(mapHeader->mapId);
        delete[] headerLoadReqCtx->buffer;
        delete headerLoadReqCtx;
        return;
    }

    // Mpages Async-load Request by Event
    numPagesToAsyncIo = mapHeader->bitmap->GetNumBitsSet();
    numPagesAsyncIoDone = 0;
    MetaIoCbPtr mpageAsyncLoadReqCB = std::bind(&MapIoHandler::_MpageAsyncLoaded, this, std::placeholders::_1);

    EventSmartPtr mpageLoadRequest = std::make_shared<EventMpageAsyncIo>(mapHeader, map, file, mpageAsyncLoadReqCB);
    EventSchedulerSingleton::Instance()->EnqueueEvent(mpageLoadRequest);

    delete[] headerLoadReqCtx->buffer;
    delete headerLoadReqCtx;
}

// Executed by meta-fs thread
void
MapIoHandler::_MpageAsyncLoaded(AsyncMetaFileIoCtx* ctx)
{
    MapFlushIoContext* mPageLoadReqCtx = static_cast<MapFlushIoContext*>(ctx);
    MpageNum startMpage = mPageLoadReqCtx->startMpage;
    int numMpages = mPageLoadReqCtx->numMpages;
    if (mPageLoadReqCtx->error < 0)
    {
        ioError = mPageLoadReqCtx->error;
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO error, ioError:{}  startMpage:{} numMpages:{}",
                        ioError, startMpage, numMpages);
    }

    POS_TRACE_DEBUG(EID(MAP_LOAD_ONGOING), "mapId:{}  startMpage:{} numMpages:{} async load completed ",
                    mapHeader->mapId, startMpage, numMpages);

    bool loadCompleted = _IncreaseAsyncIoDonePageNum(numMpages);
    if (loadCompleted)
    {
        _ResetAsyncIoPageNum();
        if (ioError != 0)
        {
            status = LOADING_ERROR;
            POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO error during _MpageAsyncLoaded mapId:{}, CHECK ABOVE LOGS!", mapHeader->mapId);
        }
        else
        {
            // MapHeader & mpages loading done
            status = LOADING_DONE;
            POS_TRACE_INFO(EID(MAP_LOAD_COMPLETED), "mapId:{} async load completed", mapHeader->mapId);
            loadFinishedCallBack(mapHeader->mapId);
        }
    }

    delete mPageLoadReqCtx;
}

int
MapIoHandler::_IssueHeaderIO(MetaFsIoOpcode opType)
{
    return _IssueHeaderIO(opType, file);
}

int
MapIoHandler::_IssueHeaderIO(MetaFsIoOpcode opType, MetaFileIntf* inputFile)
{
    uint64_t curOffset = 0;
    int ret = 0;

    ret = inputFile->AppendIO(opType, curOffset, sizeof(mapHeader->mpageData), (char*)&mapHeader->mpageData);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MFS_SYNCIO_ERROR), "AppendIO Error, retMFS:{}  fd:{}", ret, inputFile->GetFd());
        return ret;
    }

    ret = inputFile->AppendIO(opType, curOffset, sizeof(mapHeader->usedBlkCnt), (char*)&mapHeader->usedBlkCnt);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MFS_SYNCIO_ERROR), "AppendIO Error, retMFS:{}  fd:{}", ret, inputFile->GetFd());
        return ret;
    }

    ret = inputFile->AppendIO(opType, curOffset, mapHeader->bitmap->GetNumEntry() * BITMAP_ENTRY_SIZE, (char*)(mapHeader->bitmap->GetMapAddr()));
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MFS_SYNCIO_ERROR), "AppendIO Error, retMFS:{}  fd:{}", ret,
            inputFile->GetFd());
    }

    return ret;
}

int
MapIoHandler::_IssueMpageIO(MetaFsIoOpcode opType)
{
    return _IssueMpageIO(opType, file);
}

int
MapIoHandler::_IssueMpageIO(MetaFsIoOpcode opType, MetaFileIntf* fileToIo)
{
    int ret = 0;
    uint64_t mpageNum = 0;
    uint64_t numBitsSet = mapHeader->bitmap->GetNumBitsSet();

    for (uint64_t cnt = 0; cnt < numBitsSet; ++cnt)
    {
        mpageNum = mapHeader->bitmap->FindFirstSet(mpageNum);
        char* mpage = nullptr;
        if (opType == MetaFsIoOpcode::Read)
        {
            mpage = map->AllocateMpage(mpageNum);
            if (mpage == nullptr)
            {
                mpage = map->GetMpage(mpageNum);
                POS_TRACE_INFO(EID(MAPPER_SUCCESS), "filename:{}  mpageNum:{} Dumped mapfile loaded -> Mpage double allocation Error Cleared",
                                fileToIo->GetFileName(), mpageNum);
            }
        }
        else
        {
            mpage = map->GetMpage(mpageNum);
        }

        uint64_t fileOffset = mapHeader->size + (mpageNum * mapHeader->mpageSize);
        int retMFS = fileToIo->IssueIO(opType, fileOffset, mapHeader->mpageSize, (char*)mpage);
        if (retMFS < 0)
        {
            POS_TRACE_ERROR(EID(MFS_SYNCIO_ERROR), "AppendIO Error, retMFS:{}  fileName:{}  fd:{}  cnt:{}",
                            retMFS, fileToIo->GetFileName(), fileToIo->GetFd(), cnt);
            ret = retMFS;
        }

        mpageNum++;
    }

    return ret;
}

void
MapIoHandler::_RemoveCleanPages(MpageList& pages)
{
    for (auto page = pages.begin(); page != pages.end(); )
    {
        if (mapHeader->touchedPages->IsSetBit(*page) == false)
        {
            page = pages.erase(page);
        }
        else
        {
            ++page;
        }
    }
}

int
MapIoHandler::_Flush(SequentialPageFinder& sequentialPages)
{
    int ret = 0;
    while (sequentialPages.IsRemaining() == true)
    {
        MpageSet mpageSet = sequentialPages.PopNextMpageSet();
        ret = _FlushMpages(mpageSet.startMpage, mpageSet.numMpages);
    }
    return ret;
}

int
MapIoHandler::_FlushMpages(MpageNum startMpage, int numMpages)
{
    char* buffer = new char[mapHeader->mpageSize * numMpages];
    for (int offset = 0; offset < numMpages; offset++)
    {
        char* dest = buffer + mapHeader->mpageSize * offset;
        int pageNr = startMpage + offset;

        map->GetMpageLock(pageNr);
        memcpy(dest, (void*)map->GetMpage(pageNr), mapHeader->mpageSize);
        mapHeader->touchedPages->ClearBit(pageNr);
        map->ReleaseMpageLock(pageNr);
    }

    return _IssueFlush(buffer, startMpage, numMpages);
}

int
MapIoHandler::_IssueFlush(char* buffer, MpageNum startMpage, int numMpages)
{
    MapFlushIoContext* flushRequest = new MapFlushIoContext();
    flushRequest->opcode = MetaFsIoOpcode::Write;
    flushRequest->fd = file->GetFd();
    flushRequest->fileOffset = mapHeader->size + startMpage * mapHeader->mpageSize;
    flushRequest->length = mapHeader->mpageSize * numMpages;
    flushRequest->buffer = buffer;
    flushRequest->callback = std::bind(&MapIoHandler::_MpageFlushed, this, std::placeholders::_1);
    flushRequest->startMpage = startMpage;
    flushRequest->numMpages = numMpages;

    POS_TRACE_DEBUG(EID(MAP_FLUSH_ONGOING),
        "Issue mpage flush, startMpage {} numMpages {}",
        startMpage, numMpages);

    return file->AsyncIO(flushRequest);
}

void
MapIoHandler::_MpageFlushed(AsyncMetaFileIoCtx* ctx)
{
    MapFlushIoContext* mpageFlushReq = static_cast<MapFlushIoContext*>(ctx);
    MpageNum startMpage = mpageFlushReq->startMpage;
    uint32_t numMpages = mpageFlushReq->numMpages;
    if (mpageFlushReq->error < 0)
    {
        ioError = mpageFlushReq->error;
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR),
            "MFS AsyncIO error, ioError:{}  startMpage:{} numMpages {}", ioError,
            startMpage, numMpages);
    }

    int ret = EID(SUCCESS);
    bool flushCompleted = _IncreaseAsyncIoDonePageNum(numMpages);

    POS_TRACE_DEBUG(EID(MAP_FLUSH_ONGOING),
        "Map {} startMpage {} numMpages {} flush completed ", mapHeader->mapId,
        startMpage, numMpages);

    // After mpage flushing, Flush header data
    if (flushCompleted)
    {
        status = FLUSHING_HEADER;
        POS_TRACE_DEBUG(EID(MAP_FLUSH_ONGOING), "Starting Flush mapId:{} Header",
            mapHeader->mapId);

        MapFlushIoContext* headerFlushReq = new MapFlushIoContext();
        headerFlushReq->opcode = MetaFsIoOpcode::Write;
        headerFlushReq->fd = file->GetFd();
        headerFlushReq->length = mapHeader->size;
        headerFlushReq->buffer = mapHeaderTempBuffer;
        headerFlushReq->callback = std::bind(&MapIoHandler::_HeaderFlushed, this,
            std::placeholders::_1);

        ret = file->AsyncIO(headerFlushReq);
        if (ret < 0)
        {
            // TODO(r.saraf) error handling
        }
    }

    delete[] ctx->buffer;
    delete ctx;
}

void
MapIoHandler::_HeaderFlushed(AsyncMetaFileIoCtx* ctx)
{
    MapFlushIoContext* headerFlushReq = static_cast<MapFlushIoContext*>(ctx);
    if (headerFlushReq->error < 0)
    {
        ioError = headerFlushReq->error;
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR),
            "MFS AsyncIO(Map-Header) error, ioError:{}", ioError);
    }

    assert(status == FLUSHING_HEADER);

    if (ioError != 0)
    {
        status = FLUSHING_ERROR;
        _ResetAsyncIoPageNum();
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR),
            "MFS AsyncIO error during MapFlush mapId:{}, CHECK ABOVE LOGS!",
            mapHeader->mapId);
    }
    else
    {
        // mpage & header flush done
        status = FLUSHING_DONE;
        _ResetAsyncIoPageNum();
        POS_TRACE_DEBUG(EID(MAP_FLUSH_COMPLETED), "mapId:{} Flush Completed",
            mapHeader->mapId);
    }

    _CompleteFlush();

    delete[] ctx->buffer;
    delete ctx;
}

void
MapIoHandler::_GetTouchedPages(BitMap* validPages)
{
    uint32_t lastBit = 0;
    uint32_t currentPage = 0;

    while ((currentPage = validPages->FindFirstSet(lastBit)) != validPages->GetNumBits())
    {
        if (mapHeader->touchedPages->IsSetBit(currentPage) == true)
        {
            touchedPages->SetBit(currentPage);
        }
        lastBit = currentPage + 1;
    }
}

bool
MapIoHandler::_PrepareFlush(EventSmartPtr callback)
{
    if (flushInProgress.exchange(true) == true)
    {
        return false;
    }

    flushDoneCallBack = callback;

    // Copy mpage info of mapHeader to Temporary Buffer
    mapHeaderTempBuffer = new char[mapHeader->size];
    mapHeader->CopyToBuffer(mapHeaderTempBuffer);

    touchedPages = new BitMap(mapHeader->bitmap->GetNumBits());
    status = FLUSHING_STARTED;

    return true;
}

void
MapIoHandler::_CompleteFlush(void)
{
    EventSchedulerSingleton::Instance()->EnqueueEvent(flushDoneCallBack);

    delete touchedPages;
    flushInProgress = false;
}

bool
MapIoHandler::_IncreaseAsyncIoDonePageNum(int numMpagesDone)
{
    std::unique_lock<std::mutex> lock(mpageAsyncIoCountLock);
    numPagesAsyncIoDone += numMpagesDone;

    return (numPagesAsyncIoDone == numPagesToAsyncIo);
}

void
MapIoHandler::_ResetAsyncIoPageNum(void)
{
    numPagesToAsyncIo = 0;
    numPagesAsyncIoDone = 0;
}

} // namespace pos
