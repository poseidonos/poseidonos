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
  flushInProgress(false),
  isHeaderLoaded(false)
{
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
            fileFromLoad->GetFileName(), mapHeader->GetNumValidMpages(), mapHeader->GetNumTotalMpages());
        ret = _IssueMpageIO(MetaFsIoOpcode::Read, fileFromLoad);
    }
    return ret;
}

// Executed by EventWorker thread
int
MapIoHandler::LoadByEwThread(AsyncLoadCallBack& cb)
{
    // 1. Load Header Start
    MetaIoCbPtr mioCb = std::bind(&MapIoHandler::_LoadMpages, this, std::placeholders::_1);
    return _LoadHeader(cb, mioCb);
}


// Executed by CLI thread
int
MapIoHandler::LoadByCliThread(AsyncLoadCallBack& cb)
{
    // 1. Load Header Start
    MetaIoCbPtr mioCb1 = std::bind(&MapIoHandler::_HeaderLoadDone, this, std::placeholders::_1);
    int ret = _LoadHeader(cb, mioCb1);
    if (ret < 0)
    {
        return ret;
    }
    // Wait here for mapHeader to load
    while (isHeaderLoaded == false)
    {
        usleep(1);
    }

    // 2. Header load done, check 'valid mpage count' in Header
    if (0 == mapHeader->GetNumValidMpages())
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "There is no mpage to load, 'numValidMpage == 0'");
        return -EID(MAP_LOAD_COMPLETED);
    }

    // 3. Load Mpages Start
    uint64_t fileOffset = mapHeader->GetSize();
    MpageNum mpageNum = 0;
    uint64_t numBitsSet = mapHeader->GetMpageMap()->GetNumBitsSet();
    numPagesToAsyncIo = numBitsSet;
    numPagesAsyncIoDone = 0;

    MetaIoCbPtr mioCb2 = std::bind(&MapIoHandler::_MpageLoadDone, this, std::placeholders::_1);
    for (uint64_t cnt = 0; cnt < numBitsSet; ++cnt)
    {
        mpageNum = mapHeader->GetMpageMap()->FindFirstSet(mpageNum);
        char* mpage = map->AllocateMpage(mpageNum);
        MapFlushIoContext* mPageLoadRequest = new MapFlushIoContext(MetaFsIoOpcode::Read, file->GetFd(),
                                              fileOffset + (mpageNum * mapHeader->GetMpageSize()), mapHeader->GetMpageSize(),
                                              mpage, mioCb2);
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
MapIoHandler::FlushDirtyPagesGiven(MpageList dirtyPages, EventSmartPtr event)
{
    if (_PrepareFlush(event) == false)
    {
        return -EID(MAP_FLUSH_IN_PROGRESS);
    }

    int result = EID(SUCCESS);
    _RemoveCleanPages(dirtyPages);
    numPagesToAsyncIo = dirtyPages.size();

    if (numPagesToAsyncIo != 0)
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_STARTED), "FlushDirtyPagesGiven mapId:{} started", mapHeader->GetMapId());
        SequentialPageFinder sequentialPages(dirtyPages);
        result = _Flush(sequentialPages);
    }
    else
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_STARTED), "FlushDirtyPagesGiven mapId:{} completed, W/O any pages to flush", mapHeader->GetMapId());
        _CompleteFlush();
    }

    return result;
}

int
MapIoHandler::FlushTouchedPages(EventSmartPtr event)
{
    if (_PrepareFlush(event) == false)
    {
        return -EID(MAP_FLUSH_IN_PROGRESS);
    }

    BitMap* clonedMpageMap = mapHeader->GetBitmapFromTempBuffer(mapHeaderTempBuffer);
    _GetTouchedPages(clonedMpageMap);
    delete clonedMpageMap;

    return _FlushTouchedPages();
}

int
MapIoHandler::FlushAllPages(EventSmartPtr event)
{
    if (_PrepareFlush(event) == false)
    {
        return -EID(MAP_FLUSH_IN_PROGRESS);
    }

    BitMap* clonedMpageMap = mapHeader->GetBitmapFromTempBuffer(mapHeaderTempBuffer);
    touchedPages = clonedMpageMap;

    return _FlushTouchedPages();
}

int
MapIoHandler::_FlushTouchedPages(void)
{
    int ret = 0;
    if (touchedPages->GetNumBitsSet() != 0)
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_STARTED), "_TriggerFlushWithTouchedPages mapId:{} started", mapHeader->GetMapId());
        numPagesToAsyncIo = touchedPages->GetNumBitsSet();
        SequentialPageFinder finder(touchedPages);
        ret = _Flush(finder);
    }
    else
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_COMPLETED), "mapId:{} _TriggerFlushWithTouchedPages completed, W/O any pages to flush", mapHeader->GetMapId());
        _CompleteFlush();
    }
    return ret;
}

int
MapIoHandler::_LoadHeader(AsyncLoadCallBack& cb, MetaIoCbPtr mioCb)
{
    status = LOADING_HEADER_STARTED;
    POS_TRACE_INFO(EID(MAP_LOAD_STARTED), "Header Async Loading Started, mapId:{}", mapHeader->GetMapId());

    loadFinishedCallBack = cb;
    uint64_t lenToRead = mapHeader->GetSize();
    mapHeaderTempBuffer = new char[lenToRead]();

    MapFlushIoContext* headerLoadRequest = new MapFlushIoContext(MetaFsIoOpcode::Read, file->GetFd(), 0, lenToRead, mapHeaderTempBuffer, mioCb);

    ioError = 0;
    int ret = file->AsyncIO(headerLoadRequest);
    if (unlikely(ret < 0))
    {
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS returned error on Header async loading, mapId:{}", mapHeader->GetMapId());
    }

    return ret;
}

// Executed by meta-fs thread
void
MapIoHandler::_LoadMpages(AsyncMetaFileIoCtx* ctx)
{
    MapFlushIoContext* headerLoadReqCtx = static_cast<MapFlushIoContext*>(ctx);
    _FillMapHeader(ctx);

    // 2. Header load done, check 'valid mpage count' in Header
    if (0 == mapHeader->GetNumValidMpages())
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "There is no mpage to load, 'numValidMpage == 0'");
        loadFinishedCallBack(mapHeader->GetMapId());
        delete[] headerLoadReqCtx->buffer;
        delete headerLoadReqCtx;
        return;
    }

    // 3. Load Mpages Start (by Event)
    numPagesToAsyncIo = mapHeader->GetMpageMap()->GetNumBitsSet();
    numPagesAsyncIoDone = 0;
    MetaIoCbPtr mpageAsyncLoadReqCB = std::bind(&MapIoHandler::_MpageLoadDone, this, std::placeholders::_1);

    EventSmartPtr mpageLoadRequest = std::make_shared<EventMpageAsyncIo>(mapHeader, map, file, mpageAsyncLoadReqCB);
    EventSchedulerSingleton::Instance()->EnqueueEvent(mpageLoadRequest);

    delete[] headerLoadReqCtx->buffer;
    delete headerLoadReqCtx;
}

void
MapIoHandler::_HeaderLoadDone(AsyncMetaFileIoCtx* ctx)
{
    _FillMapHeader(ctx);
    isHeaderLoaded = true;
}

void
MapIoHandler::_FillMapHeader(AsyncMetaFileIoCtx* ctx)
{
    // Update mapHeader with mapHeaderTempBuffer read
    MapFlushIoContext* headerLoadReqCtx = static_cast<MapFlushIoContext*>(ctx);
    if (unlikely(headerLoadReqCtx->error < 0))
    {
        ioError = headerLoadReqCtx->error;
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO error, ioError:{}", ioError);
    }

    int bufferOffset = 0;
    memcpy(mapHeader->GetMpageDataAddr(), mapHeaderTempBuffer + bufferOffset, mapHeader->GetSizeofMpageData());
    bufferOffset += mapHeader->GetSizeofMpageData();
    memcpy(mapHeader->GetGetUsedBlkCntAddr(), mapHeaderTempBuffer + bufferOffset, mapHeader->GetSizeofUsedBlkCnt());
    bufferOffset += mapHeader->GetSizeofUsedBlkCnt();
    memcpy(mapHeader->GetMpageMap()->GetMapAddr(), mapHeaderTempBuffer + bufferOffset, mapHeader->GetMpageMap()->GetNumEntry() * BITMAP_ENTRY_SIZE);

    mapHeader->ApplyNumValidMpages();
    status = LOADING_HEADER_DONE;
    POS_TRACE_INFO(EID(MAPPER_SUCCESS), "fileName:{} Header Load Success, ValidMpages:{} / TotalMpages:{}",
                   file->GetFileName(), mapHeader->GetNumValidMpages(), mapHeader->GetNumTotalMpages());
}

// Executed by meta-fs thread
void
MapIoHandler::_MpageLoadDone(AsyncMetaFileIoCtx* ctx)
{
    MapFlushIoContext* mPageLoadReqCtx = static_cast<MapFlushIoContext*>(ctx);
    MpageNum startMpage = mPageLoadReqCtx->startMpage;
    int numMpages = mPageLoadReqCtx->numMpages;
    if (mPageLoadReqCtx->error < 0)
    {
        ioError = mPageLoadReqCtx->error;
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO error, ioError:{}  startMpage:{} numMpages:{}", ioError, startMpage, numMpages);
    }

    POS_TRACE_DEBUG(EID(MAP_LOAD_ONGOING), "mapId:{}  startMpage:{} numMpages:{} async load completed ", mapHeader->GetMapId(), startMpage, numMpages);

    bool loadCompleted = _IncreaseAsyncIoDonePageNum(numMpages);
    if (loadCompleted)
    {
        _ResetAsyncIoPageNum();
        if (ioError != 0)
        {
            status = LOADING_ERROR;
            POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO error during _MpageAsyncLoaded mapId:{}, CHECK ABOVE LOGS!", mapHeader->GetMapId());
        }
        else
        {
            // MapHeader & mpages loading done
            status = LOADING_DONE;
            POS_TRACE_INFO(EID(MAP_LOAD_COMPLETED), "mapId:{} async load completed", mapHeader->GetMapId());
            loadFinishedCallBack(mapHeader->GetMapId());
        }
    }

    delete mPageLoadReqCtx;
}

int
MapIoHandler::_IssueHeaderIO(MetaFsIoOpcode opType, MetaFileIntf* inputFile)
{
    uint64_t curOffset = 0;
    int ret = 0;

    ret = inputFile->AppendIO(opType, curOffset, mapHeader->GetSizeofMpageData(), (char*)mapHeader->GetMpageDataAddr());
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MFS_SYNCIO_ERROR), "AppendIO Error, retMFS:{}  fd:{}", ret, inputFile->GetFd());
        return ret;
    }

    ret = inputFile->AppendIO(opType, curOffset, mapHeader->GetSizeofUsedBlkCnt(), (char*)mapHeader->GetGetUsedBlkCntAddr());
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MFS_SYNCIO_ERROR), "AppendIO Error, retMFS:{}  fd:{}", ret, inputFile->GetFd());
        return ret;
    }

    ret = inputFile->AppendIO(opType, curOffset, mapHeader->GetMpageMap()->GetNumEntry() * BITMAP_ENTRY_SIZE, (char*)(mapHeader->GetMpageMap()->GetMapAddr()));
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MFS_SYNCIO_ERROR), "AppendIO Error, retMFS:{}  fd:{}", ret,
            inputFile->GetFd());
    }

    return ret;
}

int
MapIoHandler::_IssueMpageIO(MetaFsIoOpcode opType, MetaFileIntf* fileToIo)
{
    int ret = 0;
    uint64_t mpageNum = 0;
    uint64_t numBitsSet = mapHeader->GetMpageMap()->GetNumBitsSet();

    for (uint64_t cnt = 0; cnt < numBitsSet; ++cnt)
    {
        mpageNum = mapHeader->GetMpageMap()->FindFirstSet(mpageNum);
        char* mpage = nullptr;
        if (opType == MetaFsIoOpcode::Read)
        {
            mpage = map->AllocateMpage(mpageNum);
            if (mpage == nullptr)
            {
                mpage = map->GetMpage(mpageNum);
                POS_TRACE_INFO(EID(MAPPER_SUCCESS), "filename:{}  mpageNum:{} Dumped mapfile loaded -> Mpage double allocation Error Cleared", fileToIo->GetFileName(), mpageNum);
            }
        }
        else
        {
            mpage = map->GetMpage(mpageNum);
        }

        uint64_t fileOffset = mapHeader->GetSize() + (mpageNum * mapHeader->GetMpageSize());
        int retMFS = fileToIo->IssueIO(opType, fileOffset, mapHeader->GetMpageSize(), (char*)mpage);
        if (retMFS < 0)
        {
            POS_TRACE_ERROR(EID(MFS_SYNCIO_ERROR), "AppendIO Error, retMFS:{}  fileName:{}  fd:{}  cnt:{}", retMFS, fileToIo->GetFileName(), fileToIo->GetFd(), cnt);
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
        if (mapHeader->GetTouchedMpages()->IsSetBit(*page) == false)
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
    char* buffer = new char[mapHeader->GetMpageSize() * numMpages];
    for (int offset = 0; offset < numMpages; offset++)
    {
        char* dest = buffer + mapHeader->GetMpageSize() * offset;
        int pageNr = startMpage + offset;

        map->GetMpageLock(pageNr);
        memcpy(dest, (void*)map->GetMpage(pageNr), mapHeader->GetMpageSize());
        mapHeader->GetTouchedMpages()->ClearBit(pageNr);
        map->ReleaseMpageLock(pageNr);
    }

    return _IssueFlush(buffer, startMpage, numMpages);
}

int
MapIoHandler::_IssueFlush(char* buffer, MpageNum startMpage, int numMpages)
{
    MetaIoCbPtr mioCb = std::bind(&MapIoHandler::_MpageFlushed, this, std::placeholders::_1);
    MapFlushIoContext* flushRequest = new MapFlushIoContext(MetaFsIoOpcode::Write, file->GetFd(),
                                      mapHeader->GetSize() + startMpage * mapHeader->GetMpageSize(), mapHeader->GetMpageSize() * numMpages,
                                      buffer, mioCb);
    flushRequest->startMpage = startMpage;
    flushRequest->numMpages = numMpages;

    POS_TRACE_DEBUG(EID(MAP_FLUSH_ONGOING), "Issue mpage flush, startMpage {} numMpages {}", startMpage, numMpages);

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
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO error, ioError:{}  startMpage:{} numMpages {}", ioError, startMpage, numMpages);
    }

    int ret = EID(SUCCESS);
    bool flushCompleted = _IncreaseAsyncIoDonePageNum(numMpages);

    POS_TRACE_DEBUG(EID(MAP_FLUSH_ONGOING), "Map {} startMpage {} numMpages {} flush completed ", mapHeader->GetMapId(), startMpage, numMpages);

    // After mpage flushing, Flush header data
    if (flushCompleted)
    {
        status = FLUSHING_HEADER;
        POS_TRACE_DEBUG(EID(MAP_FLUSH_ONGOING), "Starting Flush mapId:{} Header", mapHeader->GetMapId());

        MetaIoCbPtr mioCb = std::bind(&MapIoHandler::_HeaderFlushed, this, std::placeholders::_1);
        MapFlushIoContext* headerFlushReq = new MapFlushIoContext(MetaFsIoOpcode::Write, file->GetFd(), 0, mapHeader->GetSize(), mapHeaderTempBuffer, mioCb);
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
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO(Map-Header) error, ioError:{}", ioError);
    }

    assert(status == FLUSHING_HEADER);

    if (ioError != 0)
    {
        status = FLUSHING_ERROR;
        _ResetAsyncIoPageNum();
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO error during MapFlush mapId:{}, CHECK ABOVE LOGS!", mapHeader->GetMapId());
    }
    else
    {
        // mpage & header flush done
        status = FLUSHING_DONE;
        _ResetAsyncIoPageNum();
        POS_TRACE_DEBUG(EID(MAP_FLUSH_COMPLETED), "mapId:{} Flush Completed", mapHeader->GetMapId());
    }

    _CompleteFlush();

    // mapHeaderTempBuffer that was deleted in _CompleteFlush() was assigned to ctx->buffer
    delete ctx;
}

void
MapIoHandler::_GetTouchedPages(BitMap* mpageMap)
{
    uint32_t lastBit = 0;
    uint32_t currentPage = 0;

    while ((currentPage = mpageMap->FindFirstSet(lastBit)) != mpageMap->GetNumBits())
    {
        // Clone 'touchedMpages' in 'mapHeader' to 'touchedPages'
        if (mapHeader->GetTouchedMpages()->IsSetBit(currentPage) == true)
        {
            touchedPages->SetBit(currentPage);
        }
        lastBit = currentPage + 1;
    }
}

bool
MapIoHandler::_PrepareFlush(EventSmartPtr event)
{
    if (flushInProgress.exchange(true) == true)
    {
        return false;
    }

    eventAfterflushDone = event;

    // Copy mpage info of mapHeader to Temporary Buffer (mapHeaderTempBuffer)
    mapHeaderTempBuffer = new char[mapHeader->GetSize()];
    mapHeader->CopyToBuffer(mapHeaderTempBuffer);

    touchedPages = new BitMap(mapHeader->GetMpageMap()->GetNumBits());
    status = FLUSHING_STARTED;

    return true;
}

void
MapIoHandler::_CompleteFlush(void)
{
    EventSchedulerSingleton::Instance()->EnqueueEvent(eventAfterflushDone);
    delete touchedPages;
    touchedPages = nullptr;
    delete mapHeaderTempBuffer;
    mapHeaderTempBuffer = nullptr;
    flushInProgress = false;
}

bool
MapIoHandler::_IncreaseAsyncIoDonePageNum(int numMpagesDone)
{
    int numIoDoneSoFar = numPagesAsyncIoDone.fetch_add(numMpagesDone) + numMpagesDone;
    return (numIoDoneSoFar == numPagesToAsyncIo);
}

void
MapIoHandler::_ResetAsyncIoPageNum(void)
{
    numPagesToAsyncIo = 0;
    numPagesAsyncIoDone = 0;
}

} // namespace pos
