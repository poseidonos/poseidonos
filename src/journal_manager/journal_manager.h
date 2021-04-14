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

#include "src/journal_service/journal_service.h"
#include "src/journal_service/i_journal_writer.h"
#include "src/array_models/interface/i_mount_sequence.h"
#include "src/include/smart_ptr_type.h"

namespace pos
{
enum JournalManagerStatus
{
    JOURNAL_INVALID,
    JOURNAL_INIT,
    WAITING_TO_BE_REPLAYED,
    REPLAYING_JOURNAL,
    JOURNALING,
    JOURNAL_BROKEN
};

class JournalConfiguration;
class JournalLogBuffer;

class LogWriteContextFactory;

class LogWriteHandler;
class JournalVolumeEventHandler;

class BufferOffsetAllocator;
class LogGroupReleaser;

class ReplayHandler;

class JournalStatusProvider;

class DirtyMapManager;
class LogBufferWriteDoneNotifier;

class Stripe;

class IVSAMap;
class IStripeMap;
class IMapFlush;

class IBlockAllocator;
class IWBStripeAllocator;
class IWBStripeCtx;
class ISegmentCtx;
class IAllocatorCtx;

class IArrayInfo;
class IStateControl;

class JournalManager : public IMountSequence, public IJournalWriter
{
public:
    JournalManager(void);
    JournalManager(IArrayInfo* info, IStateControl* iState);
    JournalManager(JournalConfiguration* config,
        JournalStatusProvider* journalStatusProvider,
        LogWriteContextFactory* logWriteContextFactory,
        LogWriteHandler* writeHandler,
        JournalVolumeEventHandler* journalVolumeEventHandler,
        JournalLogBuffer* journalLogBuffer,
        BufferOffsetAllocator* bufferOffsetAllocator,
        LogGroupReleaser* groupReleaser,
        DirtyMapManager* dirtyManager,
        LogBufferWriteDoneNotifier* logBufferWriteDoneNotifier,
        ReplayHandler* replayHandler,
        IArrayInfo* arrayInfo, JournalService* service);
    virtual ~JournalManager(void);

    virtual bool IsEnabled(void);
    virtual int AddBlockMapUpdatedLog(VolumeIoSmartPtr volumeIo, MpageList dirty,
        EventSmartPtr callbackEvent) override;
    virtual int AddStripeMapUpdatedLog(Stripe* stripe, StripeAddr oldAddr,
        MpageList dirty, EventSmartPtr callbackEvent) override;

    virtual int Init(void) override;
    virtual void Dispose(void) override;

    int Init(IVSAMap* vsaMap, IStripeMap* stripeMap,
        IMapFlush* mapFlush, IBlockAllocator* blockAllocator,
        IWBStripeAllocator* wbStripeAllocator,
        IWBStripeCtx* wbStripeCtx, ISegmentCtx* segmentCtx,
        IAllocatorCtx* allocatorCtx);

    JournalManagerStatus
    GetJournalManagerStatus(void)
    {
        return journalManagerStatus;
    }

protected:
    void _InitModules(IVSAMap* vsaMap, IStripeMap* stripeMap,
        IMapFlush* mapFlush, IBlockAllocator* blockAllocator,
        IWBStripeAllocator* wbStripeAllocator,
        IWBStripeCtx* wbStripeCtx, ISegmentCtx* segmentCtx,
        IAllocatorCtx* allocatorCtx);
    int _Init(void);

    int _Reset(void);
    int _DoRecovery(void);

    void _InitModules(void);
    void _ResetModules(void);

    int _CanJournalBeWritten(void);

    void _RegisterServices(void);
    void _UnregisterServices(void);

    IArrayInfo* arrayInfo;
    JournalService* journalService;

    JournalConfiguration* config;
    JournalStatusProvider* statusProvider;
    JournalManagerStatus journalManagerStatus;

    JournalLogBuffer* logBuffer;

    LogWriteContextFactory* logFactory;
    LogWriteHandler* logWriteHandler;
    JournalVolumeEventHandler* volumeEventHandler;

    BufferOffsetAllocator* bufferAllocator;
    LogGroupReleaser* logGroupReleaser;
    DirtyMapManager* dirtyMapManager;
    LogBufferWriteDoneNotifier* logFilledNotifier;

    ReplayHandler* replayHandler;
};

} // namespace pos
