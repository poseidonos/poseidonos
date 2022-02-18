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

#include "src/io/general_io/submit_async_write.h"

#include "src/array/service/array_service_layer.h"
#include "src/bio/ubio.h"
#include "src/event_scheduler/callback.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/io_error_type.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/general_io/array_unlocking.h"
#include "src/io/general_io/internal_write_completion.h"
#include "src/io/general_io/io_submit_handler_count.h"
#include "src/logger/logger.h"
#include "src/state/state_manager.h"
/*To do Remove after adding array Idx by Array*/
#include "src/array_mgmt/array_manager.h"
#include <set>

namespace pos
{
SubmitAsyncWrite::SubmitAsyncWrite(void)
: SubmitAsyncWrite(IOLockerSingleton::Instance(),
      ArrayService::Instance()->Getter()->GetTranslator(),
      IODispatcherSingleton::Instance())
{
}

SubmitAsyncWrite::SubmitAsyncWrite(IIOLocker* locker, IIOTranslator* translator, IODispatcher* ioDispatcher)
: locker(locker),
  translator(translator),
  ioDispatcher(ioDispatcher)
{
}

SubmitAsyncWrite::~SubmitAsyncWrite(void)
{
}

IOSubmitHandlerStatus
SubmitAsyncWrite::Execute(
    std::list<BufferEntry>& bufferList,
    LogicalBlkAddr& startLSA, uint64_t blockCount,
    PartitionType partitionToIO,
    CallbackSmartPtr callback,
    int arrayId, bool needTrim, bool parityOnly)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;

    if (bufferList.empty())
    {
        IOSubmitHandlerCountSingleton::Instance()->callbackNotCalledCount++;
        IOSubmitHandlerCountSingleton::Instance()->pendingWrite--;
        return errorToReturn;
    }
    LogicalWriteEntry logicalWriteEntry = {
        .addr = startLSA,
        .blkCnt = static_cast<uint32_t>(blockCount),
        .buffers = &bufferList};

    LogicalEntry logicalEntry = {
        .addr = startLSA,
        .blkCnt = static_cast<uint32_t>(blockCount)};

    int ret = 0;
    std::list<PhysicalEntry> physicalEntries;
    if (parityOnly == false)
    {
        ret = translator->Translate(
            arrayId, partitionToIO, physicalEntries, logicalEntry);
    }
    std::list<PhysicalWriteEntry> parityPhysicalWriteEntries;
    int parityResult = translator->GetParityList(
        arrayId, partitionToIO, parityPhysicalWriteEntries, logicalWriteEntry);

    if (ret != 0 || parityResult != 0)
    {
        callback->InformError(IOErrorType::GENERIC_ERROR);
        EventSchedulerSingleton::Instance()->EnqueueEvent(callback);
        IOSubmitHandlerCountSingleton::Instance()->callbackNotCalledCount++;
        IOSubmitHandlerCountSingleton::Instance()->pendingWrite--;
        return IOSubmitHandlerStatus::SUCCESS;
    }

    std::set<IArrayDevice*> targetDevices;
    StripeId stripeId = startLSA.stripeId;
    if (partitionToIO == PartitionType::META_SSD)
    {
        if (parityOnly == true)
        {
            
            IOSubmitHandlerCountSingleton::Instance()->callbackNotCalledCount++;
            IOSubmitHandlerCountSingleton::Instance()->pendingWrite--;
            POS_EVENT_ID eventId = POS_EVENT_ID::PARITY_ONLY_NOT_SUPPORTED;
            POS_TRACE_ERROR(eventId, "Meta Partition with parity only is not supported");
            return errorToReturn;
        }
        for (PhysicalEntry& physicalEntry : physicalEntries)
        {
            targetDevices.insert(physicalEntry.addr.arrayDev);
        }

        for (PhysicalWriteEntry& physicalWriteEntry : parityPhysicalWriteEntries)
        {
            targetDevices.insert(physicalWriteEntry.addr.arrayDev);
        }

        bool result = locker->TryLock(targetDevices, stripeId);
        if (result == false)
        {
            IOSubmitHandlerCountSingleton::Instance()->callbackNotCalledCount++;
            IOSubmitHandlerCountSingleton::Instance()->pendingWrite--;
            return IOSubmitHandlerStatus::TRYLOCK_FAIL;
        }
    }

    uint32_t totalIoCount = 0;
    if (parityOnly == false)
    {
        totalIoCount += bufferList.size();
    }
    for (PhysicalWriteEntry& parityPhysicalWriteEntry : parityPhysicalWriteEntries)
    {
        totalIoCount += parityPhysicalWriteEntry.buffers.size();
    }

    callback->SetWaitingCount(1);
    CallbackSmartPtr arrayUnlocking(
        new ArrayUnlocking(targetDevices, stripeId, locker));
    arrayUnlocking->SetCallee(callback);
    arrayUnlocking->SetWaitingCount(totalIoCount);
    arrayUnlocking->SetEventType(callback->GetEventType());

    if (parityOnly == false)
    {
        list<BufferEntry>::iterator iter = bufferList.begin();
        for (PhysicalEntry& physicalEntry : physicalEntries)
        {
            BufferEntry& buffer = *iter;
            UbioSmartPtr ubio = _SetupUbio(arrayId, needTrim, buffer, physicalEntry.addr, arrayUnlocking, callback);

            if (ioDispatcher->Submit(ubio) < 0)
            {
                errorToReturn =
                    _CheckAsyncWriteError(arrayId);
            }
            advance(iter, 1);
        }
    }

    for (PhysicalWriteEntry& physicalWriteEntry : parityPhysicalWriteEntries)
    {
        BufferEntry& buffer = physicalWriteEntry.buffers.front();
        UbioSmartPtr ubio = _SetupUbio(arrayId, needTrim, buffer, physicalWriteEntry.addr, arrayUnlocking, callback);

        if (ioDispatcher->Submit(ubio) < 0)
        {
            errorToReturn =
                _CheckAsyncWriteError(arrayId);
        }
    }

    if (errorToReturn != IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
    {
        errorToReturn = IOSubmitHandlerStatus::SUCCESS;
    }

    return errorToReturn;
}

IOSubmitHandlerStatus
SubmitAsyncWrite::_CheckAsyncWriteError(int arrayId)
{
    /*To do Remove after adding array Idx by Array*/
    IArrayInfo* info = ArrayMgr()->GetInfo(arrayId)->arrayInfo;

    IStateControl* stateControl = StateManagerSingleton::Instance()->GetStateControl(info->GetName());
    if (stateControl->GetState()->ToStateType() == StateEnum::STOP)
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::REF_COUNT_RAISE_FAIL;
        POS_TRACE_ERROR(eventId, "When Io Submit, refcount raise fail");
        return IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP;
    }

    return IOSubmitHandlerStatus::SUCCESS;
}

UbioSmartPtr
SubmitAsyncWrite::_SetupUbio(int arrayId, bool needTrim, BufferEntry& buffer,
    PhysicalBlkAddr addr, CallbackSmartPtr arrayUnlocking, CallbackSmartPtr callback)
{
    UbioSmartPtr ubio(new Ubio(buffer.GetBufferPtr(),
        buffer.GetBlkCnt() * Ubio::UNITS_PER_BLOCK, arrayId));
    if (needTrim == false)
    {
        ubio->dir = UbioDir::Write;
    }
    else
    {
        ubio->dir = UbioDir::Deallocate;
    }
    ubio->SetPba(addr);
    CallbackSmartPtr event(new InternalWriteCompletion(buffer));
    ubio->SetEventType(callback->GetEventType());
    event->SetEventType(ubio->GetEventType());
    event->SetCallee(arrayUnlocking);
    ubio->SetCallback(event);
    return ubio;
}
} // namespace pos
