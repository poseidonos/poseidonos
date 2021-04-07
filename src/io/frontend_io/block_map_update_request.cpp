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
#include "src/allocator/allocator.h"
#include "src/allocator/stripe.h"
#include "src/array/array.h"
#include "src/device/event_framework_api.h"
#include "src/include/ibof_event_id.hpp"
#include "src/include/meta_const.h"
#include "src/io/frontend_io/block_map_update.h"
#include "src/io/frontend_io/write_completion.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/io/general_io/volume_io.h"
#include "src/io/general_io/vsa_range_maker.h"
#include "src/logger/logger.h"
#include "src/scheduler/event.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/event_scheduler.h"

namespace ibofos
{
BlockMapUpdateRequest::BlockMapUpdateRequest(VolumeIoSmartPtr volumeIo, CallbackSmartPtr originCallback)
: Callback(EventFrameworkApi::IsReactorNow()),
  volumeIo(volumeIo),
  originCallback(originCallback),
  retryNeeded(false)
{
}

BlockMapUpdateRequest::~BlockMapUpdateRequest(void)
{
}

bool
BlockMapUpdateRequest::_DoSpecificJob(void)
{
    try
    {
        if (unlikely(_GetErrorCount() > 0))
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::WRCMP_IO_ERROR;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));
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
    uint64_t startRba = ChangeSectorToBlock(volumeIo->GetRba());
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
    Allocator& allocator = *AllocatorSingleton::Instance();
    Stripe* foundStripe = allocator.GetStripe(lsidEntry);

    if (unlikely(nullptr == foundStripe))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::WRCMP_INVALID_STRIPE;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        throw eventId;
    }

    return *foundStripe;
}

void
BlockMapUpdateRequest::_UpdateMeta(void)
{
    VirtualBlkAddr vsa = volumeIo->GetVsa();
    uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    BlkAddr startRba = ChangeSectorToBlock(volumeIo->GetRba());
    uint32_t volumeId = volumeIo->GetVolumeId();
    Allocator& allocator = *AllocatorSingleton::Instance();
    VirtualBlks targetVsaRange = {.startVsa = vsa,
        .numBlks = blockCount};

    bool isGc = volumeIo->IsGc();

    VsaRangeMaker vsaRangeMaker(volumeId, startRba, blockCount, isGc);
    retryNeeded = vsaRangeMaker.CheckRetry();
    if (retryNeeded)
    {
        return;
    }
    uint32_t vsaRangeCount = vsaRangeMaker.GetCount();
    bool isOldData = false;

    for (uint32_t vsaRangeIndex = 0; vsaRangeIndex < vsaRangeCount;
         vsaRangeIndex++)
    {
        VirtualBlks& vsaRange = vsaRangeMaker.GetVsaRange(vsaRangeIndex);
        if (true == volumeIo->IsGc())
        {
            if (IsSameVsa(vsaRange.startVsa, volumeIo->GetOldVsa()))
            {
                allocator.InvalidateBlks(vsaRange);
            }
            else
            {
                isOldData = true;
                allocator.InvalidateBlks(targetVsaRange);
            }
        }
        else
        {
            allocator.InvalidateBlks(vsaRange);
        }
    }

    if (false == isOldData)
    {
        EventSmartPtr event(new BlockMapUpdate(volumeIo, originCallback));
        if (likely(event != nullptr))
        {
            bool mapUpdateSuccessful = event->Execute();

            if (unlikely(false == mapUpdateSuccessful))
            {
                IBOF_EVENT_ID eventId = IBOF_EVENT_ID::WRCMP_MAP_UPDATE_FAILED;
                IBOF_TRACE_ERROR(static_cast<int>(eventId),
                    IbofEventId::GetString(eventId));
                EventArgument::GetEventScheduler()->EnqueueEvent(event);
            }
        }
        else
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::WRWRAPUP_EVENT_ALLOC_FAILED;
            IBOF_TRACE_ERROR(static_cast<int>(eventId), IbofEventId::GetString(eventId));
            EventArgument::GetEventScheduler()->EnqueueEvent(event);
        }
    }
    else
    {
        WriteCompletion event(volumeIo);

        CallbackSmartPtr callee;
        if (originCallback == nullptr)
        {
            callee = volumeIo->GetOriginUbio()->GetCallback();
        }
        else
        {
            callee = originCallback;
        }

        event.SetCallee(callee);
        bool wrapupSuccessful = event.Execute();

        if (unlikely(false == wrapupSuccessful))
        {
            IBOF_EVENT_ID eventId =
                IBOF_EVENT_ID::WRCMP_WRITE_WRAPUP_FAILED;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));

            CallbackSmartPtr callback(new WriteCompletion(volumeIo));
            callback->SetCallee(callee);
            EventArgument::GetEventScheduler()->EnqueueEvent(callback);
        }
    }
}

} // namespace ibofos
