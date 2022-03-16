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

#include "src/metadata/meta_volume_event_handler.h"

#include "src/allocator/allocator.h"
#include "src/journal_manager/journal_manager.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/logger/logger.h"
#include "src/include/pos_event_id.h"

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

int
MetaVolumeEventHandler::VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    int result = mapper->VolumeCreated(volEventBase->volId, volEventBase->volSizeByte);
    return result;
}

int
MetaVolumeEventHandler::VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    int result = mapper->VolumeMounted(volEventBase->volId, volEventBase->volSizeByte);
    return result;
}

int
MetaVolumeEventHandler::VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    int result = mapper->VolumeLoaded(volEventBase->volId, volEventBase->volSizeByte);
    return result;
}

int
MetaVolumeEventHandler::VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    // Do nothing
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

int
MetaVolumeEventHandler::VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    int result;
    IWBStripeAllocator* wbStripeManager = allocator->GetIWBStripeAllocator();
    POS_TRACE_INFO(EID(VOL_EVENT_OK),
        "Start flushign pending stripes of volume {}", volEventBase->volId);
    result = wbStripeManager->FlushAllPendingStripesInVolume(volEventBase->volId);
    if (result == 0)
    {
        bool flushMapRequired = (journal == nullptr);
        result = mapper->VolumeUnmounted(volEventBase->volId, flushMapRequired);
    }
    else
    {
        result = (int)POS_EVENT_ID::VOL_EVENT_FAIL;
    }
    return result;
}

int
MetaVolumeEventHandler::VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    int result = 0;

    // Set Volume Map Deleting State
    result = mapper->PrepareVolumeDelete(volEventBase->volId);
    if (result != 0)
    {
        return (int)POS_EVENT_ID::VOL_EVENT_FAIL;
    }

    // Invalidate all blocks in the volume
    result = mapper->InvalidateAllBlocksTo(volEventBase->volId, allocator->GetISegmentCtx());
    if (result != 0)
    {
        return result;
    }

    POS_TRACE_INFO(EID(VOL_EVENT_OK),
        "Mapper has successfully invalidated all blocks in the volume {}", volEventBase->volId);

    if (journal != nullptr)
    {
        // Write log for deleted volume
        result = journal->WriteVolumeDeletedLog(volEventBase->volId);
        if (result != 0)
        {
            return (int)POS_EVENT_ID::VOL_EVENT_FAIL;
        }

        // Trigger flushing metadata
        result = journal->TriggerMetadataFlush();
        if (result != 0)
        {
            return (int)POS_EVENT_ID::VOL_EVENT_FAIL;
        }
    }

    // Delete volume map
    result = mapper->DeleteVolumeMap(volEventBase->volId);
    if (result != 0)
    {
        return (int)POS_EVENT_ID::VOL_EVENT_FAIL;
    }

    POS_TRACE_INFO(EID(VOL_EVENT_OK),
        "Mapper has successfully deleted the map of volume {}", volEventBase->volId);

    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

int
MetaVolumeEventHandler::VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo)
{
    int result = mapper->VolumeDetached(volList);

    return result;
}

} // namespace pos
