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

#include "src/array/array.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/general_io/array_unlocking.h"
#include "src/io/general_io/internal_write_completion.h"
#include "src/io/general_io/io_submit_handler_status.h"
#include "src/io/general_io/merged_io.h"
#include "src/io/general_io/sync_io_completion.h"
#include "src/io/general_io/ubio.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/io_dispatcher.h"

namespace ibofos
{
IOSubmitHandlerStatus
IOSubmitHandler::SyncIO(
    IODirection direction,
    std::list<BufferEntry>& bufferList,
    LogicalBlkAddr& startLSA, uint64_t blockCount,
    PartitionType partitionToIO)
{
    std::atomic<bool> needToWait(true);
    uint32_t errorCount = 0;

    CallbackSmartPtr callback(new SyncIoCompletion(needToWait, errorCount));

    IOSubmitHandlerStatus errorToReturn = SubmitAsyncIO(direction, bufferList,
        startLSA, blockCount, partitionToIO, callback);

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
        return IOSubmitHandlerStatus::FAIL;
    }

    return errorToReturn;
}

IOSubmitHandlerStatus
IOSubmitHandler::SubmitAsyncIO(
    IODirection direction,
    std::list<BufferEntry>& bufferList,
    LogicalBlkAddr& startLSA, uint64_t blockCount,
    PartitionType partitionToIO,
    CallbackSmartPtr callback)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;
    do
    {
        if (IODirection::READ == direction)
        {
            errorToReturn = _AsyncRead(bufferList, startLSA, blockCount,
                partitionToIO, callback);
        }
        else if (IODirection::WRITE == direction)
        {
            errorToReturn = _AsyncWrite(bufferList, startLSA, blockCount,
                partitionToIO, callback);
        }
        else if (IODirection::TRIM == direction)
        {
            errorToReturn = _TrimData(bufferList, startLSA, blockCount,
                partitionToIO, callback);
        }
        else
        {
            break;
        }
    } while (false);

    return errorToReturn;
}

IOSubmitHandlerStatus
IOSubmitHandler::_CheckAsyncWriteError(IBOF_EVENT_ID eventId)

{
    StateManager* stateMgr = StateManagerSingleton::Instance();
    if (stateMgr->GetState() == State::STOP)
    {
        IBOF_TRACE_ERROR(eventId, IbofEventId::GetString(eventId));
        return IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP;
    }

    return IOSubmitHandlerStatus::SUCCESS;
}

IOSubmitHandlerStatus
IOSubmitHandler::_AsyncWrite(
    std::list<BufferEntry>& bufferList,
    LogicalBlkAddr& startLSA, uint64_t blockCount,
    PartitionType partitionToIO,
    CallbackSmartPtr callback)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;
    Array* sysArray = ArraySingleton::Instance();

    if (bufferList.empty())
    {
        return errorToReturn;
    }

    LogicalWriteEntry logicalWriteEntry = {
        .addr = startLSA,
        .blkCnt = static_cast<uint32_t>(blockCount),
        .buffers = &bufferList};

    StripeId stripeId = startLSA.stripeId;
    if (sysArray->TryLock(partitionToIO, stripeId) == false)
    {
        return IOSubmitHandlerStatus::TRYLOCK_FAIL;
    }

    std::list<PhysicalWriteEntry> physicalWriteEntries;
    int ret = sysArray->Convert(
        partitionToIO, physicalWriteEntries, logicalWriteEntry);

    if (ret != 0)
    {
        callback->InformError(CallbackError::GENERIC_ERROR);
        sysArray->Unlock(partitionToIO, stripeId);
        callback->Execute();
        return IOSubmitHandlerStatus::SUCCESS;
    }

    uint32_t totalIoCount = 0;

    for (PhysicalWriteEntry& physicalWriteEntry : physicalWriteEntries)
    {
        totalIoCount += physicalWriteEntry.buffers.size();
    }

    callback->SetWaitingCount(1);
    CallbackSmartPtr arrayUnlocking(
        new ArrayUnlocking(partitionToIO, stripeId));
    arrayUnlocking->SetCallee(callback);
    arrayUnlocking->SetWaitingCount(totalIoCount);
#if defined QOS_ENABLED_BE
    arrayUnlocking->SetEventType(callback->GetEventType());
#endif
    IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();

    for (PhysicalWriteEntry& physicalWriteEntry : physicalWriteEntries)
    {
        for (BufferEntry& buffer : physicalWriteEntry.buffers)
        {
            UbioSmartPtr ubio(new Ubio(buffer.GetBufferEntry(),
                buffer.GetBlkCnt() * Ubio::UNITS_PER_BLOCK));
            ubio->dir = UbioDir::Write;
            ubio->SetPba(physicalWriteEntry.addr);
            CallbackSmartPtr event(new InternalWriteCompletion(buffer));
#if defined QOS_ENABLED_BE
            ubio->SetEventType(callback->GetEventType());
            event->SetEventType(ubio->GetEventType());
#endif
            event->SetCallee(arrayUnlocking);
            ubio->SetCallback(event);

            if (ioDispatcher->Submit(ubio) < 0)
            {
                errorToReturn =
                    _CheckAsyncWriteError(IBOF_EVENT_ID::REF_COUNT_RAISE_FAIL);
                continue;
            }
        }
    }

    if (errorToReturn != IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
    {
        errorToReturn = IOSubmitHandlerStatus::SUCCESS;
    }

    return errorToReturn;
}

IOSubmitHandlerStatus
IOSubmitHandler::_AsyncRead(
    std::list<BufferEntry>& bufferList,
    LogicalBlkAddr& startLSA, uint64_t blockCount,
    PartitionType partitionToIO,
    CallbackSmartPtr callback)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;
    Array* arrayManager = ArraySingleton::Instance();

    if (bufferList.empty())
    {
        return errorToReturn;
    }
    std::list<BufferEntry>::iterator it = bufferList.begin();
    LogicalBlkAddr currentLSA = startLSA;
    callback->SetWaitingCount(blockCount);

    MergedIO mergedIO(callback);

    uint64_t blockCountFromBufferList = 0;
    for (auto& iter : bufferList)
    {
        blockCountFromBufferList += iter.GetBlkCnt();
    }

    if (unlikely(blockCount != blockCountFromBufferList))
    {
        if (blockCount < blockCountFromBufferList)
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::IOSMHDLR_COUNT_DIFFERENT;
            IBOF_TRACE_WARN(eventId, IbofEventId::GetString(eventId),
                blockCountFromBufferList, blockCount);
        }
        else
        {
            return errorToReturn;
        }
    }

    for (uint64_t blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        BufferEntry& currentBufferEntry = *it;

        uint32_t bufferCount =
            Min((blockCount - blockIndex), currentBufferEntry.GetBlkCnt());

        for (uint32_t bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
        {
            PhysicalBlkAddr physicalBlkAddr;

            // Ignore handling the return status.
            arrayManager->Translate(partitionToIO, physicalBlkAddr, currentLSA);

            if (mergedIO.IsContiguous(physicalBlkAddr))
            {
                mergedIO.AddContiguousBlock();
            }
            else
            {
                // Ignore handling the return status.
                mergedIO.Process();

                void* newBuffer = currentBufferEntry.GetBlock(bufferIndex);
                mergedIO.SetNewStart(newBuffer, physicalBlkAddr);
            }

            currentLSA.offset++;
        }

        // Net status of the upper operations is gathered from here.
        errorToReturn = mergedIO.Process();
        mergedIO.Reset();

        blockIndex += (bufferCount - 1);
        it++;
    }

    if (errorToReturn != IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
    {
        errorToReturn = IOSubmitHandlerStatus::SUCCESS;
    }
    return errorToReturn;
}

IOSubmitHandlerStatus
IOSubmitHandler::_TrimData(
    std::list<BufferEntry>& bufferList,
    LogicalBlkAddr& startLSA, uint64_t blockCount,
    PartitionType partitionToIO,
    CallbackSmartPtr callback)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;
    Array* sysArray = ArraySingleton::Instance();

    LogicalWriteEntry logicalWriteEntry = {
        .addr = startLSA,
        .blkCnt = static_cast<uint32_t>(blockCount),
        .buffers = &bufferList};

    StripeId stripeId = startLSA.stripeId;
    if (sysArray->TryLock(partitionToIO, stripeId) == false)
    {
        return IOSubmitHandlerStatus::TRYLOCK_FAIL;
    }

    std::list<PhysicalWriteEntry> physicalWriteEntries;
    int ret = sysArray->Convert(
        partitionToIO, physicalWriteEntries, logicalWriteEntry);

    if (ret != 0)
    {
        callback->InformError(CallbackError::GENERIC_ERROR);
        sysArray->Unlock(partitionToIO, stripeId);
        callback->Execute();
        return IOSubmitHandlerStatus::SUCCESS;
    }

    uint32_t totalIoCount = 0;

    for (PhysicalWriteEntry& physicalWriteEntry : physicalWriteEntries)
    {
        totalIoCount += physicalWriteEntry.buffers.size();
    }

    callback->SetWaitingCount(1);
    CallbackSmartPtr arrayUnlocking(
        new ArrayUnlocking(partitionToIO, stripeId));
    arrayUnlocking->SetCallee(callback);
    arrayUnlocking->SetWaitingCount(totalIoCount);

    IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();

    for (PhysicalWriteEntry& physicalWriteEntry : physicalWriteEntries)
    {
        for (BufferEntry& buffer : physicalWriteEntry.buffers)
        {
            UbioSmartPtr ubio(new Ubio(buffer.GetBufferEntry(),
                buffer.GetBlkCnt() * Ubio::UNITS_PER_BLOCK));

            ubio->dir = UbioDir::Deallocate;
            ubio->SetPba(physicalWriteEntry.addr);
            CallbackSmartPtr event(new InternalWriteCompletion(buffer));
            event->SetCallee(arrayUnlocking);
            ubio->SetCallback(event);

            if (ioDispatcher->Submit(ubio) < 0 ||
                ubio->GetError() != CallbackError::SUCCESS)
            {
                errorToReturn = _CheckAsyncWriteError(IBOF_EVENT_ID::REF_COUNT_RAISE_FAIL);

                continue;
            }
        }

    }

    if (errorToReturn != IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
    {
        errorToReturn = IOSubmitHandlerStatus::SUCCESS;
    }

    return errorToReturn;
}
} // namespace ibofos
