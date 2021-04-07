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

#include "src/io/general_io/ubio.h"

#include <stdlib.h>
#include <unistd.h>

#include <sstream>

#include "mk/ibof_config.h"
#include "src/array/device/array_device.h"
#include "src/device/event_framework_api.h"
#include "src/include/ibof_event_id.hpp"
#include "src/include/memory.h"
#include "src/include/meta_const.h"
#include "src/io/general_io/device_failure.h"
#include "src/logger/logger.h"
#include "src/scheduler/callback.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/event_scheduler.h"

#if defined QOS_ENABLED_BE
#include "src/qos/qos_manager.h"
#endif

namespace ibofos
{
Ubio::Ubio(void* buffer, uint32_t unitCount)
: dir(UbioDir::Read),
  ubioPrivate(nullptr),
#if defined QOS_ENABLED_BE
  eventIoType(BackendEvent_Unknown),
#endif
  dataBuffer(unitCount * BYTES_PER_UNIT, buffer),
  callback(nullptr),
  sectorRba(INVALID_RBA),
  retry(false),
  origin(nullptr),
  error(CallbackError::SUCCESS),
  referenceIncreased(false),
  recoveryIgnored(false)
{
    pba.dev = nullptr;
    pba.lba = INVALID_LBA;

    SetAsyncMode();
}

Ubio::Ubio(const Ubio& ubio)
: dataBuffer(ubio.dataBuffer),
  callback(nullptr),
  pba(ubio.pba),
  sectorRba(ubio.sectorRba),
  retry(ubio.retry),
  origin(nullptr),
  error(CallbackError::SUCCESS),
  referenceIncreased(false)
{
    dir = ubio.dir;
    SetAsyncMode();
    ubioPrivate = ubio.ubioPrivate;

#if defined QOS_ENABLED_BE
    eventIoType = ubio.eventIoType;
#endif
}

Ubio::~Ubio(void)
{
}

#if defined QOS_ENABLED_BE
uint64_t
Ubio::GetUbioSize(void)
{
    return dataBuffer.GetSize();
}
bool
Ubio::IsSyncMode(void)
{
    return sync;
}
void
Ubio::SetEventType(BackendEvent eventType)
{
    eventIoType = eventType;
}
BackendEvent
Ubio::GetEventType()
{
    return (eventIoType);
}
#endif
void
Ubio::FreeDataBuffer(void)
{
    dataBuffer.Free();
}

void
Ubio::SetSyncMode(void)
{
    recoveryIgnored = true;
    syncDone = false;
    sync = true;
}

void
Ubio::SetAsyncMode(void)
{
    sync = false;
    recoveryIgnored = false;
}

void
Ubio::SetCallback(CallbackSmartPtr inputCallback)
{
    callback = inputCallback;
}

void
Ubio::ClearCallback(void)
{
    callback = nullptr;
}

CallbackSmartPtr
Ubio::GetCallback(void)
{
    return callback;
}

bool
Ubio::CheckValid()
{
    bool isWrite = (dir == UbioDir::Write) ? true : false;

    ArrayDeviceState state;
    std::tie(referenceIncreased, state) = GetDev()->GetStatusAndAddPendingIo(isWrite);
    if (referenceIncreased == false && likely(recoveryIgnored == false))
    {
        error = CallbackError::REFERENCE_INCREASE_FAIL;
        EventSmartPtr failure(new DeviceFailure(shared_from_this(), state));

        EventArgument::GetEventScheduler()->EnqueueEvent(failure);
    }

    return referenceIncreased;
}

void
Ubio::CompleteOrigin()
{
    if (origin != nullptr)
    {
        CallbackSmartPtr callee = origin->GetCallback();
        origin->ClearCallback();
        if (callback != nullptr)
        {
            callback->SetCallee(callee);
            origin->Complete(CallbackError::SUCCESS, false);
        }
        origin = nullptr;
    }
}

void
Ubio::Complete(CallbackError errorType, bool executeCallback)
{
    if (errorType != CallbackError::SUCCESS && likely(recoveryIgnored == false))
    {
        error = errorType;
        EventSmartPtr failure(new DeviceFailure(shared_from_this()));
        EventArgument::GetEventScheduler()->EnqueueEvent(failure);
        return;
    }

    CompleteWithoutRecovery(errorType, executeCallback);
}

void
Ubio::ResetError(void)
{
    error = CallbackError::SUCCESS;
}

CallbackError
Ubio::GetError(void)
{
    return error;
}

void
Ubio::SetPba(PhysicalBlkAddr& pbaInput)
{
    if (unlikely(IsInvalidPba(pbaInput)))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UBIO_INVALID_PBA;
        IBOF_TRACE_ERROR(eventId, IbofEventId::GetString(eventId));
        return;
    }

    pba = pbaInput;
}

uint32_t
Ubio::GetOriginCore(void)
{
    return INVALID_CORE;
}

void*
Ubio::GetBuffer(uint32_t blockIndex, uint32_t sectorOffset) const
{
    return dataBuffer.GetAddress(blockIndex, sectorOffset);
}

void*
Ubio::GetWholeBuffer(void) const
{
    return dataBuffer.GetBaseAddress();
}

uint64_t
Ubio::GetSize(void)
{
    return dataBuffer.GetSize();
}

void
Ubio::CompleteWithoutRecovery(CallbackError errorType, bool executeCallback)
{
    if (referenceIncreased)
    {
        GetDev()->RemovePendingIo();
    }

    error = errorType;
    if (unlikely(sync))
    {
        MarkDone();
        return;
    }

    if (executeCallback)
    {
        bool done = false;

        EventSmartPtr eventToExecute(callback);

        callback->InformError(errorType);
        callback = nullptr;

        uint32_t originCore = GetOriginCore();

        if (originCore != INVALID_CORE)
        {
            bool keepCurrentReactor = EventFrameworkApi::IsSameReactorNow(originCore);
            if (keepCurrentReactor)
            {
                done = eventToExecute->Execute();
            }
            else
            {
                done = EventFrameworkApi::SendSpdkEvent(originCore, eventToExecute);
            }
        }

        if (false == done)
        {
            EventArgument::GetEventScheduler()->EnqueueEvent(eventToExecute);
        }
    }
}

void
Ubio::MarkDone(void)
{
    if (unlikely(false == sync))
    {
        IbofEventId::Print(IBOF_EVENT_ID::UBIO_SYNC_FLAG_NOT_SET,
            EventLevel::WARNING);
        return;
    }
    if (unlikely(true == syncDone))
    {
        IbofEventId::Print(IBOF_EVENT_ID::UBIO_ALREADY_SYNC_DONE,
            EventLevel::WARNING);
        return;
    }

    syncDone = true;
}

void
Ubio::WaitDone(void)
{
    if (unlikely(false == sync))
    {
        IbofEventId::Print(IBOF_EVENT_ID::UBIO_SYNC_FLAG_NOT_SET,
            EventLevel::WARNING);
        return;
    }

    // Busy polling can be replaced to cond_variable
    // But it is not applied because all sync IO shuold be replaced to async IO
    while (false == syncDone)
    {
        usleep(1);
    }
}

UbioSmartPtr
Ubio::Split(uint32_t sectors, bool removalFromTail)
{
    UbioSmartPtr newUbio(new Ubio(*this));
    _ReflectSplit(newUbio, sectors, removalFromTail);

    return newUbio;
}

void
Ubio::_ReflectSplit(UbioSmartPtr newUbio, uint32_t sectors,
    bool removalFromTail)
{
    uint64_t removalSize = ChangeSectorToByte(sectors);
    uint64_t remainingSize = GetSize() - removalSize;

    newUbio->dataBuffer.Remove(remainingSize, !removalFromTail);
    dataBuffer.Remove(removalSize, removalFromTail);

    if (removalFromTail)
    {
        newUbio->sectorRba += ChangeByteToSector(remainingSize);
    }
    else
    {
        sectorRba += sectors;
    }
}

bool
Ubio::IsInvalidPba(PhysicalBlkAddr& inputPba)
{
    bool isInvalidPba(inputPba.dev == nullptr);
    return isInvalidPba;
}

bool
Ubio::IsInvalidRba(uint64_t inputRba)
{
    bool isInvalidRba = (inputRba == INVALID_RBA);
    return isInvalidRba;
}

UBlockDevice*
Ubio::GetUBlock(void)
{
    if (unlikely(false == CheckPbaSet()))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_PBA,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_PBA));
        throw IBOF_EVENT_ID::UBIO_INVALID_PBA;
    }
    return pba.dev->uBlock;
}

ArrayDevice*
Ubio::GetDev(void)
{
    if (unlikely(false == CheckPbaSet()))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_PBA,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_PBA));
        throw IBOF_EVENT_ID::UBIO_INVALID_PBA;
    }
    return pba.dev;
}

const PhysicalBlkAddr&
Ubio::GetPba(void)
{
    if (unlikely(false == CheckPbaSet()))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_PBA,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_PBA));
        throw IBOF_EVENT_ID::UBIO_INVALID_PBA;
    }

    return pba;
}

uint64_t
Ubio::GetLba(void)
{
    if (unlikely(false == CheckPbaSet()))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_PBA,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_PBA));
        throw IBOF_EVENT_ID::UBIO_INVALID_PBA;
    }

    return pba.lba;
}

bool
Ubio::CheckPbaSet(void)
{
    bool isPbaSet = (false == IsInvalidPba(pba));
    return isPbaSet;
}

void
Ubio::SetRba(uint64_t inputSectorRba)
{
    if (unlikely(IsInvalidRba(inputSectorRba)))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_RBA,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_RBA));
        return;
    }

    sectorRba = inputSectorRba;
}

uint64_t
Ubio::GetRba(void)
{
    if (unlikely(false == CheckRbaSet()))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_RBA,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_RBA));
        throw IBOF_EVENT_ID::UBIO_INVALID_RBA;
    }

    return sectorRba;
}

bool
Ubio::CheckRbaSet(void)
{
    bool isRbaSet = (false == IsInvalidRba(sectorRba));
    return isRbaSet;
}

void
Ubio::SetOriginUbio(UbioSmartPtr ubio)
{
    origin = ubio;
}

UbioSmartPtr
Ubio::GetOriginUbio(void)
{
    if (unlikely(false == CheckOriginUbioSet()))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UBIO_INVALID_ORIGIN_UBIO;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        return nullptr;
    }

    return origin;
}

void
Ubio::IgnoreRecovery(void)
{
    recoveryIgnored = true;
}

bool
Ubio::CheckOriginUbioSet(void)
{
    bool isOriginUbioSet = (nullptr != origin);
    return isOriginUbioSet;
}

uint32_t
Ubio::GetMemSize(void)
{
    return dataBuffer.GetOriginalSize();
}

bool
Ubio::IsRetry(void)
{
    return retry;
}

void
Ubio::SetRetry(bool retry)
{
    this->retry = retry;
}

} // namespace ibofos
