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

#include "src/io/general_io/io_submit_handler.h"

#include <list>
#include <string>

#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/general_io/submit_async_read.h"
#include "src/io/general_io/submit_async_write.h"
#include "src/io/general_io/sync_io_completion.h"
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
    PartitionType partitionToIO, std::string arrayName)
{
    std::atomic<bool> needToWait(true);
    uint32_t errorCount = 0;

    CallbackSmartPtr callback(new SyncIoCompletion(needToWait, errorCount));

    IOSubmitHandlerStatus errorToReturn = SubmitAsyncIO(direction, bufferList,
        startLSA, blockCount, partitionToIO, callback, arrayName);

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
    std::string arrayName)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;
    do
    {
        if (IODirection::READ == direction)
        {
            SubmitAsyncRead asyncRead(callback);
            errorToReturn = asyncRead.Execute(bufferList, startLSA, blockCount, partitionToIO, callback, arrayName);
        }
        else if (IODirection::WRITE == direction)
        {
            SubmitAsyncWrite asyncWrite;
            bool needTrim = false;
            errorToReturn = asyncWrite.Execute(bufferList, startLSA, blockCount,
                partitionToIO, callback, arrayName, needTrim);
        }
        else if (IODirection::TRIM == direction)
        {
            SubmitAsyncWrite asyncWrite;
            bool needTrim = true;
            errorToReturn = asyncWrite.Execute(bufferList, startLSA, blockCount,
                partitionToIO, callback, arrayName, needTrim);
        }
        else
        {
            break;
        }
    } while (false);

    return errorToReturn;
}
} // namespace pos
