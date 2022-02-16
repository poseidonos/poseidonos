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

#include "src/io/general_io/submit_async_read.h"

#include "src/array/service/array_service_layer.h"
#include "src/include/branch_prediction.h"
#include "src/include/memory.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/general_io/io_submit_handler_count.h"
#include "src/logger/logger.h"

#define MIN(x, y) (x) > (y) ? (y) : (x)
namespace pos
{
SubmitAsyncRead::SubmitAsyncRead(CallbackSmartPtr callback)
: SubmitAsyncRead(new MergedIO(callback),
      ArrayService::Instance()->Getter()->GetTranslator())
{
}

SubmitAsyncRead::SubmitAsyncRead(MergedIO* mergedIO, IIOTranslator* translator)
: mergedIO(mergedIO),
  translator(translator)
{
}
SubmitAsyncRead::~SubmitAsyncRead(void)
{
    if (nullptr != mergedIO)
    {
        delete mergedIO;
        mergedIO = nullptr;
    }
}

IOSubmitHandlerStatus
SubmitAsyncRead::Execute(
    std::list<BufferEntry>& bufferList,
    LogicalBlkAddr& startLSA, uint64_t blockCount,
    PartitionType partitionToIO,
    CallbackSmartPtr callback, int arrayId)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;

    if (bufferList.empty())
    {
        IOSubmitHandlerCountSingleton::Instance()->callbackNotCalledCount++;
        IOSubmitHandlerCountSingleton::Instance()->pendingRead--;
        return errorToReturn;
    }

    std::list<BufferEntry>::iterator it = bufferList.begin();
    LogicalBlkAddr currentLSA = startLSA;
    callback->SetWaitingCount(blockCount);

    uint64_t blockCountFromBufferList = 0;
    for (auto& iter : bufferList)
    {
        blockCountFromBufferList += iter.GetBlkCnt();
    }

    if (unlikely(blockCount != blockCountFromBufferList))
    {
        if (blockCount < blockCountFromBufferList)
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::IOSMHDLR_COUNT_DIFFERENT;
            POS_TRACE_WARN(eventId,
                "IOSubmitHandler Async BufferCounts are different :  total of each entries {}  blkcnt :{}",
                blockCountFromBufferList, blockCount);
        }
        else
        {
            IOSubmitHandlerCountSingleton::Instance()->callbackNotCalledCount++;
            IOSubmitHandlerCountSingleton::Instance()->pendingRead--;
            return errorToReturn;
        }
    }

    for (uint64_t blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        BufferEntry& currentBufferEntry = *it;

        uint32_t bufferCount =
            MIN((blockCount - blockIndex), currentBufferEntry.GetBlkCnt());

        for (uint32_t bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
        {
            //PhysicalBlkAddr physicalBlkAddr;
            list<PhysicalEntry> physicalEntries;
            LogicalEntry logicalEntry{
                .addr = currentLSA,
                .blkCnt = 1};

            // Ignore handling the return status.
            translator->Translate(
                arrayId, partitionToIO, physicalEntries, logicalEntry);
            //arrayId, partitionToIO, physicalBlkAddr, currentLSA);

            PhysicalEntry physicalEntry = physicalEntries.front();
            if (mergedIO->IsContiguous(physicalEntry.addr))
            {
                mergedIO->AddContiguousBlock();
            }
            else
            {
                // Ignore handling the return status.
                mergedIO->Process(arrayId);

                void* newBuffer = currentBufferEntry.GetBlock(bufferIndex);
                mergedIO->SetNewStart(newBuffer, physicalEntry.addr);
            }

            currentLSA.offset++;
        }
        // Net status of the upper operations is gathered from here.
        errorToReturn = mergedIO->Process(arrayId);
        mergedIO->Reset();

        blockIndex += (bufferCount - 1);
        it++;
    }

    if (errorToReturn != IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
    {
        errorToReturn = IOSubmitHandlerStatus::SUCCESS;
    }
    return errorToReturn;
}

} // namespace pos
