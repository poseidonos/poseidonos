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

#include "src/allocator/wbstripe_manager/read_stripe_completion.h"

#include <stdlib.h>

#include "src/include/meta_const.h"

namespace pos
{
ReadStripeCompletion::ReadStripeCompletion(StripeAddr stripeAddr, std::vector<void*> buffer, CallbackSmartPtr callback, int arrayId)
: ReadStripeCompletion(stripeAddr, buffer, callback, arrayId, IIOSubmitHandler::GetInstance())
{
}

ReadStripeCompletion::ReadStripeCompletion(StripeAddr stripeAddr, std::vector<void*> buffer, CallbackSmartPtr callback, int arrayId, IIOSubmitHandler* ioSubmitHandler)
: Callback(false, CallbackType_WriteThroughStripeLoad),
  buffers(buffer),
  doneCallback(callback),
  arrayId(arrayId),
  ioSubmitHandler(ioSubmitHandler)
{
    assert(stripeAddr.stripeLoc == IN_WRITE_BUFFER_AREA);

    writeAddr.stripeId = stripeAddr.stripeId;
    writeAddr.offset = 0;
}

bool
ReadStripeCompletion::_DoSpecificJob(void)
{
    std::list<BufferEntry> bufferList;
    uint32_t blockCount = 0;

    for (auto b : buffers)
    {
        BufferEntry bufferEntry(b, BLOCKS_IN_CHUNK);
        bufferList.push_back(bufferEntry);
        blockCount += BLOCKS_IN_CHUNK;
    }

    IOSubmitHandlerStatus result =
        ioSubmitHandler->SubmitAsyncIO(IODirection::WRITE,
            bufferList, writeAddr, blockCount,
            PartitionType::WRITE_BUFFER, doneCallback, arrayId);

    return (result == IOSubmitHandlerStatus::SUCCESS ||
        result == IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP);
}

} // namespace pos
