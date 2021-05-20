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

#include "journal_manager.h"

#include <iostream>
#include <string>

#include "src/journal_manager/checkpoint/dirty_map_manager.h"
#include "src/journal_manager/checkpoint/log_group_releaser.h"
#include "src/journal_manager/config/journal_configuration.h"
#include "src/journal_manager/log_buffer/journal_log_buffer.h"
#include "src/journal_manager/log_buffer/log_write_context_factory.h"
#include "src/journal_manager/log_buffer/buffer_write_done_notifier.h"
#include "src/journal_manager/log_buffer/callback_sequence_controller.h"
#include "src/journal_manager/log_write/buffer_offset_allocator.h"
#include "src/journal_manager/log_write/journal_volume_event_handler.h"
#include "src/journal_manager/log_write/log_write_handler.h"
#include "src/journal_manager/replay/replay_handler.h"
#include "src/journal_manager/status/journal_status_provider.h"

#include "src/array_models/interface/i_array_info.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/allocator_service/allocator_service.h"

namespace pos
{
JournalManager::JournalManager(void)
: arrayInfo(nullptr),
  journalService(nullptr),
  config(nullptr),
  statusProvider(nullptr),
  journalManagerStatus(JOURNAL_INVALID),
  logBuffer(nullptr),
  logFactory(nullptr),
  logWriteHandler(nullptr),
  volumeEventHandler(nullptr),
  bufferAllocator(nullptr),
  logGroupReleaser(nullptr),
  dirtyMapManager(nullptr),
  logFilledNotifier(nullptr),
  sequenceController(nullptr),
  replayHandler(nullptr)
{
}

// Constructor for injecting dependencies in unit tests
JournalManager::JournalManager(JournalConfiguration* configuration,
    JournalStatusProvider* journalStatusProvider,
    LogWriteContextFactory* logWriteContextFactory,
    LogWriteHandler* writeHandler,
    JournalVolumeEventHandler* journalVolumeEventHandler,
    JournalLogBuffer* journalLogBuffer,
    BufferOffsetAllocator* bufferOffsetAllocator,
    LogGroupReleaser* groupReleaser,
    DirtyMapManager* dirtyManager,
    LogBufferWriteDoneNotifier* logBufferWriteDoneNotifier,
    CallbackSequenceController* callbackSequenceController,
    ReplayHandler* replay,
    IArrayInfo* info, JournalService* service)
: JournalManager()
{
    config = configuration;
    statusProvider = journalStatusProvider;

    logFactory = logWriteContextFactory;
    logWriteHandler = writeHandler;
    volumeEventHandler = journalVolumeEventHandler;

    logBuffer = journalLogBuffer;
    bufferAllocator = bufferOffsetAllocator;
    logGroupReleaser = groupReleaser;

    dirtyMapManager = dirtyManager;
    logFilledNotifier = logBufferWriteDoneNotifier;
    sequenceController = callbackSequenceController;

    replayHandler = replay;

    arrayInfo = info;
    journalService = service;
}

// Constructor for injecting mock module dependencies in product code
JournalManager::JournalManager(IArrayInfo* info, IStateControl* state)
: JournalManager(new JournalConfiguration(info->GetName()),
    new JournalStatusProvider(),
    new LogWriteContextFactory(),
    new LogWriteHandler(),
    new JournalVolumeEventHandler(),
    new JournalLogBuffer(info->GetName()),
    new BufferOffsetAllocator(),
    new LogGroupReleaser(),
    new DirtyMapManager(),
    new LogBufferWriteDoneNotifier(),
    new CallbackSequenceController(),
    new ReplayHandler(state),
    info, JournalServiceSingleton::Instance())
{
}

JournalManager::~JournalManager(void)
{
    delete replayHandler;

    delete sequenceController;
    delete logFilledNotifier;
    delete dirtyMapManager;

    delete logGroupReleaser;
    delete bufferAllocator;
    delete logBuffer;

    delete volumeEventHandler;
    delete logWriteHandler;
    delete logFactory;

    delete statusProvider;
    delete config;
}

int
JournalManager::Init(void)
{
    // TODO (huijeong.kim) Dependency injection should be moved to the constructor
    return Init(MapperServiceSingleton::Instance()->GetIVSAMap(arrayInfo->GetName()),
        MapperServiceSingleton::Instance()->GetIStripeMap(arrayInfo->GetName()),
        MapperServiceSingleton::Instance()->GetIMapFlush(arrayInfo->GetName()),
        AllocatorServiceSingleton::Instance()->GetIBlockAllocator(arrayInfo->GetName()),
        AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(arrayInfo->GetName()),
        AllocatorServiceSingleton::Instance()->GetIWBStripeCtx(arrayInfo->GetName()),
        AllocatorServiceSingleton::Instance()->GetISegmentCtx(arrayInfo->GetName()),
        AllocatorServiceSingleton::Instance()->GetIAllocatorCtx(arrayInfo->GetName()));
}

int
JournalManager::Init(IVSAMap* vsaMap, IStripeMap* stripeMap,
    IMapFlush* mapFlush, IBlockAllocator* blockAllocator,
    IWBStripeAllocator* wbStripeAllocator,
    IWBStripeCtx* wbStripeCtx, ISegmentCtx* segmentCtx,
    IAllocatorCtx* allocatorCtx)
{
    int result = 0;

    if (config->IsEnabled() == true)
    {
        result = _InitConfigAndPrepareLogBuffer();
        if (result < 0)
        {
            return result;
        }

        _InitModules(vsaMap, stripeMap, mapFlush, blockAllocator,
            wbStripeAllocator, wbStripeCtx, segmentCtx, allocatorCtx);

        if (journalManagerStatus == WAITING_TO_BE_REPLAYED)
        {
            result = _DoRecovery();
        }
        else
        {
            result = _Reset();
        }
    }

    if (result == 0)
    {
        _RegisterServices();
    }
    return result;
}

int
JournalManager::_InitConfigAndPrepareLogBuffer(void)
{
    int result = 0;

    bool logBufferExist = logBuffer->DoesLogFileExist();
    if (logBufferExist == true)
    {
        uint64_t loadedLogBufferSize = 0;

        result = logBuffer->Open(loadedLogBufferSize);
        if (result < 0)
        {
            return result;
        }
        config->Init(loadedLogBufferSize);

        journalManagerStatus = WAITING_TO_BE_REPLAYED;
    }
    else
    {
        result = config->Init();
        if (result == 0)
        {
            result = logBuffer->Create(config->GetLogBufferSize());
        }
    }

    return result;
}

int
JournalManager::_DoRecovery(void)
{
    if (config->IsEnabled() == false || journalManagerStatus == JOURNALING)
    {
        return 0;
    }

    if (journalManagerStatus == JOURNAL_INVALID)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_MANAGER_NOT_INITIALIZED,
            "Journal manager accessed without initialization");
        return -EID(JOURNAL_REPLAY_FAILED);
    }

    if (journalManagerStatus == WAITING_TO_BE_REPLAYED)
    {
        journalManagerStatus = REPLAYING_JOURNAL;

        POS_TRACE_INFO(EID(JOURNAL_REPLAY_STARTED), "Journal replay started");

        int result = replayHandler->Start();
        if (result < 0)
        {
            journalManagerStatus = JOURNAL_BROKEN;
            return -EID(JOURNAL_REPLAY_FAILED);
        }

        _ResetModules();
        journalManagerStatus = JOURNALING;
    }

    return 0;
}

void
JournalManager::Dispose(void)
{
    _UnregisterServices();

    if (config->IsEnabled() == true)
    {
        _Reset();
        logBuffer->Dispose();
    }
}

void
JournalManager::Shutdown(void)
{
    _UnregisterServices();
    if (config->IsEnabled() == true)
    {
        logBuffer->Dispose();
    }
}

void
JournalManager::_RegisterServices(void)
{
    std::string arrayName = arrayInfo->GetName();

    journalService->Register(arrayName, this);
    journalService->Register(arrayName, volumeEventHandler);
    journalService->Register(arrayName, statusProvider);
}

void
JournalManager::_UnregisterServices(void)
{
    journalService->Unregister(arrayInfo->GetName());
}

bool
JournalManager::IsEnabled(void)
{
    return config->IsEnabled();
}

int
JournalManager::_CanJournalBeWritten(void)
{
    if (config->IsEnabled() == false)
    {
        int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_CONFIGURATION);
        return (-1) * eventId;
    }

    if (journalManagerStatus != JOURNALING)
    {
        int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_NOT_READY);
        return eventId;
    }
    return 0;
}

int
JournalManager::AddBlockMapUpdatedLog(VolumeIoSmartPtr volumeIo,
    MpageList dirty, EventSmartPtr callbackEvent)
{
    int journalWriteStatus = _CanJournalBeWritten();
    if (journalWriteStatus != 0)
    {
        return journalWriteStatus;
    }
    else
    {
        LogWriteContext* logWriteContext =
            logFactory->CreateBlockMapLogWriteContext(volumeIo, dirty, callbackEvent);
        return logWriteHandler->AddLog(logWriteContext);
    }
}

int
JournalManager::AddStripeMapUpdatedLog(Stripe* stripe, StripeAddr oldAddr,
    MpageList dirty, EventSmartPtr callbackEvent)
{
    int result = 0;
    if ((result = _CanJournalBeWritten()) == 0)
    {
        LogWriteContext* logWriteContext =
            logFactory->CreateStripeMapLogWriteContext(stripe, oldAddr, dirty, callbackEvent);
        return logWriteHandler->AddLog(logWriteContext);
    }
    else
    {
        return result;
    }
}

int
JournalManager::AddGcStripeFlushedLog(GcStripeMapUpdateList mapUpdates,
    MapPageList dirty, EventSmartPtr callbackEvent)
{
    int result = 0;
    if ((result = _CanJournalBeWritten()) == 0)
    {
        LogWriteContext* logWriteContext =
            logFactory->CreateGcStripeFlushedLogWriteContext(mapUpdates, dirty, callbackEvent);
        return logWriteHandler->AddLog(logWriteContext);
    }
    else
    {
        return result;
    }
}
int
JournalManager::_Reset(void)
{
    _ResetModules();

    int ret = logBuffer->SyncResetAll();
    if (ret == 0)
    {
        POS_TRACE_INFO(POS_EVENT_ID::JOURNAL_MANAGER_INITIALIZED,
            "Journal manager is initialized to status {}",
            journalManagerStatus);

        journalManagerStatus = JOURNALING;
    }
    return ret;
}

void
JournalManager::_InitModules(IVSAMap* vsaMap, IStripeMap* stripeMap,
    IMapFlush* mapFlush, IBlockAllocator* blockAllocator,
    IWBStripeAllocator* wbStripeAllocator,
    IWBStripeCtx* wbStripeCtx, ISegmentCtx* segmentCtx,
    IAllocatorCtx* allocatorCtx)
{
    logBuffer->Init(config);

    bufferAllocator->Init(logGroupReleaser, config);
    dirtyMapManager->Init(config);

    logFactory->Init(logFilledNotifier, sequenceController);

    // Note that bufferAllocator should be notified after dirtyMapManager,
    // and logWriteHandler should be notified after bufferAllocator
    logFilledNotifier->Register(dirtyMapManager);
    logFilledNotifier->Register(bufferAllocator);
    logFilledNotifier->Register(logWriteHandler);

    logGroupReleaser->Init(logFilledNotifier, logBuffer, dirtyMapManager, sequenceController,
        mapFlush, allocatorCtx);

    logWriteHandler->Init(bufferAllocator, logBuffer, config);
    volumeEventHandler->Init(logFactory, dirtyMapManager, logWriteHandler, config,
        allocatorCtx);

    replayHandler->Init(config, logBuffer, vsaMap, stripeMap, mapFlush, blockAllocator,
        wbStripeAllocator, wbStripeCtx, segmentCtx, allocatorCtx, arrayInfo);

    statusProvider->Init(bufferAllocator, config, logGroupReleaser);
}

void
JournalManager::_ResetModules(void)
{
    bufferAllocator->Reset();
    logGroupReleaser->Reset();
}

} // namespace pos
