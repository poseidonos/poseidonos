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

#include "src/array/service/array_service_layer.h"
#include "src/include/pos_event_id.hpp"
#include "src/include/branch_prediction.h"
#include "src/io/general_io/array_unlocking.h"
#include "src/io/general_io/internal_write_completion.h"
#include "src/io_submit_interface/io_submit_handler_status.h"
#include "src/io/general_io/merged_io.h"
#include "src/io/general_io/sync_io_completion.h"
#include "src/bio/ubio.h"
#include "src/state/state_manager.h"
#include "src/io_scheduler/io_dispatcher.h"
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
    CallbackSmartPtr callback,
    std::string arrayName)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;
    do
    {
        if (IODirection::READ == direction)
        {
            errorToReturn = _AsyncRead(bufferList, startLSA, blockCount,
                partitionToIO, callback, arrayName);
        }
        else if (IODirection::WRITE == direction)
        {
            errorToReturn = _AsyncWrite(bufferList, startLSA, blockCount,
                partitionToIO, callback, arrayName);
        }
        else if (IODirection::TRIM == direction)
        {
            errorToReturn = _TrimData(bufferList, startLSA, blockCount,
                partitionToIO, callback, arrayName);
        }
        else
        {
            break;
        }
    } while (false);

    return errorToReturn;
}

IOSubmitHandlerStatus
IOSubmitHandler::_CheckAsyncWriteError(POS_EVENT_ID eventId, const std::string& arrayName)
{
    IStateControl* stateControl = StateManagerSingleton::Instance()->GetStateControl(arrayName);
    if (stateControl->GetState()->ToStateType() == StateEnum::STOP)
    {
        POS_TRACE_ERROR(eventId, PosEventId::GetString(eventId));
        return IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP;
    }

    return IOSubmitHandlerStatus::SUCCESS;
}

IOSubmitHandlerStatus
IOSubmitHandler::_AsyncWrite(
    std::list<BufferEntry>& bufferList,
    LogicalBlkAddr& startLSA, uint64_t blockCount,
    PartitionType partitionToIO,
    CallbackSmartPtr callback, std::string& arrayName)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;

    if (bufferList.empty())
    {
        return errorToReturn;
    }

    LogicalWriteEntry logicalWriteEntry = {
        .addr = startLSA,
        .blkCnt = static_cast<uint32_t>(blockCount),
        .buffers = &bufferList};

    StripeId stripeId = startLSA.stripeId;
    if (partitionToIO == PartitionType::META_SSD)
    {
        IIOLocker* locker = ArrayService::Instance()->Getter()->GetLocker();
        if (locker->TryLock(arrayName, stripeId) == false)
        {
            return IOSubmitHandlerStatus::TRYLOCK_FAIL;
        }
    }

    std::list<PhysicalWriteEntry> physicalWriteEntries;
    IIOTranslator* translator = ArrayService::Instance()->Getter()->GetTranslator();
    int ret = translator->Convert(
            arrayName, partitionToIO, physicalWriteEntries, logicalWriteEntry);

    if (ret != 0)
    {
        callback->InformError(IOErrorType::GENERIC_ERROR);
        if (partitionToIO == PartitionType::META_SSD)
        {
            IIOLocker* locker = ArrayService::Instance()->Getter()->GetLocker();
            locker->Unlock(arrayName, stripeId);
        }
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
        new ArrayUnlocking(partitionToIO, stripeId, arrayName));
    arrayUnlocking->SetCallee(callback);
    arrayUnlocking->SetWaitingCount(totalIoCount);
#if defined QOS_ENABLED_BE
    arrayUnlocking->SetEventType(callback->GetEventType());
#endif
    IODispatcher* ioDispatcher = IODispatcherSingleton::Instance();

    for (PhysicalWriteEntry& physicalWriteEntry : physicalWriteEntries)
    {
        for (BufferEntry& buffer : physicalWriteEntry.buffers)
        {
            UbioSmartPtr ubio(new Ubio(buffer.GetBufferPtr(),
                buffer.GetBlkCnt() * Ubio::UNITS_PER_BLOCK, arrayName));
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
                    _CheckAsyncWriteError(POS_EVENT_ID::REF_COUNT_RAISE_FAIL, arrayName);
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
    CallbackSmartPtr callback, std::string& arrayName)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;

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
            POS_EVENT_ID eventId = POS_EVENT_ID::IOSMHDLR_COUNT_DIFFERENT;
            POS_TRACE_WARN(eventId, PosEventId::GetString(eventId),
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
            IIOTranslator* translator = ArrayService::Instance()->Getter()->GetTranslator();
            translator->Translate(
            arrayName, partitionToIO, physicalBlkAddr, currentLSA);

            if (mergedIO.IsContiguous(physicalBlkAddr))
            {
                mergedIO.AddContiguousBlock();
            }
            else
            {
                // Ignore handling the return status.
                mergedIO.Process(arrayName);

                void* newBuffer = currentBufferEntry.GetBlock(bufferIndex);
                mergedIO.SetNewStart(newBuffer, physicalBlkAddr);
            }

            currentLSA.offset++;
        }

        // Net status of the upper operations is gathered from here.
        errorToReturn = mergedIO.Process(arrayName);
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
    CallbackSmartPtr callback,
    std::string& arrayName)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;

    LogicalWriteEntry logicalWriteEntry = {
        .addr = startLSA,
        .blkCnt = static_cast<uint32_t>(blockCount),
        .buffers = &bufferList};

    StripeId stripeId = startLSA.stripeId;

    if (partitionToIO == PartitionType::META_SSD)
    {
        IIOLocker* locker = ArrayService::Instance()->Getter()->GetLocker();
        if (locker->TryLock(arrayName, stripeId) == false)
        {
            return IOSubmitHandlerStatus::TRYLOCK_FAIL;
        }
    }

    std::list<PhysicalWriteEntry> physicalWriteEntries;
    IIOTranslator* translator = ArrayService::Instance()->Getter()->GetTranslator();
    int ret = translator->Convert(
            arrayName, partitionToIO, physicalWriteEntries, logicalWriteEntry);

    if (ret != 0)
    {
        callback->InformError(IOErrorType::GENERIC_ERROR);
        if (partitionToIO == PartitionType::META_SSD)
        {
            IIOLocker* locker = ArrayService::Instance()->Getter()->GetLocker();
            locker->Unlock(arrayName, stripeId);
        }
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
        new ArrayUnlocking(partitionToIO, stripeId, arrayName));
    arrayUnlocking->SetCallee(callback);
    arrayUnlocking->SetWaitingCount(totalIoCount);

    IODispatcher* ioDispatcher = IODispatcherSingleton::Instance();

    for (PhysicalWriteEntry& physicalWriteEntry : physicalWriteEntries)
    {
        for (BufferEntry& buffer : physicalWriteEntry.buffers)
        {
            UbioSmartPtr ubio(new Ubio(buffer.GetBufferPtr(),
                buffer.GetBlkCnt() * Ubio::UNITS_PER_BLOCK, arrayName));

            ubio->dir = UbioDir::Deallocate;
            ubio->SetPba(physicalWriteEntry.addr);
            CallbackSmartPtr event(new InternalWriteCompletion(buffer));
            event->SetCallee(arrayUnlocking);
            ubio->SetCallback(event);

            if (ioDispatcher->Submit(ubio) < 0 ||
                ubio->GetError() != IOErrorType::SUCCESS)
            {
                errorToReturn =
                    _CheckAsyncWriteError(POS_EVENT_ID::REF_COUNT_RAISE_FAIL, arrayName);

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
} // namespace pos
