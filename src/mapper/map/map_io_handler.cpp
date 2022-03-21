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

#include "src/mapper/map/map_io_handler.h"

#include "src/event_scheduler/event_scheduler.h"
#include "src/logger/logger.h"
#include "src/mapper/i_map_flush.h"
#include "src/mapper/map/event_mpage_async_io.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "src/metafs/metafs_file_intf.h"

namespace pos
{
MapIoHandler::MapIoHandler(MetaFileIntf* file_, Map* mapData, MapHeader* mapHeaderData, int mapId_, MapperAddressInfo* addrInfo_)
: touchedPages(nullptr),
  mapId(mapId_),
  map(mapData),
  mapHeader(mapHeaderData),
  file(file_),
  mapHeaderTempBuffer(nullptr),
  status(IDLE),
  numPagesToAsyncIo(0),
  numPagesAsyncIoDone(0),
  flushInProgress(false),
  ioError(0),
  addrInfo(addrInfo_)
{
}

MapIoHandler::MapIoHandler(Map* mapData, MapHeader* mapHeaderData, int mapId_, MapperAddressInfo* addrInfo_)
: MapIoHandler(nullptr,  mapData, mapHeaderData, mapId_, addrInfo_)
{
}
// LCOV_EXCL_START
MapIoHandler::~MapIoHandler(void)
{
    Dispose();
}
// LCOV_EXCL_STOP
int
MapIoHandler::OpenFile(std::string fileName, uint64_t fileSize)
{
    if (addrInfo->IsUT() == false)
    {
        assert(file == nullptr);
        file = new MetaFsFileIntf(fileName, addrInfo->GetArrayId());
    }
    else
    {
        if (file == nullptr)
        {
            file = new MockFileIntf(fileName, addrInfo->GetArrayId());
        }
    }

    if (file->DoesFileExist() == false)
    {
        int ret = file->Create(fileSize);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MFS_FILE_CREATE_FAILED), "Map file creation failed, fileName:{}, array:{}", fileName, addrInfo->GetArrayId());
            delete file;
            file = nullptr;
            return ret;
        }
        POS_TRACE_INFO(EID(MFS_START), "Map file created, fileName:{}, array:{}", fileName, addrInfo->GetArrayId());
        file->Open();
        return EID(NEED_TO_INITIAL_STORE); // SaveHeader
    }
    else
    {
        file->Open();
        return 1; // Load
    }
}

void
MapIoHandler::Dispose(void)
{
    if (file != nullptr)
    {
        if (file->IsOpened())
        {
            file->Close();
        }
        delete file;
        file = nullptr;
    }
}

int
MapIoHandler::DeleteFile(void)
{
    int ret = 0;
    if (file != nullptr)
    {
        if (file->DoesFileExist() == true)
        {
            if (file->IsOpened() == true)
            {
                file->Close();
            }
            ret = file->Delete();
            if (ret < 0)
            {
                POS_TRACE_ERROR(EID(MFS_FILE_DELETE_FAILED), "MFS File:{} delete failed", file->GetFileName());
                return ret;
            }
        }
        delete file;
        file = nullptr;
    }
    return ret;
}

bool
MapIoHandler::DoesFileExist(void)
{
    if ((file == nullptr) || (file->DoesFileExist() == false))
    {
        return false;
    }
    return true;
}

int
MapIoHandler::Load(AsyncLoadCallBack& cb)
{
    int ret = _MakeFileReady();
    if (ret < 0)
    {
        return ret;
    }

    status = LOADING_HEADER_STARTED;
    POS_TRACE_INFO(EID(MAP_LOAD_STARTED), "Header Async Loading Started, mapId:{}", mapId);

    loadFinishedCallBack = cb;
    int lenToRead = mapHeader->GetSize();
    mapHeaderTempBuffer = new char[lenToRead]();

    MapFlushIoContext* headerLoadRequest = new MapFlushIoContext();
    headerLoadRequest->opcode = MetaFsIoOpcode::Read;
    headerLoadRequest->fd = file->GetFd();
    headerLoadRequest->fileOffset = 0;
    headerLoadRequest->length = lenToRead;
    headerLoadRequest->buffer = mapHeaderTempBuffer;
    headerLoadRequest->callback = std::bind(&MapIoHandler::_HeaderAsyncLoaded, this, std::placeholders::_1);

    ioError = 0;
    ret = file->AsyncIO(headerLoadRequest);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS returned error on Header async loading, mapId:{}", mapId);
    }
    return ret;
}

int
MapIoHandler::FlushHeader(EventSmartPtr callback)
{
    if (_PrepareFlush(callback) == false)
        return -EID(MAP_FLUSH_IN_PROGRESS);
    {
    }
    return _IssueFlushHeader();
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
        POS_TRACE_DEBUG(EID(MAP_FLUSH_STARTED), "FlushDirtyPagesGiven mapId:{} started", mapId);

        SequentialPageFinder sequentialPages(dirtyPages);
        result = _Flush(sequentialPages);
    }
    else
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_STARTED), "FlushDirtyPagesGiven mapId:{} completed, W/O any pages to flush", mapId);
        _CompleteFlush();
    }

    return result;
}

int
MapIoHandler::FlushTouchedPages(EventSmartPtr callback)
{
    if (_PrepareFlush(callback) == false)
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_COMPLETED), "[MAPPER FLUSH] Failed to Issue Flush, Another Flush is still progressing, mapId:{}, status:{}", mapId, status);
        return -EID(MAP_FLUSH_IN_PROGRESS);
    }
    int result = EID(SUCCESS);
    BitMap* copiedBitmap = mapHeader->GetBitmapFromTempBuffer(mapHeaderTempBuffer);
    _GetTouchedPages(copiedBitmap);
    delete copiedBitmap;
    if (touchedPages->GetNumBitsSet() != 0)
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_STARTED), "[MAPPER FlushTouchedPages mapId:{}] started", mapId);
        numPagesToAsyncIo = touchedPages->GetNumBitsSet();
        SequentialPageFinder finder(touchedPages);
        result = _Flush(finder);
    }
    else
    {
        POS_TRACE_DEBUG(EID(MAP_FLUSH_COMPLETED), "[MAPPER FlushTouchedPages mapId:{}] completed, W/O any pages to flush", mapId);
        _CompleteFlush();
        // TODO(r.saraf) Handle callback returning false value (callback executed in _CompleteFlush)
    }

    return result;
}

int
MapIoHandler::LoadForWBT(MetaFileIntf* fileFromLoad)
{
    int lenToRead = mapHeader->GetSize();
    char *headerBuf = new char[lenToRead]();
    int ret = _IssueHeaderIoByMockFs(MetaFsIoOpcode::Read, fileFromLoad, headerBuf);
    if (ret == 0)
    {
        mapHeader->ApplyHeader(headerBuf);
        POS_TRACE_INFO(EID(MAP_LOAD_ONGOING),
            "fileName:{} header load success, valid:{} / total:{}",
            fileFromLoad->GetFileName(), mapHeader->GetNumValidMpages(), map->GetNumMpages());
        ret = _IssueMpageIoByMockFs(MetaFsIoOpcode::Read, fileFromLoad);
    }
    delete [] headerBuf;
    return ret;
}

int
MapIoHandler::StoreForWBT(MetaFileIntf* fileToStore)
{
    if (fileToStore == nullptr)
    {
        fileToStore = file;
    }
    // mpage store
    int ret = _IssueMpageIoByMockFs(MetaFsIoOpcode::Write, fileToStore);
    if (ret == 0)
    {
        // header store
        int lenToRead = mapHeader->GetSize();
        char *headerBuf = new char[lenToRead]();
        mapHeader->CopyToBuffer(headerBuf);
        ret = _IssueHeaderIoByMockFs(MetaFsIoOpcode::Write, fileToStore, headerBuf);
        delete [] headerBuf;
    }

    return ret;
}

int
MapIoHandler::_MakeFileReady(void)
{
    if (file->DoesFileExist() == true)
    {
        if (file->IsOpened() == false)
        {
            file->Open();
        }
        return 0;
    }
    else
    {
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "Tried to access file that doesn't exist:{}", file->GetFileName());
        return -1;
    }
}

void
MapIoHandler::_HeaderAsyncLoaded(AsyncMetaFileIoCtx* ctx)
{
    MapFlushIoContext* headerLoadReqCtx = static_cast<MapFlushIoContext*>(ctx);
    if (headerLoadReqCtx->error < 0)
    {
        ioError = headerLoadReqCtx->error;
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO error, ioError:{}", ioError);
    }

    mapHeader->ApplyHeader(mapHeaderTempBuffer);
    status = LOADING_HEADER_DONE;
    POS_TRACE_INFO(EID(MAPPER_SUCCESS),
        "fileName:{} Header Async Load Done, Valid:{} / Total:{}",
        file->GetFileName(), mapHeader->GetNumValidMpages(), map->GetNumMpages());

    if (0 == mapHeader->GetNumValidMpages())
    {
        POS_TRACE_INFO(EID(MAPPER_SUCCESS), "There is no mpage to load, 'numValidMpage == 0'");
        loadFinishedCallBack(mapId);
        delete[] headerLoadReqCtx->buffer;
        delete headerLoadReqCtx;
        return;
    }

    // Mpages Async-load Request by Event
    numPagesToAsyncIo = mapHeader->GetNumValidMpages();
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
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO error, ioError:{}  startMpage:{} numMpages:{}", ioError, startMpage, numMpages);
    }
    bool loadCompleted = _IncreaseAsyncIoDonePageNum(numMpages);
    if (loadCompleted)
    {
        _ResetAsyncIoPageNum();
        if (ioError != 0)
        {
            status = LOADING_ERROR;
            POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO error during _MpageAsyncLoaded mapId:{}, CHECK ABOVE LOGS!", mapId);
        }
        else
        {
            // MapHeader & mpages loading done
            status = LOADING_DONE;
            POS_TRACE_INFO(EID(MAP_LOAD_COMPLETED), "mapId:{} async load completed", mapId);
            loadFinishedCallBack(mapId);
        }
    }

    delete mPageLoadReqCtx;
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
    char* buffer = new char[map->GetSize() * numMpages];
    for (int offset = 0; offset < numMpages; offset++)
    {
        char* dest = buffer + map->GetSize() * offset;
        int pageNr = startMpage + offset;

        map->GetMpageLock(pageNr);
        memcpy(dest, (void*)map->GetMpage(pageNr), map->GetSize());
        mapHeader->GetTouchedMpages()->ClearBit(pageNr);
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
    flushRequest->fileOffset = mapHeader->GetSize() + startMpage * map->GetSize();
    flushRequest->length = map->GetSize() * numMpages;
    flushRequest->buffer = buffer;
    flushRequest->callback = std::bind(&MapIoHandler::_MpageFlushed, this, std::placeholders::_1);
    flushRequest->startMpage = startMpage;
    flushRequest->numMpages = numMpages;

    POS_TRACE_DEBUG(EID(MAP_FLUSH_ONGOING), "Issue mpage flush, startMpage {} numMpages {}", startMpage, numMpages);

    return file->AsyncIO(flushRequest);
}

int
MapIoHandler::_IssueFlushHeader(void)
{
    status = FLUSHING_HEADER;
    POS_TRACE_DEBUG(EID(MAP_FLUSH_ONGOING), "Starting Flush mapId:{} Header", mapId);

    MapFlushIoContext* headerFlushReq = new MapFlushIoContext();
    headerFlushReq->opcode = MetaFsIoOpcode::Write;
    headerFlushReq->fd = file->GetFd();
    headerFlushReq->length = mapHeader->GetSize();
    headerFlushReq->buffer = mapHeaderTempBuffer;
    headerFlushReq->callback = std::bind(&MapIoHandler::_HeaderFlushed, this, std::placeholders::_1);

    int ret = file->AsyncIO(headerFlushReq);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MAP_FLUSH_ONGOING), "Failed to Flush mapId:{} Header", mapId);
        // TODO(r.saraf) error handling
    }
    return ret;
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

    POS_TRACE_DEBUG(EID(MAP_FLUSH_ONGOING), "Map {} startMpage {} numMpages {} flush completed ", mapId, startMpage, numMpages);

    // After mpage flushing, Flush header data
    if (flushCompleted)
    {
        ret = _IssueFlushHeader();
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
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "MFS AsyncIO error during MapFlush mapId:{}, CHECK ABOVE LOGS!", mapId);
    }
    else
    {
        // mpage & header flush done
        status = FLUSHING_DONE;
        _ResetAsyncIoPageNum();
        POS_TRACE_DEBUG(EID(MAP_FLUSH_COMPLETED), "mapId:{} Flush Completed", mapId);
    }

    _CompleteFlush();

    // mapHeaderTempBuffer that was deleted in _CompleteFlush() was assigned to ctx->buffer
    delete ctx;
}

void
MapIoHandler::_GetTouchedPages(BitMap* validPages)
{
    uint32_t lastBit = 0;
    uint32_t currentPage = 0;

    while ((currentPage = validPages->FindFirstSet(lastBit)) != validPages->GetNumBits())
    {
        if (mapHeader->GetTouchedMpages()->IsSetBit(currentPage) == true)
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
    mapHeaderTempBuffer = new char[mapHeader->GetSize()];
    mapHeader->CopyToBuffer(mapHeaderTempBuffer);

    touchedPages = new BitMap(map->GetNumMpages());
    status = FLUSHING_STARTED;

    return true;
}

void
MapIoHandler::_CompleteFlush(void)
{
    EventSchedulerSingleton::Instance()->EnqueueEvent(flushDoneCallBack);
    delete touchedPages;
    touchedPages = nullptr;
    delete[] mapHeaderTempBuffer;
    mapHeaderTempBuffer = nullptr;
    flushInProgress = false;
}

bool
MapIoHandler::_IncreaseAsyncIoDonePageNum(int numMpagesDone)
{
    int res = numPagesAsyncIoDone.fetch_add(numMpagesDone);
    return ((res + numMpagesDone) == numPagesToAsyncIo);
}

void
MapIoHandler::_ResetAsyncIoPageNum(void)
{
    numPagesToAsyncIo = 0;
    numPagesAsyncIoDone = 0;
}
int
MapIoHandler::_IssueHeaderIoByMockFs(MetaFsIoOpcode opType, MetaFileIntf* fileToIo, char* headerBuf)
{
    // Used by WBT only
    int ret = fileToIo->IssueIO(opType, 0, mapHeader->GetSize(), headerBuf);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(MFS_SYNCIO_ERROR), "MapHeader AppendIO Error, retMFS:{}  fileName:{}  fd:{}", ret, fileToIo->GetFileName(), fileToIo->GetFd());
    }
    return ret;
}

int
MapIoHandler::_IssueMpageIoByMockFs(MetaFsIoOpcode opType, MetaFileIntf* fileToIo)
{
    // Used by WBT only
    int ret = 0;
    uint64_t mpageNum = 0;
    uint64_t numBitsSet = mapHeader->GetNumValidMpages();

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

        uint64_t fileOffset = mapHeader->GetSize() + (mpageNum * map->GetSize());
        int retMFS = fileToIo->IssueIO(opType, fileOffset, map->GetSize(), (char*)mpage);
        if (retMFS < 0)
        {
            POS_TRACE_ERROR(EID(MFS_SYNCIO_ERROR), "AppendIO Error, retMFS:{}  fileName:{}  cnt:{}", retMFS, fileToIo->GetFileName(), cnt);
            ret = retMFS;
        }

        mpageNum++;
    }

    return ret;
}

} // namespace pos
