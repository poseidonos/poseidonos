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

#include "src/io/frontend_io/block_map_update_request.h"

#include "mk/ibof_config.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator/i_block_allocator.h"
#include "src/array/array.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/include/pos_event_id.hpp"
#include "src/include/meta_const.h"
#include "src/include/branch_prediction.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/bio/volume_io.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/io_completer.h"
#include "src/event_scheduler/event.h"

namespace pos
{

bool
writeCompletionExecute(VolumeIoSmartPtr volumeIo, CallbackSmartPtr callee)
{
     WriteCompletion event(volumeIo);
     event.SetCallee(callee);
     bool wrapupSuccessful = event.Execute();
     return wrapupSuccessful;
}

BlockMapUpdateRequest::BlockMapUpdateRequest(VolumeIoSmartPtr volumeIo, CallbackSmartPtr originCallback)
: BlockMapUpdateRequest(volumeIo, originCallback, AllocatorServiceSingleton::Instance(),
  std::make_shared<BlockMapUpdate>(volumeIo, originCallback),
  writeCompletionExecute, EventSchedulerSingleton::Instance(),
  new VsaRangeMaker(volumeIo->GetVolumeId(), ChangeSectorToBlock(volumeIo->GetSectorRba()), DivideUp(volumeIo->GetSize(), BLOCK_SIZE), volumeIo->IsGc()))
{
}

BlockMapUpdateRequest::BlockMapUpdateRequest(VolumeIoSmartPtr volumeIo, CallbackSmartPtr originCallback,
  AllocatorService *allocatorService, EventSmartPtr blockMapUpdateEvent,
  WriteCompletionFunc writeCompletionFunc, EventScheduler *eventScheduler, VsaRangeMaker* vsaRangeMaker)
: Callback(EventFrameworkApi::IsReactorNow()),
  volumeIo(volumeIo),
  originCallback(originCallback),
  retryNeeded(false),
  allocatorService(allocatorService),
  blockMapUpdateEvent(blockMapUpdateEvent),
  writeCompletionFunc(writeCompletionFunc),
  eventScheduler(eventScheduler),
  vsaRangeMaker(vsaRangeMaker)
{
}

BlockMapUpdateRequest::~BlockMapUpdateRequest(void)
{
    if (nullptr != vsaRangeMaker)
    {
        delete vsaRangeMaker;
        vsaRangeMaker = nullptr;
    }
}

bool
BlockMapUpdateRequest::_DoSpecificJob(void)
{
    try
    {
        if (unlikely(_GetErrorCount() > 0))
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::WRCMP_IO_ERROR;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                PosEventId::GetString(eventId));
            throw eventId;
        }
        StripeAddr lsidEntry = volumeIo->GetLsidEntry();
        Stripe& stripe = _GetStripe(lsidEntry);
        _UpdateReverseMap(stripe);
        _UpdateMeta();
    }
    catch (...)
    {
        CallbackSmartPtr callee;
        if (originCallback == nullptr)
        {
            callee = volumeIo->GetOriginUbio()->GetCallback();
        }
        else
        {
            callee = originCallback;
        }
        this->SetCallee(callee);
        return true;
    }

    if (retryNeeded)
    {
        return false;
    }
    else
    {
        volumeIo = nullptr;
        return true;
    }
}

void
BlockMapUpdateRequest::_UpdateReverseMap(Stripe& stripe)
{
    VirtualBlkAddr startVsa = volumeIo->GetVsa();
    uint32_t blocks = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    uint64_t startRba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    uint32_t startVsOffset = startVsa.offset;
    uint32_t volumeId = volumeIo->GetVolumeId();

    for (uint32_t blockIndex = 0; blockIndex < blocks; blockIndex++)
    {
        uint64_t vsOffset = startVsOffset + blockIndex;
        BlkAddr targetRba = startRba + blockIndex;
        stripe.UpdateReverseMap(vsOffset, targetRba, volumeId);
    }
}

Stripe&
BlockMapUpdateRequest::_GetStripe(StripeAddr& lsidEntry)
{
    IWBStripeAllocator* iWBStripeAllocator = allocatorService->GetIWBStripeAllocator(volumeIo->GetArrayName());
    Stripe* foundStripe = iWBStripeAllocator->GetStripe(lsidEntry);

    if (unlikely(nullptr == foundStripe))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::WRCMP_INVALID_STRIPE;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
        throw eventId;
    }

    return *foundStripe;
}

void
BlockMapUpdateRequest::_UpdateMeta(void)
{
    VirtualBlkAddr vsa = volumeIo->GetVsa();
    uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    IBlockAllocator* iBlockAllocator = allocatorService->GetIBlockAllocator(volumeIo->GetArrayName());
    VirtualBlks targetVsaRange = {.startVsa = vsa,
        .numBlks = blockCount};

    bool isGc = volumeIo->IsGc();

    retryNeeded = vsaRangeMaker->CheckRetry();
    if (retryNeeded)
    {
        return;
    }
    uint32_t vsaRangeCount = vsaRangeMaker->GetCount();
    bool isOldData = false;

    for (uint32_t vsaRangeIndex = 0; vsaRangeIndex < vsaRangeCount;
         vsaRangeIndex++)
    {
        VirtualBlks& vsaRange = vsaRangeMaker->GetVsaRange(vsaRangeIndex);
        if (isGc)
        {
            if (IsSameVsa(vsaRange.startVsa, volumeIo->GetOldVsa()) == false)
            {
                isOldData = true;
                iBlockAllocator->InvalidateBlks(targetVsaRange);
            }
        }
        else
        {
            iBlockAllocator->InvalidateBlks(vsaRange);
        }
    }
    if (false == isOldData)
    {
        if (likely(blockMapUpdateEvent != nullptr))
        {
            bool mapUpdateSuccessful = blockMapUpdateEvent->Execute();
            if (unlikely(false == mapUpdateSuccessful))
            {
                POS_EVENT_ID eventId = POS_EVENT_ID::WRCMP_MAP_UPDATE_FAILED;
                POS_TRACE_ERROR(static_cast<int>(eventId),
                    PosEventId::GetString(eventId));
                eventScheduler->EnqueueEvent(blockMapUpdateEvent);
            }
        }
        else
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::WRWRAPUP_EVENT_ALLOC_FAILED;
            POS_TRACE_ERROR(static_cast<int>(eventId), PosEventId::GetString(eventId));
            eventScheduler->EnqueueEvent(blockMapUpdateEvent);
        }
    }
    else
    {
        CallbackSmartPtr callee;
        if (originCallback == nullptr)
        {
            callee = volumeIo->GetOriginUbio()->GetCallback();
        }
        else
        {
            callee = originCallback;
        }

        bool wrapupSuccessful = writeCompletionFunc(volumeIo, callee);

        if (unlikely(false == wrapupSuccessful))
        {
            POS_EVENT_ID eventId =
                POS_EVENT_ID::WRCMP_WRITE_WRAPUP_FAILED;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                PosEventId::GetString(eventId));

            CallbackSmartPtr callback(new WriteCompletion(volumeIo));
            callback->SetCallee(callee);
            eventScheduler->EnqueueEvent(callback);
        }
    }
}

} // namespace pos
