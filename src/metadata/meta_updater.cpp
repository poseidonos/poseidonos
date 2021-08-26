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

#include "src/metadata/meta_updater.h"
#include "src/metadata/block_map_update.h"
#include "src/bio/volume_io.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

namespace pos
{
MetaUpdater::MetaUpdater(IVSAMap* vsaMap, IStripeMap* stripeMap,
    IBlockAllocator* blockAllocator, IWBStripeAllocator* wbStripeAllocator,
    IJournalManager* journal, IJournalWriter* journalWriter,
    EventScheduler* eventScheduler)
: vsaMap(vsaMap),
  stripeMap(stripeMap),
  blockAllocator(blockAllocator),
  wbStripeAllocator(wbStripeAllocator),
  journal(journal),
  journalWriter(journalWriter),
  eventScheduler(eventScheduler)
{
}

MetaUpdater::~MetaUpdater(void)
{
}

int
MetaUpdater::UpdateBlockMap(VolumeIoSmartPtr volumeIo, CallbackSmartPtr callback)
{
    int result = 0;

    CallbackSmartPtr blockMapUpdate(new BlockMapUpdate(volumeIo, vsaMap, blockAllocator, wbStripeAllocator));
    blockMapUpdate->SetCallee(callback);

    if (journal->IsEnabled() == true)
    {
        MpageList dirty = _GetDirtyPages(volumeIo);
        result = journalWriter->AddBlockMapUpdatedLog(volumeIo, dirty, blockMapUpdate);
    }
    else
    {
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

int
MetaUpdater::UpdateStripeMap(Stripe* stripe, StripeAddr oldAddr, EventSmartPtr callback)
{
    // TODO (huijeong.kim) to update
    return 0;
}

} // namespace pos
