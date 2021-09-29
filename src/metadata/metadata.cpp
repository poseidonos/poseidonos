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

#include "src/allocator/allocator.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/journal_manager/journal_manager.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/meta_service/meta_service.h"
#include "src/metadata/meta_updater.h"
#include "src/metadata/meta_volume_event_handler.h"

namespace pos
{
Metadata::Metadata(void)
: arrayInfo(nullptr),
  mapper(nullptr),
  allocator(nullptr),
  journal(nullptr),
  metaUpdater(nullptr),
  volumeEventHandler(nullptr)
{
}

Metadata::Metadata(TelemetryPublisher* tp, IArrayInfo* info, IStateControl* state)
: Metadata(info,
      new Mapper(info, nullptr),
      new Allocator(tp, info, state),
      new JournalManager(info, state))
{
}

Metadata::Metadata(IArrayInfo* info, Mapper* mapper, Allocator* allocator,
    JournalManager* journal)
: arrayInfo(info),
  mapper(mapper),
  allocator(allocator),
  journal(journal),
  metaUpdater(nullptr)
{
    volumeEventHandler = new MetaVolumeEventHandler(arrayInfo,
        mapper->GetVolumeEventHandler(),
        allocator,
        (journal->IsEnabled() ? journal->GetVolumeEventHandler() : nullptr));
}

Metadata::~Metadata(void)
{
    if (journal != nullptr)
    {
        delete journal;
    }

    if (allocator != nullptr)
    {
        delete allocator;
    }

    if (mapper != nullptr)
    {
        delete mapper;
    }

    if (metaUpdater != nullptr)
    {
        delete metaUpdater;
    }

    if (volumeEventHandler != nullptr)
    {
        delete volumeEventHandler;
    }
}

int
Metadata::Init(void)
{
    int eventId = static_cast<int>(POS_EVENT_ID::ARRAY_MOUNTING);
    int result = 0;

    std::string arrayName = arrayInfo->GetName();

    POS_TRACE_INFO(eventId, "Start initializing mapper of array {}", arrayName);
    result = mapper->Init();
    if (result != 0)
    {
        POS_TRACE_ERROR(eventId, "[Metadata Error!!] Failed to Init Mapper, array {}", arrayName);
        mapper->Dispose();
        return result;
    }

    POS_TRACE_INFO(eventId, "Start initializing allocator of array {}", arrayName);
    result = allocator->Init();
    if (result != 0)
    {
        POS_TRACE_ERROR(eventId, "[Metadata Error!!] Failed to Init Allocator, array {}", arrayName);
        allocator->Dispose();
        mapper->Dispose();
        return result;
    }

    // MetaUpdater should be registered before journal initialized
    //  as journal might request stripe flush and meta udpate in journal recovery
    // Meta update will be re-tried when journal is not ready
    // TODO (huijeong.kim) Split journal initialization and recovery
    _RegisterMetaServices();

    POS_TRACE_INFO(eventId, "Start initializing journal of array {}", arrayName);
    result = journal->Init();
    if (result != 0)
    {
        POS_TRACE_ERROR(eventId, "[Metadata Error!!] Failed to Init Journal, array {}", arrayName);
        journal->Dispose();
        allocator->Dispose();
        mapper->Dispose();
        return result;
    }

    return result;
}

void
Metadata::_RegisterMetaServices(void)
{
    metaUpdater = new MetaUpdater(mapper->GetIVSAMap(),
        mapper->GetIStripeMap(),
        allocator->GetIContextManager(),
        allocator->GetIBlockAllocator(),
        allocator->GetIWBStripeAllocator(),
        journal,
        journal->GetJournalWriter(),
        EventSchedulerSingleton::Instance(),
        arrayInfo);

    MetaServiceSingleton::Instance()->Register(arrayInfo->GetName(),
       arrayInfo->GetIndex(), metaUpdater, journal->GetJournalStatusProvider());
}

void
Metadata::Dispose(void)
{
    int eventId = static_cast<int>(POS_EVENT_ID::ARRAY_UNMOUNTING);
    std::string arrayName = arrayInfo->GetName();

    POS_TRACE_INFO(eventId, "Start disposing allocator of array {}", arrayName);
    allocator->Dispose();

    POS_TRACE_INFO(eventId, "Start disposing mapper of array {}", arrayName);
    mapper->Dispose();

    POS_TRACE_INFO(eventId, "Start disposing journal of array {}", arrayName);
    journal->Dispose();
}

void
Metadata::Shutdown(void)
{
    int eventId = static_cast<int>(POS_EVENT_ID::ARRAY_UNMOUNTING);
    std::string arrayName = arrayInfo->GetName();

    POS_TRACE_INFO(eventId, "Start shutdown allocator of array {}", arrayName);
    allocator->Shutdown();

    POS_TRACE_INFO(eventId, "Start shutdown mapper of array {}", arrayName);
    mapper->Shutdown();

    POS_TRACE_INFO(eventId, "Start shutdown journal of array {}", arrayName);
    journal->Shutdown();
}

void
Metadata::Flush(void)
{
    // no-op for IMountSequence
}

bool
Metadata::NeedRebuildAgain(void)
{
    return allocator->GetIContextManager()->NeedRebuildAgain();
}

int
Metadata::PrepareRebuild(void)
{
    return allocator->GetIWBStripeAllocator()->PrepareRebuild();
}

void
Metadata::StopRebuilding(void)
{
    allocator->GetIContextManager()->StopRebuilding();
}

} // namespace pos
