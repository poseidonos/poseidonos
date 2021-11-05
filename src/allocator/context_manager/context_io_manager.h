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

class ContextIoManager
{
public:
    enum IOTYPE
    {
        IOTYPE_READ,
        IOTYPE_FLUSH,
        IOTYPE_REBUILDFLUSH,
        IOTYPE_ALL
    };
    ContextIoManager(void) = default;
    ContextIoManager(AllocatorAddressInfo* info, TelemetryPublisher* tp);
    ContextIoManager(AllocatorAddressInfo* info, TelemetryPublisher* tp, EventScheduler* scheduler);
    virtual ~ContextIoManager(void);

    void SetAllocatorFileIo(int owner, AllocatorFileIo* io);

    virtual void Init(void);
    virtual void Dispose(void);

    virtual int FlushContexts(EventSmartPtr callback, bool sync);
    virtual int FlushRebuildContext(EventSmartPtr callback, bool sync);

    virtual uint64_t GetStoredContextVersion(int owner);

    virtual char* GetContextSectionAddr(int owner, int section);
    virtual int GetContextSectionSize(int owner, int section);

    // for UT
    void SetCallbackFunc(EventSmartPtr callback);
    void TestCallbackFunc(AsyncMetaFileIoCtx* ctx, IOTYPE type, int cnt);

private:
    int _FindOwner(uint32_t signature);

    void _LoadCompletedThenCB(AsyncMetaFileIoCtx* ctx);
    void _FlushCompletedThenCB(AsyncMetaFileIoCtx* ctx);
    void _RebuildFlushCompletedThenCB(AsyncMetaFileIoCtx* ctx);

    void _WaitPendingIo(IOTYPE type);

    std::atomic<bool> flushInProgress;
    std::atomic<int> numFilesFlushing;
    std::atomic<int> numFilesReading;

    std::atomic<int> numRebuildFlushInProgress;

    EventSmartPtr flushCallback;

    AllocatorFileIo* fileIo[NUM_FILES];

    AllocatorAddressInfo* addrInfo;
    TelemetryPublisher* telPublisher;
    EventScheduler* eventScheduler;
};

} // namespace pos
