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

#include "src/io/general_io/io_submit_handler.h"

#include <list>
#include <string>


#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.h"
#include "src/io/general_io/submit_async_read.h"
#include "src/io/general_io/submit_async_write.h"
#include "src/io/general_io/submit_async_byte_io.h"
#include "src/io/general_io/sync_io_completion.h"
#include "src/io/general_io/io_submit_handler_count.h"
#include "src/logger/logger.h"

namespace pos
{
IOSubmitHandler::IOSubmitHandler(void)
{
}

IOSubmitHandler::~IOSubmitHandler(void)
{
}

IOSubmitHandlerStatus
IOSubmitHandler::SyncIO(
    IODirection direction,
    std::list<BufferEntry>& bufferList,
    LogicalBlkAddr& startLSA, uint64_t blockCount,
    PartitionType partitionToIO, int arrayId)
{
    std::atomic<bool> needToWait(true);
    uint32_t errorCount = 0;

    CallbackSmartPtr callback(new SyncIoCompletion(needToWait, errorCount));

    IOSubmitHandlerStatus errorToReturn = SubmitAsyncIO(direction, bufferList,
        startLSA, blockCount, partitionToIO, callback, arrayId);

    if (IOSubmitHandlerStatus::SUCCESS == errorToReturn ||
        IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP == errorToReturn)
    {
        while (needToWait)
        {
            usleep(1);
        }
    }

    if (errorCount > 0 && errorToReturn == IOSubmitHandlerStatus::SUCCESS)
    {
        return IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP;
    }

    return errorToReturn;
}

IOSubmitHandlerStatus
IOSubmitHandler::SubmitAsyncIO(
    IODirection direction,
    std::list<BufferEntry>& bufferList,
    LogicalBlkAddr& startLSA, uint64_t blockCount,
    PartitionType partitionToIO,
    CallbackSmartPtr callback,
    int arrayId,
    bool parityOnly)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;
    do
    {
        if (IODirection::READ == direction)
        {
            SubmitAsyncRead asyncRead(callback);
            IOSubmitHandlerCountSingleton::Instance()->pendingRead++;
            errorToReturn = asyncRead.Execute(bufferList, startLSA, blockCount, partitionToIO, callback, arrayId);
        }
        else if (IODirection::WRITE == direction)
        {
            SubmitAsyncWrite asyncWrite;
            bool needTrim = false;
            IOSubmitHandlerCountSingleton::Instance()->pendingWrite++;
            errorToReturn = asyncWrite.Execute(bufferList, startLSA, blockCount,
                partitionToIO, callback, arrayId, needTrim, parityOnly);
        }
        else if (IODirection::TRIM == direction)
        {
            SubmitAsyncWrite asyncWrite;
            bool needTrim = true;
            IOSubmitHandlerCountSingleton::Instance()->pendingWrite++;
            errorToReturn = asyncWrite.Execute(bufferList, startLSA, blockCount,
                partitionToIO, callback, arrayId, needTrim);
        }
        else
        {
            IOSubmitHandlerCountSingleton::Instance()->callbackNotCalledCount++;
            break;
        }
    } while (false);
    return errorToReturn;
}

IOSubmitHandlerStatus
IOSubmitHandler::SubmitAsyncByteIO(
    IODirection direction,
    void* buffer,
    LogicalByteAddr& startLSA,
    PartitionType partitionToIO,
    CallbackSmartPtr callback,
    int arrayId)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;

    AsyncByteIO asyncByteIO;
    IOSubmitHandlerCountSingleton::Instance()->pendingByteIo++;
    errorToReturn = asyncByteIO.Execute(direction,
        buffer, startLSA, partitionToIO, callback, arrayId);
    return errorToReturn;
}
} // namespace pos
