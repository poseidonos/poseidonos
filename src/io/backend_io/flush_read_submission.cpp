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

#include "src/io/backend_io/flush_read_submission.h"

#include <unistd.h>

#include "src/allocator/wb_stripe_manager/stripe.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/include/meta_const.h"
#include "src/include/backend_event.h"
#include "src/io/backend_io/flush_read_completion.h"
#include "src/io/backend_io/flush_submission.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/bdev_api.h"
#include "src/event_scheduler/event_scheduler.h"


namespace pos
{
FlushReadSubmission::FlushReadSubmission(Stripe* stripe, std::string arrayName)
: FlushReadSubmission(stripe, IIOSubmitHandler::GetInstance(), arrayName)
{
}

FlushReadSubmission::FlushReadSubmission(Stripe* stripe, IIOSubmitHandler* handler, std::string arrayName)
: Event(false, BackendEvent_Flush),
  stripe(stripe),
  ioSubmitHandler(handler),
  arrayName(arrayName)
{
    SetEventType(BackendEvent_Flush);
}

FlushReadSubmission::~FlushReadSubmission(void)
{
}

bool
FlushReadSubmission::Execute(void)
{
    StripeId logicalStripeId = stripe->GetWbLsid();

    std::list<BufferEntry> bufferList;
    uint64_t blocksInStripe = 0;

    for (auto it = stripe->DataBufferBegin(); it != stripe->DataBufferEnd(); ++it)
    {
        BufferEntry bufferEntry(*it, BLOCKS_IN_CHUNK);
        bufferList.push_back(bufferEntry);
        blocksInStripe += BLOCKS_IN_CHUNK;
    }
    CallbackSmartPtr callback(new FlushReadCompletion(stripe, arrayName));
    if (unlikely(nullptr == callback))
    {
        POS_EVENT_ID eventId =
            POS_EVENT_ID::FLUSHREAD_FAIL_TO_ALLOCATE_MEMORY;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
        return false;
    }

    if (BdevApi::GetBufferPointer() != nullptr)
    {
        bool executed = callback->Execute();
        if (executed == false)
        {
            EventSchedulerSingleton::Instance()->EnqueueEvent(callback);
        }
        return true;
    }

    LogicalBlkAddr startLSA = {
        .stripeId = logicalStripeId,
        .offset = 0};

    POS_EVENT_ID eventId = POS_EVENT_ID::FLUSHREAD_DEBUG_SUBMIT;
    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_FLUSH, eventId, PosEventId::GetString(eventId), startLSA.stripeId, blocksInStripe);

    IOSubmitHandlerStatus errorReturned = ioSubmitHandler->SubmitAsyncIO(IODirection::READ,
        bufferList, startLSA, blocksInStripe, WRITE_BUFFER, callback, arrayName);

    bool done = (errorReturned == IOSubmitHandlerStatus::SUCCESS);

    return done;
}

} // namespace pos

