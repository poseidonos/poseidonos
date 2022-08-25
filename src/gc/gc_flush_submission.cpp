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

#include "gc_flush_submission.h"

#include <list>
#include <memory>
#include <string>

#include "src/allocator/allocator.h"
#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator/stripe/stripe.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array/service/array_service_layer.h"
#include "src/array_mgmt/array_manager.h"
#include "src/bio/ubio.h"
#include "src/gc/flow_control/flow_control.h"
#include "src/gc/flow_control/flow_control_service.h"
#include "src/gc/gc_flush_completion.h"
#include "src/include/address_type.h"
#include "src/include/backend_event.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/general_io/rba_state_manager.h"
#include "src/io/general_io/rba_state_service.h"
#include "src/io/general_io/translator.h"
#include "src/logger/logger.h"
#include "src/volume/volume_service.h"

namespace pos
{
GcFlushSubmission::GcFlushSubmission(std::string arrayName, std::vector<BlkInfo>* blkInfoList, uint32_t volumeId,
    GcWriteBuffer* dataBuffer, GcStripeManager* gcStripeManager)
: GcFlushSubmission(arrayName, blkInfoList, volumeId, dataBuffer, gcStripeManager, nullptr,
      AllocatorServiceSingleton::Instance()->GetIBlockAllocator(arrayName),
      IIOSubmitHandler::GetInstance(),
      FlowControlServiceSingleton::Instance()->GetFlowControl(arrayName),
      ArrayMgr()->GetInfo(arrayName)->arrayInfo)
{
}

GcFlushSubmission::GcFlushSubmission(std::string arrayName, std::vector<BlkInfo>* blkInfoList, uint32_t volumeId,
    GcWriteBuffer* dataBuffer, GcStripeManager* gcStripeManager,
    CallbackSmartPtr inputCallback, IBlockAllocator* inputIBlockAllocator,
    IIOSubmitHandler* inputIIOSubmitHandler,
    FlowControl* inputFlowControl, IArrayInfo* inputIArrayInfo)
: Event(false, BackendEvent_Flush),
  arrayName(arrayName),
  blkInfoList(blkInfoList),
  volumeId(volumeId),
  dataBuffer(dataBuffer),
  gcStripeManager(gcStripeManager),
  inputCallback(inputCallback),
  iBlockAllocator(inputIBlockAllocator),
  iIOSubmitHandler(inputIIOSubmitHandler),
  flowControl(inputFlowControl),
  iArrayInfo(inputIArrayInfo)
{
    SetEventType(BackendEvent_Flush);
}

GcFlushSubmission::~GcFlushSubmission(void)
{
}

bool
GcFlushSubmission::Execute(void)
{
    const PartitionLogicalSize* udSize =
        iArrayInfo->GetSizeInfo(PartitionType::USER_DATA);
    uint32_t totalBlksPerUserStripe = udSize->blksPerStripe;

    int token = flowControl->GetToken(FlowControlType::GC, totalBlksPerUserStripe);
    if (0 >= token)
    {
        return false;
    }

    Stripe* allocatedStripe = _AllocateStripe(volumeId);
    if (allocatedStripe == nullptr)
    {
        if (0 < token)
        {
            flowControl->ReturnToken(FlowControlType::GC, token);
        }
        return false;
    }

    StripeSmartPtr stripe(allocatedStripe);

    for (uint32_t offset = 0; offset < blkInfoList->size(); offset++)
    {
        std::vector<BlkInfo>::iterator it = blkInfoList->begin();
        std::advance(it, offset);
        BlkInfo blkInfo = *it;
        stripe->UpdateReverseMapEntry(offset, blkInfo.rba, volumeId);
        stripe->UpdateVictimVsa(offset, blkInfo.vsa);
    }

    blkInfoList->clear();
    delete blkInfoList;

    std::list<BufferEntry> bufferList;

    uint64_t blocksInStripe = 0;

    for (auto it = dataBuffer->begin(); it != dataBuffer->end(); ++it)
    {
        BufferEntry bufferEntry(*it, BLOCKS_IN_CHUNK);
        bufferList.push_back(bufferEntry);
        blocksInStripe += BLOCKS_IN_CHUNK;
    }

    CallbackSmartPtr callback;
    if (nullptr == inputCallback)
    {
        callback = std::make_shared<GcFlushCompletion>(stripe, arrayName, gcStripeManager, dataBuffer);
    }
    else
    {
        callback = inputCallback;
    }

    StripeId logicalStripeId = stripe->GetUserLsid();
    LogicalBlkAddr startLSA = {
        .stripeId = logicalStripeId,
        .offset = 0};

    POS_EVENT_ID eventId = EID(FLUSH_DEBUG_SUBMIT);

    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_FLUSH, eventId,
        "Flush Submission vsid  {}: StartLSA.stripeId : {} blocksInStripe : {}",
        stripe->GetVsid(), startLSA.stripeId, blocksInStripe);

    IOSubmitHandlerStatus errorReturned = iIOSubmitHandler->SubmitAsyncIO(
        IODirection::WRITE,
        bufferList,
        startLSA, blocksInStripe,
        USER_DATA,
        callback,
        iArrayInfo->GetIndex(),
        false);

    POS_TRACE_DEBUG(EID(GC_STRIPE_FLUSH_SUBMIT),
        "gc flush submission, arrayName:{}, stripeUserLsid:{}, result:{}",
        arrayName,
        logicalStripeId,
        (IOSubmitHandlerStatus::SUCCESS == errorReturned || IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP == errorReturned));

    return (IOSubmitHandlerStatus::SUCCESS == errorReturned || IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP == errorReturned);
}

Stripe*
GcFlushSubmission::_AllocateStripe(uint32_t volumeId)
{
    Stripe* stripe = iBlockAllocator->AllocateGcDestStripe(volumeId);
    return stripe;
}

} // namespace pos
