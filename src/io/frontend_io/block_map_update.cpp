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

#include "src/io/frontend_io/block_map_update.h"

#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/memory.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/frontend_io/block_map_update_completion.h"
#include "src/journal_service/journal_service.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
BlockMapUpdate::BlockMapUpdate(VolumeIoSmartPtr inputVolumeIo, CallbackSmartPtr originCallback,
    bool isReactorNow, IVSAMap* iVSAMap, JournalService* journalService, EventScheduler* eventScheduler,
    EventSmartPtr blockMapUpdateCompletionEvent)
: Event(isReactorNow),
  volumeIo(inputVolumeIo),
  iVSAMap(iVSAMap),
  originCallback(originCallback),
  journalService(journalService),
  eventScheduler(eventScheduler),
  blockMapUpdateCompletionEvent(blockMapUpdateCompletionEvent)
{
}

BlockMapUpdate::BlockMapUpdate(VolumeIoSmartPtr inputVolumeIo, CallbackSmartPtr originCallback)
: BlockMapUpdate(
    inputVolumeIo, originCallback, EventFrameworkApiSingleton::Instance()->IsReactorNow(),
    MapperServiceSingleton::Instance()->GetIVSAMap(inputVolumeIo->GetArrayId()),
    JournalServiceSingleton::Instance(),
    EventSchedulerSingleton::Instance(),
    std::make_shared<BlockMapUpdateCompletion>(inputVolumeIo, originCallback))
{
}

bool
BlockMapUpdate::Execute(void)
{
    bool executionSuccessful = false;

    if (journalService->IsEnabled(volumeIo->GetArrayId()))
    {
        IJournalWriter* journal = journalService->GetWriter(volumeIo->GetArrayId());

        MpageList dirty = _GetDirtyPages(volumeIo);
        int result = journal->AddBlockMapUpdatedLog(volumeIo, dirty, blockMapUpdateCompletionEvent);

        executionSuccessful = (result == 0);
    }
    else
    {
        executionSuccessful = blockMapUpdateCompletionEvent->Execute();
        if (unlikely(false == executionSuccessful))
        {
            POS_EVENT_ID eventId =
                POS_EVENT_ID::WRCMP_MAP_UPDATE_FAILED;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                PosEventId::GetString(eventId));
            eventScheduler->EnqueueEvent(blockMapUpdateCompletionEvent);
            executionSuccessful = true;
        }
    }

    if (executionSuccessful == true)
    {
        volumeIo = nullptr;
    }
    return executionSuccessful;
}

MpageList
BlockMapUpdate::_GetDirtyPages(VolumeIoSmartPtr volumeIo)
{
    uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    BlkAddr rba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    MpageList dirty = iVSAMap->GetDirtyVsaMapPages(volumeIo->GetVolumeId(), rba, blockCount);
    return dirty;
}

} // namespace pos
