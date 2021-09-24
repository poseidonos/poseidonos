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

#include "src/allocator/i_context_manager.h"
#include "src/allocator/i_context_replayer.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/pos_event_id.h"
#include "src/journal_manager/checkpoint/checkpoint_manager.h"
#include "src/journal_manager/checkpoint/dirty_map_manager.h"
#include "src/journal_manager/checkpoint/log_group_releaser.h"
#include "src/journal_manager/config/journal_configuration.h"
#include "src/journal_manager/journal_writer.h"
#include "src/journal_manager/log_buffer/buffer_write_done_notifier.h"
#include "src/journal_manager/log_buffer/buffered_segment_context_manager.h"
#include "src/journal_manager/log_buffer/callback_sequence_controller.h"
#include "src/journal_manager/log_buffer/journal_log_buffer.h"
#include "src/journal_manager/log_buffer/log_write_context_factory.h"
#include "src/journal_manager/log_write/buffer_offset_allocator.h"
#include "src/journal_manager/log_write/journal_event_factory.h"
#include "src/journal_manager/log_write/journal_volume_event_handler.h"
#include "src/journal_manager/log_write/log_write_handler.h"
#include "src/journal_manager/replay/replay_handler.h"
#include "src/journal_manager/status/journal_status_provider.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/metafs/include/metafs_service.h"
#include "src/volume/volume_service.h"

namespace pos
{
JournalManager::JournalManager(void)
: arrayInfo(nullptr),
  journalService(nullptr),
  config(nullptr),
  statusProvider(nullptr),
  logBuffer(nullptr),
  logFactory(nullptr),
  eventFactory(nullptr),
  logWriteHandler(nullptr),
  volumeEventHandler(nullptr),
  journalWriter(nullptr),
  bufferAllocator(nullptr),
  logGroupReleaser(nullptr),
  checkpointManager(nullptr),
  bufferedSegCtxManager(nullptr),
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
    JournalEventFactory* journalEventFactory,
    LogWriteHandler* writeHandler,
    JournalVolumeEventHandler* journalVolumeEventHandler,
    JournalWriter* writer,
    JournalLogBuffer* journalLogBuffer,
    BufferOffsetAllocator* bufferOffsetAllocator,
    LogGroupReleaser* groupReleaser,
    CheckpointManager* cpManager,
    BufferedSegmentContextManager* bufSegCtxManager,
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
    eventFactory = journalEventFactory;

    logWriteHandler = writeHandler;
    volumeEventHandler = journalVolumeEventHandler;
    journalWriter = writer;

    logBuffer = journalLogBuffer;
    bufferAllocator = bufferOffsetAllocator;
    logGroupReleaser = groupReleaser;

    checkpointManager = cpManager;
    bufferedSegCtxManager = bufSegCtxManager;
    dirtyMapManager = dirtyManager;
    logFilledNotifier = logBufferWriteDoneNotifier;
    sequenceController = callbackSequenceController;

    replayHandler = replay;

    arrayInfo = info;
    journalService = service;
}

// Constructor for injecting mock module dependencies in product code
JournalManager::JournalManager(IArrayInfo* info, IStateControl* state)
: JournalManager(new JournalConfiguration(),
    new JournalStatusProvider(),
    new LogWriteContextFactory(),
    new JournalEventFactory(),
    new LogWriteHandler(),
    new JournalVolumeEventHandler(),
    new JournalWriter(),
    new JournalLogBuffer(info->GetIndex()),
    new BufferOffsetAllocator(),
    new LogGroupReleaser(),
    new CheckpointManager(),
    new BufferedSegmentContextManager(),
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

    delete journalWriter;
    delete volumeEventHandler;
    delete logWriteHandler;
    delete logFactory;
    delete eventFactory;

    delete checkpointManager;
    delete bufferedSegCtxManager;
    delete statusProvider;
    delete config;
}

int
JournalManager::Init(void)
{
    int arrayId = arrayInfo->GetIndex();

    // TODO (huijeong.kim) Dependency injection should be moved to the constructor
    return Init(MapperServiceSingleton::Instance()->GetIVSAMap(arrayId),
        MapperServiceSingleton::Instance()->GetIStripeMap(arrayId),
        MapperServiceSingleton::Instance()->GetIMapFlush(arrayId),
        AllocatorServiceSingleton::Instance()->GetIBlockAllocator(arrayId),
        AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(arrayId),
        AllocatorServiceSingleton::Instance()->GetIContextManager(arrayId),
        AllocatorServiceSingleton::Instance()->GetIContextReplayer(arrayId),
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayId),
        MetaFsServiceSingleton::Instance()->GetMetaFs(arrayId)->ctrl,
        EventSchedulerSingleton::Instance());
}

int
JournalManager::Init(IVSAMap* vsaMap, IStripeMap* stripeMap,
    IMapFlush* mapFlush, IBlockAllocator* blockAllocator,
    IWBStripeAllocator* wbStripeAllocator,
    IContextManager* ctxManager, IContextReplayer* ctxReplayer,
    IVolumeManager* volumeManager, MetaFsFileControlApi* metaFsCtrl,
    EventScheduler* eventScheduler)
{
    int result = 0;

    if (config->IsEnabled() == true)
    {
        journalingStatus.Set(JOURNAL_INIT);

        result = _InitConfigAndPrepareLogBuffer(metaFsCtrl);
        if (result < 0)
        {
            return result;
        }

        _InitModules(vsaMap, stripeMap, mapFlush, blockAllocator,
            wbStripeAllocator, ctxManager, ctxReplayer, volumeManager, eventScheduler);

        if (journalingStatus.Get() == WAITING_TO_BE_REPLAYED)
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
JournalManager::_InitConfigAndPrepareLogBuffer(MetaFsFileControlApi* metaFsCtrl)
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
        config->Init(loadedLogBufferSize, metaFsCtrl);

        journalingStatus.Set(WAITING_TO_BE_REPLAYED);
    }
    else
    {
        result = config->Init(0, metaFsCtrl);
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
    if (config->IsEnabled() == false || journalingStatus.Get() == JOURNALING)
    {
        return 0;
    }

    if (journalingStatus.Get() == JOURNAL_INVALID)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_MANAGER_NOT_INITIALIZED,
            "Journal manager accessed without initialization");
        return -EID(JOURNAL_REPLAY_FAILED);
    }

    if (journalingStatus.Get() == WAITING_TO_BE_REPLAYED)
    {
        journalingStatus.Set(REPLAYING_JOURNAL);

        POS_TRACE_INFO(EID(JOURNAL_REPLAY_STARTED), "Journal replay started");

        int result = replayHandler->Start();
        if (result < 0)
        {
            journalingStatus.Set(JOURNAL_BROKEN);
            return -EID(JOURNAL_REPLAY_FAILED);
        }

        _ResetModules();
        journalingStatus.Set(JOURNALING);
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
        _DisposeModules();
    }
}

void
JournalManager::_DisposeModules(void)
{
    logBuffer->Dispose();
    bufferAllocator->Dispose();
    dirtyMapManager->Dispose();
    logFilledNotifier->Dispose();
    logWriteHandler->Dispose();
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
JournalManager::Flush(void)
{
    // no-op for IMountSequence
}

void
JournalManager::_RegisterServices(void)
{
    journalService->Register(arrayInfo->GetName(), arrayInfo->GetIndex(),
        this, journalWriter, statusProvider);
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

IJournalWriter*
JournalManager::GetJournalWriter(void)
{
    return journalWriter;
}

IJournalVolumeEventHandler*
JournalManager::GetVolumeEventHandler(void)
{
    return volumeEventHandler;
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
            journalingStatus.Get());

        journalingStatus.Set(JOURNALING);
    }
    return ret;
}

void
JournalManager::_InitModules(IVSAMap* vsaMap, IStripeMap* stripeMap,
    IMapFlush* mapFlush, IBlockAllocator* blockAllocator,
    IWBStripeAllocator* wbStripeAllocator, IContextManager* contextManager,
    IContextReplayer* contextReplayer, IVolumeManager* volumeManager,
    EventScheduler* eventScheduler)
{
    logBuffer->Init(config, logFactory);

    bufferAllocator->Init(logGroupReleaser, config);
    dirtyMapManager->Init(config);
    checkpointManager->Init(mapFlush, contextManager, eventScheduler, sequenceController, dirtyMapManager);
    bufferedSegCtxManager->Init(config);

    logFactory->Init(config, logFilledNotifier, sequenceController);
    eventFactory->Init(logWriteHandler);

    // Note that bufferAllocator should be notified after dirtyMapManager,
    // and logWriteHandler should be notified after bufferAllocator
    logFilledNotifier->Register(dirtyMapManager);
    logFilledNotifier->Register(bufferAllocator);
    logFilledNotifier->Register(logWriteHandler);

    logGroupReleaser->Init(config, logFilledNotifier, logBuffer,
        checkpointManager, mapFlush, contextManager, eventScheduler);

    logWriteHandler->Init(bufferAllocator, logBuffer, config);
    volumeEventHandler->Init(logFactory, checkpointManager, logWriteHandler, config,
        contextManager, eventScheduler);
    journalWriter->Init(logWriteHandler, logFactory, eventFactory, &journalingStatus);

    replayHandler->Init(config, logBuffer, vsaMap, stripeMap, mapFlush, blockAllocator,
        wbStripeAllocator, contextManager, contextReplayer, arrayInfo, volumeManager);

    statusProvider->Init(bufferAllocator, config, logGroupReleaser);
}

void
JournalManager::_ResetModules(void)
{
    bufferAllocator->Reset();
    logGroupReleaser->Reset();
}

} // namespace pos
