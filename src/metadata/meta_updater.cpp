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

#include "src/metadata/meta_updater.h"

#include "src/array_models/interface/i_array_info.h"
#include "src/bio/volume_io.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/gc/gc_stripe_manager.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/metadata/block_map_update.h"
#include "src/metadata/gc_map_update.h"
#include "src/metadata/meta_event_factory.h"
#include "src/metadata/stripe_map_update.h"

namespace pos
{
MetaUpdater::MetaUpdater(IVSAMap* vsaMap, IStripeMap* stripeMap,
    IContextManager* contextManager,
    IBlockAllocator* blockAllocator, IWBStripeAllocator* wbStripeAllocator,
    IJournalManager* journal, IJournalWriter* journalWriter,
    EventScheduler* eventScheduler, IArrayInfo* arrayInfo)
: MetaUpdater(vsaMap, stripeMap, contextManager, blockAllocator, wbStripeAllocator,
      journal, journalWriter, eventScheduler,
      new MetaEventFactory(vsaMap, stripeMap, blockAllocator, wbStripeAllocator, contextManager, arrayInfo), arrayInfo)
{
}

MetaUpdater::MetaUpdater(IVSAMap* vsaMap, IStripeMap* stripeMap,
    IContextManager* contextManager,
    IBlockAllocator* blockAllocator, IWBStripeAllocator* wbStripeAllocator,
    IJournalManager* journal, IJournalWriter* journalWriter,
    EventScheduler* eventScheduler, MetaEventFactory* eventFactory, IArrayInfo* arrayInfo)
: vsaMap(vsaMap),
  stripeMap(stripeMap),
  contextManager(contextManager),
  blockAllocator(blockAllocator),
  wbStripeAllocator(wbStripeAllocator),
  journal(journal),
  journalWriter(journalWriter),
  eventScheduler(eventScheduler),
  metaEventFactory(eventFactory),
  arrayInfo(arrayInfo)
{
}

MetaUpdater::~MetaUpdater(void)
{
    if (metaEventFactory != nullptr)
    {
        delete metaEventFactory;
        metaEventFactory = nullptr;
    }
}

int
MetaUpdater::UpdateBlockMap(VolumeIoSmartPtr volumeIo, CallbackSmartPtr callback)
{
    int result = 0;

    CallbackSmartPtr blockMapUpdate =
        metaEventFactory->CreateBlockMapUpdateEvent(volumeIo);
    blockMapUpdate->SetCallee(callback);

    if (journal->IsEnabled() == true)
    {
        MpageList dirty = _GetDirtyPages(volumeIo);
        result = journalWriter->AddBlockMapUpdatedLog(volumeIo, dirty, blockMapUpdate);
    }
    else
    {
        // TODO (huijeong.kim) change to use ExecuteOrScheduleEvent
        bool executedSuccessfully = blockMapUpdate->Execute();
        if (unlikely(false == executedSuccessfully))
        {
            POS_EVENT_ID eventId =
                POS_EVENT_ID::WRCMP_MAP_UPDATE_FAILED;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                PosEventId::GetString(eventId));
            eventScheduler->EnqueueEvent(blockMapUpdate);
        }
    }
    return result;
}

MpageList
MetaUpdater::_GetDirtyPages(VolumeIoSmartPtr volumeIo)
{
    uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    BlkAddr rba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    MpageList dirty = vsaMap->GetDirtyVsaMapPages(volumeIo->GetVolumeId(), rba, blockCount);
    return dirty;
}

MpageList
MetaUpdater::_GetDirtyPages(BlkAddr rba, uint32_t volId)
{
    uint32_t blockCount = 1;
    MpageList dirty = vsaMap->GetDirtyVsaMapPages(volId, rba, blockCount);
    return dirty;
}

int
MetaUpdater::UpdateStripeMap(Stripe* stripe, CallbackSmartPtr callback)
{
    int result = 0;

    CallbackSmartPtr stripeMapUpdate(new StripeMapUpdate(stripe, stripeMap, contextManager));
    stripeMapUpdate->SetCallee(callback);

    if (journal->IsEnabled() == true)
    {
        MpageList dirty = stripeMap->GetDirtyStripeMapPages(stripe->GetVsid());
        StripeAddr oldAddr = stripeMap->GetLSA(stripe->GetVsid());

        result = journalWriter->AddStripeMapUpdatedLog(stripe, oldAddr,
            dirty, stripeMapUpdate);
    }
    else
    {
        bool executionSuccessful = stripeMapUpdate->Execute();
        if (unlikely(false == executionSuccessful))
        {
            POS_EVENT_ID eventId =
                POS_EVENT_ID::NFLSH_STRIPE_DEBUG_UPDATE;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                PosEventId::GetString(eventId));

            eventScheduler->EnqueueEvent(stripeMapUpdate);
        }
    }
    return result;
}

int
MetaUpdater::UpdateGcMap(Stripe* stripe, GcStripeMapUpdateList mapUpdateInfoList, std::map<SegmentId, uint32_t> invalidSegCnt, CallbackSmartPtr callback)
{
    int result = 0;

    CallbackSmartPtr gcMapUpdate =
        metaEventFactory->CreateGcMapUpdateEvent(stripe, mapUpdateInfoList, invalidSegCnt);
    gcMapUpdate->SetCallee(callback);

    if (journal->IsEnabled() == true)
    {
        StripeId stripeId = stripe->GetVsid();
        MpageList volumeDirtyList;
        uint32_t totalBlksPerUserStripe = arrayInfo->GetSizeInfo(PartitionType::USER_DATA)->blksPerStripe;

        for (uint32_t offset = 0; offset < totalBlksPerUserStripe; offset++)
        {
            BlkAddr rba;
            uint32_t volId;
            std::tie(rba, volId) = stripe->GetReverseMapEntry(offset);
            MpageList dirty = _GetDirtyPages(rba, volId);
            volumeDirtyList.insert(dirty.begin(), dirty.end());
        }
        uint32_t volumeId = mapUpdateInfoList.volumeId;
        MapPageList dirtyMap;
        dirtyMap[volumeId] = volumeDirtyList;
        dirtyMap[STRIPE_MAP_ID] = stripeMap->GetDirtyStripeMapPages(stripeId);

        result = journalWriter->AddGcStripeFlushedLog(mapUpdateInfoList, dirtyMap, gcMapUpdate);
    }
    else
    {
        bool executionSuccessful = gcMapUpdate->Execute();
        if (unlikely(false == executionSuccessful))
        {
            POS_EVENT_ID eventId =
                POS_EVENT_ID::NFLSH_STRIPE_DEBUG_UPDATE;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                PosEventId::GetString(eventId));

            eventScheduler->EnqueueEvent(gcMapUpdate);
        }
    }
    return result;
}

} // namespace pos
