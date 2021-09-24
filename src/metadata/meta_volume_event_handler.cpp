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

#include "src/metadata/meta_volume_event_handler.h"

#include "src/allocator/allocator.h"
#include "src/journal_manager/journal_manager.h"
#include "src/sys_event/volume_event_publisher.h"

namespace pos
{
MetaVolumeEventHandler::MetaVolumeEventHandler(IArrayInfo* info,
    IMapperVolumeEventHandler* mapperVolumeEventHandler,
    Allocator* allocator,
    IJournalVolumeEventHandler* journal)
: VolumeEvent("Metadata", info->GetName(), info->GetIndex()),
  arrayInfo(info),
  mapper(mapperVolumeEventHandler),
  allocator(allocator),
  journal(journal)
{
    VolumeEventPublisherSingleton::Instance()->RegisterSubscriber(this, arrayInfo->GetName(), arrayInfo->GetIndex());
}

MetaVolumeEventHandler::~MetaVolumeEventHandler(void)
{
    VolumeEventPublisherSingleton::Instance()->RemoveSubscriber(this, arrayInfo->GetName(), arrayInfo->GetIndex());
}

bool
MetaVolumeEventHandler::VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    bool result = mapper->VolumeCreated(volEventBase->volId, volEventBase->volSizeByte);
    return result;
}

bool
MetaVolumeEventHandler::VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    bool result = mapper->VolumeMounted(volEventBase->volId, volEventBase->volSizeByte);
    return result;
}

bool
MetaVolumeEventHandler::VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    bool result = mapper->VolumeLoaded(volEventBase->volId, volEventBase->volSizeByte);
    return result;
}

bool
MetaVolumeEventHandler::VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    // Do nothing
    return true;
}

bool
MetaVolumeEventHandler::VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    bool result = allocator->FinalizeActiveStripes(volEventBase->volId);
    if (result == true)
    {
        bool flushMapRequired = (journal == nullptr);
        result = mapper->VolumeUnmounted(volEventBase->volId, flushMapRequired);
    }
    return result;
}

bool
MetaVolumeEventHandler::VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    int result = 0;

    // Set Volume Map Deleting State and invalidate all blocks
    result = mapper->PrepareVolumeDelete(volEventBase->volId);
    if (result != 0)
    {
        return false;
    }

    if (journal != nullptr)
    {
        // Write log for deleted volume
        result = journal->WriteVolumeDeletedLog(volEventBase->volId);
        if (result != 0)
        {
            return false;
        }

        // Trigger flushing metadata
        result = journal->TriggerMetadataFlush();
        if (result != 0)
        {
            return false;
        }
    }

    // Delete volume map
    result = mapper->DeleteVolumeMap(volEventBase->volId);
    if (result != 0)
    {
        return false;
    }

    return true;
}

void
MetaVolumeEventHandler::VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo)
{
    mapper->VolumeDetached(volList);
}

} // namespace pos
