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

#include "flush_submission.h"

#include <list>
#include <string>

#include "src/allocator/i_allocator_ctx.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/include/pos_event_id.hpp"
#include "src/include/meta_const.h"
#include "src/include/backend_event.h"
#include "src/io/backend_io/stripe_map_update_request.h"
#include "src/logger/logger.h"
#include "src/array/service/array_service_layer.h"
#include "src/spdk_wrapper/bdev_api.h"
#include "src/include/array_config.h"

namespace pos
{
FlushSubmission::FlushSubmission(Stripe* inputStripe, std::string& arrayName)
: FlushSubmission(inputStripe,
    AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(arrayName),
    IIOSubmitHandler::GetInstance(), arrayName)
{
}

FlushSubmission::FlushSubmission(Stripe* inputStripe, IWBStripeAllocator* wbStripeAllocator, IIOSubmitHandler* ioSubmitHandler, std::string& arrayName)
: Event(false, BackendEvent_Flush),
  stripe(inputStripe),
  iWBStripeAllocator(wbStripeAllocator),
  iIOSubmitHandler(ioSubmitHandler),
  arrayName(arrayName)
{
    SetEventType(BackendEvent_Flush);
}

FlushSubmission::~FlushSubmission(void)
{
}

bool
FlushSubmission::Execute(void)
{
    StripeId logicalStripeId = iWBStripeAllocator->AllocateUserDataStripeId(stripe->GetVsid());

    uint64_t blocksInStripe = 0;
    bufferList.clear();

    if (BdevApi::GetBufferPointer() != nullptr)
    {
        StripeId logicalWbStripeId = stripe->GetWbLsid();
        LogicalBlkAddr startWbLSA = {
            .stripeId = logicalWbStripeId,
            .offset = 0};

        IIOTranslator* translator = ArrayService::Instance()->Getter()->GetTranslator();
        PhysicalBlkAddr physicalWriteEntry;
        translator->Translate(
            "", WRITE_BUFFER, physicalWriteEntry, startWbLSA);
        char *offset = static_cast<char *>(BdevApi::GetBufferPointer()) + (physicalWriteEntry.lba * ArrayConfig::SECTOR_SIZE_BYTE);
        for (auto it = stripe->DataBufferBegin(); it != stripe->DataBufferEnd(); ++it)
        {
            BufferEntry bufferEntry(offset, BLOCKS_IN_CHUNK);
            bufferList.push_back(bufferEntry);
            blocksInStripe += BLOCKS_IN_CHUNK;
            offset += BLOCKS_IN_CHUNK * ArrayConfig::BLOCK_SIZE_BYTE;
        }
    }
    else
    {
        for (auto it = stripe->DataBufferBegin(); it != stripe->DataBufferEnd(); ++it)
        {
            BufferEntry bufferEntry(*it, BLOCKS_IN_CHUNK);
            bufferList.push_back(bufferEntry);
            blocksInStripe += BLOCKS_IN_CHUNK;
        }
    }
    LogicalBlkAddr startLSA = {
        .stripeId = logicalStripeId,
        .offset = 0};

    stripe->SetUserLsid(logicalStripeId);
    CallbackSmartPtr callback(new StripeMapUpdateRequest(stripe, arrayName));

    POS_EVENT_ID eventId = POS_EVENT_ID::FLUSH_DEBUG_SUBMIT;

    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_FLUSH, eventId, PosEventId::GetString(eventId), stripe->GetVsid(), startLSA.stripeId, blocksInStripe);

    IOSubmitHandlerStatus errorReturned = iIOSubmitHandler->SubmitAsyncIO(
        IODirection::WRITE,
        bufferList,
        startLSA, blocksInStripe,
        USER_DATA,
        callback, arrayName);

    return (IOSubmitHandlerStatus::SUCCESS == errorReturned || IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP == errorReturned);
}

uint32_t
FlushSubmission::GetBufferListSize(void)
{
    return bufferList.size();
}

} // namespace pos
