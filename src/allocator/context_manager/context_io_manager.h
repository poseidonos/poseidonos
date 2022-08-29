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

#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"
#include "src/allocator/context_manager/allocator_file_io.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"
#include "src/allocator/include/allocator_const.h"

namespace pos
{
class TelemetryPublisher;
class AllocatorFileIo;

class ContextIoManager
{
public:
    enum IOTYPE
    {
        IOTYPE_READ = 1,
        IOTYPE_FLUSH = 2,
        IOTYPE_REBUILD_FLUSH = 4,
        IOTYPE_ALL = (IOTYPE_READ | IOTYPE_FLUSH | IOTYPE_REBUILD_FLUSH),
    };
    ContextIoManager(void) = default;
    ContextIoManager(AllocatorAddressInfo* info, TelemetryPublisher* tp,
        AllocatorFileIo* segmentFileIo, AllocatorFileIo* allocatorFileIo, AllocatorFileIo* rebuildFileIo);
    ContextIoManager(AllocatorAddressInfo* info, TelemetryPublisher* tp, EventScheduler* scheduler,
        AllocatorFileIo* segmentFileIo, AllocatorFileIo* allocatorFileIo, AllocatorFileIo* rebuildFileIo);
    virtual ~ContextIoManager(void);

    virtual void Init(void);
    virtual void Dispose(void);

    virtual int FlushContexts(EventSmartPtr callback, bool sync,
        char* externalBuf = nullptr);

    virtual void WaitPendingIo(IOTYPE type);

    virtual uint64_t GetStoredContextVersion(int owner);

    virtual char* GetContextSectionAddr(int owner, int section);
    virtual int GetContextSectionSize(int owner, int section);

private:
    void _FlushCompleted(void);

    int _GetNumFilesReading(void);
    int _GetNumFilesFlushing(void);
    int _GetNumRebuildFlush(void);
    uint32_t _GetPendingIoCount(uint32_t checkType);

    std::atomic<bool> flushInProgress;

    EventSmartPtr flushCallback;

    AllocatorFileIo* fileIo[NUM_FILES];

    AllocatorAddressInfo* addrInfo;
    TelemetryPublisher* telPublisher;
    EventScheduler* eventScheduler;
};

} // namespace pos
