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

#include "src/allocator/context_manager/context_io_manager.h"

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/allocator_file_io.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
ContextIoManager::ContextIoManager(AllocatorAddressInfo* info, TelemetryPublisher* tp)
: ContextIoManager(info, tp, EventSchedulerSingleton::Instance())
{
}

ContextIoManager::ContextIoManager(AllocatorAddressInfo* info, TelemetryPublisher* tp, EventScheduler* scheduler)
: flushInProgress(false),
  numFilesFlushing(0),
  numFilesReading(0),
  addrInfo(info),
  telPublisher(tp),
  eventScheduler(scheduler)
{
    for (int owner = 0; owner < NUM_FILES; owner++)
    {
        fileIo[owner] = nullptr;
    }
}

ContextIoManager::~ContextIoManager(void)
{
    for (int owner = 0; owner < NUM_FILES; owner++)
    {
        if (fileIo[owner] != nullptr)
        {
            delete fileIo[owner];
        }
    }
}

void
ContextIoManager::SetAllocatorFileIo(int owner, AllocatorFileIo* io)
{
    fileIo[owner] = io;
}

void
ContextIoManager::Init(void)
{
    int ret = 0;

    POS_TRACE_INFO(EID(ALLOCATOR_META_ASYNCLOAD), "[AllocatorLoad] start to init and load allocator files");
    for (int owner = 0; owner < NUM_FILES; owner++)
    {
        fileIo[owner]->Init();
        numFilesReading++;

        MetaIoCbPtr loadCallback = std::bind(&ContextIoManager::_LoadCompletedThenCB, this, std::placeholders::_1);
        ret = fileIo[owner]->LoadContext(loadCallback);
        if (ret == 0) // new file created
        {
            numFilesReading--;

            MetaIoCbPtr cbPtr;
            if (owner == REBUILD_CTX)
            {
                numRebuildFlushInProgress++;
                cbPtr = std::bind(&ContextIoManager::_RebuildFlushCompletedThenCB, this, std::placeholders::_1);
            }
            else
            {
                numFilesFlushing++;
                cbPtr = std::bind(&ContextIoManager::_FlushCompletedThenCB, this, std::placeholders::_1);
            }

            ret = fileIo[owner]->Flush(cbPtr);
            if (ret == 0)
            {
                _WaitPendingIo(IOTYPE_ALL);
                POS_TRACE_INFO(EID(ALLOCATOR_META_ASYNCLOAD),
                    "[AllocatorLoad] initial flush allocator file:{}, pendingMetaIo:{}, pendingRebuildIo:{}",
                    owner, numFilesFlushing + numFilesFlushing, numRebuildFlushInProgress);
            }
            else
            {
                break;
            }
        }
        else if (ret == 1) // loading
        {
            _WaitPendingIo(IOTYPE_READ);
        }
        else
        {
            break;
        }
    }

    if (ret < 0)
    {
        while (addrInfo->IsUT() != true)
        {
            usleep(1); // assert(false);
        }
    }
}

void
ContextIoManager::_LoadCompletedThenCB(AsyncMetaFileIoCtx* ctx)
{
    CtxHeader* header = reinterpret_cast<CtxHeader*>(ctx->buffer);

    int owner = _FindOwner(header->sig);
    assert(numFilesReading > 0);
    numFilesReading--;
    if (owner == -1)
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR),
            "[AllocatorLoad] Error!! Loaded Allocator file signature is not matched:{}", header->sig);
        delete[] ctx->buffer;
        delete ctx;

        while (addrInfo->IsUT() != true)
        {
            usleep(1); // assert(false);
        }
        return;
    }
    else
    {
        fileIo[owner]->AfterLoad(ctx->buffer);
        POS_TRACE_INFO(EID(ALLOCATOR_META_ASYNCLOAD), "[AllocatorLoad] Async allocator file:{} load done!", owner);
        delete[] ctx->buffer;
        delete ctx;
    }
}

void
ContextIoManager::Dispose(void)
{
    for (int owner = 0; owner < NUM_FILES; owner++)
    {
        fileIo[owner]->Dispose();
    }
}

int
ContextIoManager::FlushContexts(EventSmartPtr callback, bool sync)
{
    if (flushInProgress.exchange(true) == true)
    {
        return (int)POS_EVENT_ID::ALLOCATOR_META_ARCHIVE_FLUSH_IN_PROGRESS;
    }
    POS_TRACE_INFO(EID(ALLOCATOR_META_ARCHIVE_STORE), "[AllocatorFlush] sync:{}, start to flush", sync);

    int ret = 0;
    flushCallback = callback;

    numFilesFlushing += NUM_ALLOCATOR_FILES; // Issue 2 contexts(segmentctx, allocatorctx)
    for (int owner = 0; owner < NUM_ALLOCATOR_FILES; owner++)
    {
        MetaIoCbPtr curCb = (owner == REBUILD_CTX) ? std::bind(&ContextIoManager::_RebuildFlushCompletedThenCB, this, std::placeholders::_1) : std::bind(&ContextIoManager::_FlushCompletedThenCB, this, std::placeholders::_1);

        ret = fileIo[owner]->Flush(curCb);
        if (ret != 0)
        {
            break;
        }
    }

    telPublisher->PublishData(TEL002_ALCT_ALCTX_PENDINGIO_CNT, numFilesFlushing);

    if (sync == true)
    {
        for (int owner = 0; owner < NUM_ALLOCATOR_FILES; owner++)
        {
            _WaitPendingIo(IOTYPE_FLUSH);
        }
    }
    return ret;
}

void
ContextIoManager::_WaitPendingIo(IOTYPE type)
{
    while (type == IOTYPE_ALL)
    {
        if ((numFilesReading + numFilesFlushing == 0) || (addrInfo->IsUT() == true))
        {
            return;
        }
        usleep(1);
    }
    while (type == IOTYPE_READ)
    {
        if ((numFilesReading == 0) || (addrInfo->IsUT() == true))
        {
            return;
        }
        usleep(1);
    }
    while (type == IOTYPE_FLUSH)
    {
        if ((numFilesFlushing == 0) || (addrInfo->IsUT() == true))
        {
            return;
        }
        usleep(1);
    }
}

void
ContextIoManager::_FlushCompletedThenCB(AsyncMetaFileIoCtx* ctx)
{
    CtxHeader* header = reinterpret_cast<CtxHeader*>(ctx->buffer);
    assert(numFilesFlushing > 0);
    telPublisher->PublishData(TEL002_ALCT_ALCTX_PENDINGIO_CNT, numFilesFlushing);
    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_STORE),
        "[AllocatorFlush] Allocator file stored, sig:{}, version:{}, pendingMetaIo:{}",
        header->sig, header->ctxVersion, numFilesFlushing);

    int owner = _FindOwner(header->sig);
    if (owner == -1)
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_META_ARCHIVE_STORE),
            "[AllocatorFlush] Wrong context returned, signature is {}", header->sig);
        return;
    }

    fileIo[owner]->AfterFlush(ctx);

    int remaining = numFilesFlushing.fetch_sub(1) - 1;
    if (remaining == 0)
    {
        POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_STORE), "[AllocatorFlush] Complete to flush allocator files");
        flushInProgress = false;
        if (flushCallback != nullptr)
        {
            eventScheduler->EnqueueEvent(flushCallback);
            flushCallback = nullptr;
        }
    }

    delete[] ctx->buffer;
    delete ctx;
}

void
ContextIoManager::_RebuildFlushCompletedThenCB(AsyncMetaFileIoCtx* ctx)
{
    CtxHeader* header = reinterpret_cast<CtxHeader*>(ctx->buffer);
    int owner = _FindOwner(header->sig);
    assert(owner == REBUILD_CTX);

    fileIo[owner]->AfterFlush(ctx);
    assert(numRebuildFlushInProgress > 0);
    numRebuildFlushInProgress--;
    POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_STORE),
        "[RebuildCtx Flush] Complete to flush RebuildCtx files, sig:{}, version:{}, RebuildIoCount:{}",
        header->sig, header->ctxVersion, numRebuildFlushInProgress);
    delete[] ctx->buffer;
    delete ctx;
}

int
ContextIoManager::FlushRebuildContext(EventSmartPtr callback, bool sync)
{
    POS_TRACE_INFO(EID(ALLOCATOR_META_ARCHIVE_STORE),
        "[RebuildCtxFlush] sync:{}, rebuildIssuedCount:{}, FlushIssuedCount:{}, start to flush",
        sync, numRebuildFlushInProgress, numFilesFlushing);
    numRebuildFlushInProgress++;

    MetaIoCbPtr curCb = std::bind(&ContextIoManager::_RebuildFlushCompletedThenCB, this, std::placeholders::_1);
    int ret = fileIo[REBUILD_CTX]->Flush(curCb);
    if (sync == true)
    {
        _WaitPendingIo(IOTYPE_FLUSH);
    }
    return ret;
}

uint64_t
ContextIoManager::GetStoredContextVersion(int owner)
{
    return fileIo[owner]->GetStoredVersion();
}

char*
ContextIoManager::GetContextSectionAddr(int owner, int section)
{
    return fileIo[owner]->GetSectionAddr(section);
}

int
ContextIoManager::GetContextSectionSize(int owner, int section)
{
    return fileIo[owner]->GetSectionSize(section);
}

void
ContextIoManager::SetCallbackFunc(EventSmartPtr callback)
{
    flushCallback = callback;
}

void
ContextIoManager::TestCallbackFunc(AsyncMetaFileIoCtx* ctx, IOTYPE type, int cnt)
{
    // only for UT
    if (type == IOTYPE_READ)
    {
        numFilesReading = cnt;
        _LoadCompletedThenCB(ctx);
    }
    else if (type == IOTYPE_FLUSH)
    {
        numFilesFlushing = cnt;
        _FlushCompletedThenCB(ctx);
    }
    else if (type == IOTYPE_REBUILDFLUSH)
    {
        numRebuildFlushInProgress = cnt;
        _RebuildFlushCompletedThenCB(ctx);
    }
    else
    {
        numFilesFlushing = cnt;
        _WaitPendingIo(IOTYPE_ALL);
    }
}

int
ContextIoManager::_FindOwner(uint32_t signature)
{
    if (signature == RebuildCtx::SIG_REBUILD_CTX)
        return REBUILD_CTX;
    else if (signature == SegmentCtx::SIG_SEGMENT_CTX)
        return SEGMENT_CTX;
    else if (signature == AllocatorCtx::SIG_ALLOCATOR_CTX)
        return ALLOCATOR_CTX;
    else
        return -1;
}
} // namespace pos
