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

#include "src/io/frontend_io/block_map_update_completion.h"

#include "src/allocator/i_block_allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/frontend_io/write_completion.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/spdk_wrapper/event_framework_api.h"
namespace pos
{
BlockMapUpdateCompletion::BlockMapUpdateCompletion(
    VolumeIoSmartPtr input, CallbackSmartPtr originCallback,
    bool isReactorNow, IVSAMap* iVSAMap, EventScheduler* eventScheduler, CallbackSmartPtr writeCompletionEvent,
    IBlockAllocator* iBlockAllocator, IWBStripeAllocator* iWBStripeAllocator, VsaRangeMaker* vsaRangeMaker)
: Event(isReactorNow),
  volumeIo(input),
  originCallback(originCallback),
  iVSAMap(iVSAMap),
  eventScheduler(eventScheduler),
  writeCompletionEvent(writeCompletionEvent),
  iBlockAllocator(iBlockAllocator),
  iWBStripeAllocator(iWBStripeAllocator),
  vsaRangeMaker(vsaRangeMaker)
{
}

BlockMapUpdateCompletion::BlockMapUpdateCompletion(VolumeIoSmartPtr inputVolumeIo,
    CallbackSmartPtr originCallback)
: BlockMapUpdateCompletion(
    inputVolumeIo, originCallback, EventFrameworkApiSingleton::Instance()->IsReactorNow(),
    MapperServiceSingleton::Instance()->GetIVSAMap(inputVolumeIo->GetArrayName()),
    EventSchedulerSingleton::Instance(),
    std::make_shared<WriteCompletion>(inputVolumeIo),
    AllocatorServiceSingleton::Instance()->GetIBlockAllocator(inputVolumeIo->GetArrayName()),
    AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(inputVolumeIo->GetArrayName()),
    new VsaRangeMaker(inputVolumeIo->GetVolumeId(), ChangeSectorToBlock(inputVolumeIo->GetSectorRba()),
        DivideUp(inputVolumeIo->GetSize(), BLOCK_SIZE), inputVolumeIo->IsGc(), inputVolumeIo->GetArrayName()))
{
}

BlockMapUpdateCompletion::~BlockMapUpdateCompletion(void)
{
    if (nullptr != vsaRangeMaker)
    {
        delete vsaRangeMaker;
        vsaRangeMaker = nullptr;
    }
}

bool
BlockMapUpdateCompletion::Execute(void)
{
    uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    VirtualBlks targetVsaRange = {.startVsa = volumeIo->GetVsa(), .numBlks = blockCount};
    BlkAddr rba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    iVSAMap->SetVSAs(volumeIo->GetVolumeId(), rba, targetVsaRange);

    StripeAddr lsidEntry = volumeIo->GetLsidEntry();
    Stripe& stripe = _GetStripe(lsidEntry);
    _UpdateReverseMap(stripe);

    uint32_t vsaRangeCount = vsaRangeMaker->GetCount();
    for (uint32_t vsaRangeIndex = 0; vsaRangeIndex < vsaRangeCount;
         vsaRangeIndex++)
    {
        VirtualBlks& vsaRange = vsaRangeMaker->GetVsaRange(vsaRangeIndex);
        iBlockAllocator->InvalidateBlks(vsaRange);
    }

    iBlockAllocator->ValidateBlks(targetVsaRange);

    CallbackSmartPtr callee;

    if (originCallback == nullptr)
    {
        callee = volumeIo->GetOriginUbio()->GetCallback();
    }
    else
    {
        callee = originCallback;
    }

    writeCompletionEvent->SetCallee(callee);
    bool wrapupSuccessful = writeCompletionEvent->Execute();

    if (unlikely(false == wrapupSuccessful))
    {
        writeCompletionEvent->SetCallee(callee);
        eventScheduler->EnqueueEvent(writeCompletionEvent);
    }
    volumeIo = nullptr;

    return true;
}

void
BlockMapUpdateCompletion::_UpdateReverseMap(Stripe& stripe)
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
BlockMapUpdateCompletion::_GetStripe(StripeAddr& lsidEntry)
{
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

} // namespace pos
