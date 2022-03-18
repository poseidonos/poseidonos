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
ContextIoManager::ContextIoManager(AllocatorAddressInfo* info, TelemetryPublisher* tp,
    AllocatorFileIo* segmentFileIo, AllocatorFileIo* allocatorFileIo, AllocatorFileIo* rebuildFileIo)
: ContextIoManager(info, tp, EventSchedulerSingleton::Instance(), segmentFileIo, allocatorFileIo, rebuildFileIo)
{
}

ContextIoManager::ContextIoManager(AllocatorAddressInfo* info, TelemetryPublisher* tp, EventScheduler* scheduler,
    AllocatorFileIo* segmentFileIo, AllocatorFileIo* allocatorFileIo, AllocatorFileIo* rebuildFileIo)
: flushInProgress(false),
  addrInfo(info),
  telPublisher(tp),
  eventScheduler(scheduler)
{
    fileIo[SEGMENT_CTX] = segmentFileIo;
    fileIo[ALLOCATOR_CTX] = allocatorFileIo;
    fileIo[REBUILD_CTX] = rebuildFileIo;
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
ContextIoManager::Init(void)
{
    int ret = 0;
    POS_TRACE_INFO(EID(ALLOCATOR_META_ASYNCLOAD), "[AllocatorLoad] start to init and load allocator files");
    for (int owner = 0; owner < NUM_FILES; owner++)
    {
        fileIo[owner]->Init();

        ret = fileIo[owner]->LoadContext();
        if (ret == 0) // new file created
        {
            AllocatorCtxIoCompletion completion = std::bind(&ContextIoManager::_FlushCompleted, this);
            ret = fileIo[owner]->Flush(completion);
            if (ret == 0)
            {
                WaitPendingIo(IOTYPE_ALL);
                POS_TRACE_INFO(EID(ALLOCATOR_META_ASYNCLOAD),
                    "[AllocatorLoad] initial flush allocator file:{}", owner);
            }
            else
            {
                break;
            }
        }
        else if (ret == 1) // loading
        {
            WaitPendingIo(IOTYPE_READ);
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

    for (int owner = 0; owner < NUM_ALLOCATOR_FILES; owner++)
    {
        AllocatorCtxIoCompletion completion = std::bind(&ContextIoManager::_FlushCompleted, this);
        ret = fileIo[owner]->Flush(completion);
        if (ret != 0)
        {
            break;
        }
    }
    POSMetricValue v;
    v.gauge = _GetNumFilesFlushing();
    telPublisher->PublishData(TEL30001_ALCT_ALCTX_PENDINGIO_CNT, v, MT_GAUGE);

    if (sync == true)
    {
        for (int owner = 0; owner < NUM_ALLOCATOR_FILES; owner++)
        {
            WaitPendingIo(IOTYPE_FLUSH);
        }
    }
    return ret;
}

void
ContextIoManager::WaitPendingIo(IOTYPE type)
{
    while (type == IOTYPE_ALL)
    {
        if ((_GetNumFilesReading() + _GetNumFilesFlushing() + _GetNumRebuildFlush() == 0) || (addrInfo->IsUT() == true))
        {
            return;
        }
        usleep(1);
    }
    while (type == IOTYPE_READ)
    {
        if ((_GetNumFilesReading() == 0) || (addrInfo->IsUT() == true))
        {
            return;
        }
        usleep(1);
    }
    while (type == IOTYPE_FLUSH)
    {
        if ((_GetNumFilesFlushing() == 0) || (addrInfo->IsUT() == true))
        {
            return;
        }
        usleep(1);
    }
}

void
ContextIoManager::_FlushCompleted(void)
{
    int remaining = _GetNumFilesFlushing();
    POSMetricValue v;
    v.gauge = remaining;
    telPublisher->PublishData(TEL30001_ALCT_ALCTX_PENDINGIO_CNT, v, MT_GAUGE);

    if (remaining == 0 && flushInProgress.exchange(false) == true)
    {
        POS_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_STORE), "[AllocatorFlush] Complete to flush allocator files");
        if (flushCallback != nullptr)
        {
            eventScheduler->EnqueueEvent(flushCallback);
            flushCallback = nullptr;
        }
    }
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

int
ContextIoManager::_GetNumFilesReading(void)
{
    int numFilesReading = 0;

    for (int owner = 0; owner < NUM_ALLOCATOR_FILES; owner++)
    {
        numFilesReading += fileIo[owner]->GetNumFilesReading();
    }

    return numFilesReading;
}

int
ContextIoManager::_GetNumFilesFlushing(void)
{
    int numFilesFlushing = 0;
    for (int owner = 0; owner < NUM_ALLOCATOR_FILES; owner++)
    {
        numFilesFlushing += fileIo[owner]->GetNumFilesFlushing();
    }

    return numFilesFlushing;
}

int
ContextIoManager::_GetNumRebuildFlush(void)
{
    return fileIo[REBUILD_CTX]->GetNumFilesFlushing();
}

} // namespace pos
