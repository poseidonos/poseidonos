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

#include "src/metadata/metadata.h"

#include <string>

#include "src/master_context/config_manager.h"
#include "src/allocator/allocator.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/journal_manager/journal_manager.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/meta_service/meta_service.h"
#include "src/metadata/meta_event_factory.h"
#include "src/metadata/meta_updater.h"
#include "src/metadata/meta_volume_event_handler.h"
#include "src/metadata/segment_context_updater.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"
#include "src/volume/volume_service.h"

namespace pos
{
Metadata::Metadata(IArrayInfo* info, IStateControl* state)
: Metadata(info,
      new Mapper(info, nullptr),
      new Allocator(info, state),
      new JournalManager(info, state),
      MetaFsServiceSingleton::Instance()->GetMetaFs(info->GetIndex())->ctrl,
      MetaServiceSingleton::Instance())
{
}

Metadata::Metadata(IArrayInfo* info, Mapper* mapper, Allocator* allocator,
    JournalManager* journal, MetaFsFileControlApi* metaFsCtrl, MetaService* service)
: arrayInfo(info),
  mapper(mapper),
  allocator(allocator),
  journal(journal),
  metaFsCtrl(metaFsCtrl),
  volumeEventHandler(nullptr),
  metaService(service),
  metaUpdater(nullptr),
  segmentContextUpdater(nullptr),
  metaEventFactory(nullptr)
{
    volumeEventHandler = new MetaVolumeEventHandler(arrayInfo,
        mapper->GetVolumeEventHandler(),
        allocator,
        (journal->IsEnabled() ? journal->GetVolumeEventHandler() : nullptr));

    auto sizeInfo = info->GetSizeInfo(PartitionType::USER_DATA);
    segmentContextUpdater = new SegmentContextUpdater(allocator->GetISegmentCtx(), journal->GetVersionedSegmentContext(), sizeInfo);

    metaEventFactory = new MetaEventFactory(
        mapper->GetIVSAMap(),
        mapper->GetIStripeMap(),
        segmentContextUpdater,
        allocator->GetIWBStripeAllocator(),
        allocator->GetIContextManager(),
        arrayInfo);

    metaUpdater = new MetaUpdater(
        mapper->GetIStripeMap(),
        journal,
        journal->GetJournalWriter(),
        EventSchedulerSingleton::Instance(),
        metaEventFactory,
        arrayInfo);

    _SetGCThreshold();
}

Metadata::~Metadata(void)
{
    if (journal != nullptr)
    {
        delete journal;
        journal = nullptr;
    }

    if (allocator != nullptr)
    {
        delete allocator;
        allocator = nullptr;
    }

    if (mapper != nullptr)
    {
        delete mapper;
        mapper = nullptr;
    }

    if (volumeEventHandler != nullptr)
    {
        delete volumeEventHandler;
        volumeEventHandler = nullptr;
    }

    if (metaEventFactory != nullptr)
    {
        delete metaEventFactory;
        metaEventFactory = nullptr;
    }

    if (segmentContextUpdater != nullptr)
    {
        delete segmentContextUpdater;
        segmentContextUpdater = nullptr;
    }

    if (metaUpdater != nullptr)
    {
        delete metaUpdater;
        metaUpdater = nullptr;
    }
}

int
Metadata::Init(void)
{
    int eventId = EID(MOUNT_ARRAY_DEBUG_MSG);
    int result = 0;

    std::string arrayName = arrayInfo->GetName();

    POS_TRACE_INFO(eventId, "Start initializing mapper of array {}", arrayName);
    result = mapper->Init();
    if (result != 0)
    {
        POS_TRACE_ERROR(eventId, "[Metadata Error!!] Failed to Init Mapper, array {}", arrayName);
        return result;
    }

    POS_TRACE_INFO(eventId, "Start initializing allocator of array {}", arrayName);
    result = allocator->Init();
    if (result != 0)
    {
        POS_TRACE_ERROR(eventId, "[Metadata Error!!] Failed to Init Allocator, array {}", arrayName);
        return result;
    }

    allocator->PrepareVersionedSegmentCtx(journal->GetVersionedSegmentContext());

    // MetaUpdater should be registered before journal initialized
    //  as journal might request stripe flush and meta udpate in journal recovery
    // Meta update will be re-tried when journal is not ready
    // TODO (huijeong.kim) Split journal initialization and recovery
    metaService->Register(arrayInfo->GetName(),
        arrayInfo->GetIndex(), metaUpdater, journal->GetJournalStatusProvider());

    POS_TRACE_INFO(eventId, "Start initializing journal of array {}", arrayName);
    result = journal->Init(
        mapper->GetIVSAMap(),
        mapper->GetIStripeMap(),
        mapper->GetIMapFlush(),
        allocator->GetISegmentCtx(),
        allocator->GetIWBStripeAllocator(),
        allocator->GetIContextManager(),
        allocator->GetIContextReplayer(),
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayInfo->GetIndex()),
        metaFsCtrl,
        EventSchedulerSingleton::Instance(),
        TelemetryClientSingleton::Instance());

    if (result != 0)
    {
        POS_TRACE_ERROR(eventId, "[Metadata Error!!] Failed to Init Journal, array {}", arrayName);
        return result;
    }

    return result;
}

void
Metadata::Dispose(void)
{
    int eventId = EID(UNMOUNT_ARRAY_DEBUG_MSG);
    std::string arrayName = arrayInfo->GetName();

    POS_TRACE_INFO(eventId, "Start disposing allocator of array {}", arrayName);
    allocator->Dispose();

    POS_TRACE_INFO(eventId, "Start disposing mapper of array {}", arrayName);
    mapper->Dispose();

    POS_TRACE_INFO(eventId, "Start disposing journal of array {}", arrayName);
    journal->Dispose();

    metaService->Unregister(arrayInfo->GetName());
}

void
Metadata::Shutdown(void)
{
    int eventId = EID(UNMOUNT_ARRAY_DEBUG_MSG);
    std::string arrayName = arrayInfo->GetName();

    POS_TRACE_INFO(eventId, "Start shutdown allocator of array {}", arrayName);
    allocator->Shutdown();

    POS_TRACE_INFO(eventId, "Start shutdown mapper of array {}", arrayName);
    mapper->Shutdown();

    POS_TRACE_INFO(eventId, "Start shutdown journal of array {}", arrayName);
    journal->Shutdown();

    metaService->Unregister(arrayInfo->GetName());
}

void
Metadata::Flush(void)
{
    // no-op for IMountSequence
}

bool
Metadata::NeedRebuildAgain(void)
{
    IContextManager* contextManager = allocator->GetIContextManager();
    if (contextManager != nullptr)
    {
        return contextManager->NeedRebuildAgain();
    }
    else
    {
        int eventId = EID(UNKNOWN_ALLOCATOR_ERROR);
        POS_TRACE_ERROR(eventId, "Can't find context manager to check if rebuild is needed");
        return false;
    }
}

int
Metadata::PrepareRebuild(void)
{
    return allocator->PrepareRebuild();
}

void
Metadata::StopRebuilding(void)
{
    IContextManager* contextManager = allocator->GetIContextManager();
    if (contextManager != nullptr)
    {
        contextManager->StopRebuilding();
    }
    else
    {
        int eventId = EID(UNKNOWN_ALLOCATOR_ERROR);
        POS_TRACE_ERROR(eventId, "Can't find context manager to check if rebuild is needed");
    }
}

void
Metadata::_SetGCThreshold(void)
{
    uint32_t normal_gc_ratio = 0;
    uint32_t urgent_gc_ratio = 0;
    uint32_t normal_gc_lb = 0;
    uint32_t urgent_gc_lb = 0;

    int ret = ConfigManagerSingleton::Instance()->GetValue("gc_threshold", "percent_of_normal_gc_threshold_to_total_capacity",
        &normal_gc_ratio, ConfigType::CONFIG_TYPE_UINT32);
    ret += ConfigManagerSingleton::Instance()->GetValue("gc_threshold", "normal_gc_threshold_count_lower_bound",
        &normal_gc_lb, ConfigType::CONFIG_TYPE_UINT32);
    ret += ConfigManagerSingleton::Instance()->GetValue("gc_threshold", "percent_of_urgent_gc_threshold_to_normal_gc_threshold",
        &urgent_gc_ratio, ConfigType::CONFIG_TYPE_UINT32);
    ret += ConfigManagerSingleton::Instance()->GetValue("gc_threshold", "urgent_gc_threshold_count_lower_bound",
        &urgent_gc_lb, ConfigType::CONFIG_TYPE_UINT32);

    if (ret == 0)
    {
        const PartitionLogicalSize* dataPartitionSize = arrayInfo->GetSizeInfo(PartitionType::USER_DATA);
        if (dataPartitionSize != nullptr)
        {
            uint32_t normalGcThreshold = (uint32_t)(dataPartitionSize->totalSegments * normal_gc_ratio / 100);
            if (normalGcThreshold < normal_gc_lb)
            {
                normalGcThreshold = normal_gc_lb;
            }
            uint32_t urgentGcThreshold = (uint32_t)(normalGcThreshold * urgent_gc_ratio / 100);
            if (urgentGcThreshold < urgent_gc_lb)
            {
                urgentGcThreshold = urgent_gc_lb;
            }
            allocator->SetNormalGcThreshold(normalGcThreshold);
            allocator->SetUrgentThreshold(urgentGcThreshold);
            return;
        }
    }
    POS_TRACE_TRACE(EID(GC_THRESHOLD_IS_SET), "The gc threshold is not set and operates as the default value.");
}
} // namespace pos
