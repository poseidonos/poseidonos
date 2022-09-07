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

#include "src/bio/ubio.h"

#include <air/Air.h>
#include <stdlib.h>
#include <unistd.h>

#include <sstream>
#include <string>

#include "mk/ibof_config.h"
#include "src/event_scheduler/callback.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/core_const.h"
#include "src/include/i_array_device.h"
#include "src/include/memory.h"
#include "src/include/meta_const.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/qos/qos_manager.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
Ubio::Ubio(void* buffer, uint32_t unitCount, int arrayID)
: dir(UbioDir::Read),
  ubioPrivate(nullptr),
  eventIoType(BackendEvent_Unknown),
  dataBuffer(unitCount * BYTES_PER_UNIT, buffer),
  callback(nullptr),
  syncDone(false),
  retry(false),
  origin(nullptr),
  error(IOErrorType::SUCCESS),
  lba(0),
  uBlock(nullptr),
  arrayDev(nullptr),
  arrayId(arrayID),
  originCore(INVALID_CORE)
{
    SetAsyncMode();
    airlog("Ubio_Constructor", "internal", GetEventType(), 1);
}

Ubio::Ubio(const Ubio& ubio)
: dataBuffer(ubio.dataBuffer),
  callback(nullptr),
  syncDone(false),
  retry(ubio.retry),
  origin(nullptr),
  error(IOErrorType::SUCCESS),
  lba(0),
  uBlock(nullptr),
  arrayDev(nullptr),
  arrayId(ubio.arrayId),
  originCore(INVALID_CORE)
{
    dir = ubio.dir;
    SetAsyncMode();
    ubioPrivate = ubio.ubioPrivate;
    eventIoType = ubio.eventIoType;
    airlog("Ubio_Constructor", "internal", GetEventType(), 1);
}

Ubio::~Ubio(void)
{
    airlog("Ubio_Destructor", "internal", GetEventType(), 1);
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
Ubio::GetEventType(void)
{
    return (eventIoType);
}

void
Ubio::FreeDataBuffer(void)
{
    dataBuffer.Free();
}

void
Ubio::SetSyncMode(void)
{
    syncDone = false;
    sync = true;

    if (uBlock == nullptr)
    {
        uBlock = arrayDev->GetUblock();
    }
}

void
Ubio::SetAsyncMode(void)
{
    sync = false;
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

void
Ubio::ClearOrigin(void)
{
    origin = nullptr;
}

void
Ubio::ResetError(void)
{
    error = IOErrorType::SUCCESS;
}

IOErrorType
Ubio::GetError(void)
{
    return error;
}

void
Ubio::SetError(IOErrorType inputErrorType)
{
    error = inputErrorType;
}

bool
Ubio::CheckRecoveryAllowed(void)
{
    if (arrayDev == nullptr || sync == true)
    {
        return false;
    }
    return true;
}

void
Ubio::SetPba(PhysicalBlkAddr& pba)
{
    if (unlikely(pba.arrayDev == nullptr))
    {
        POS_EVENT_ID eventId = EID(UBIO_INVALID_PBA);
        POS_TRACE_ERROR(eventId, "Invalid PBA for Ubio");
        return;
    }

    lba = pba.lba;
    arrayDev = pba.arrayDev;
}

uint32_t
Ubio::GetOriginCore(void)
{
    return originCore;
}

void
Ubio::SetOriginCore(uint32_t inputOriginCore)
{
    originCore = inputOriginCore;
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
Ubio::Complete(IOErrorType errorType)
{
    error = errorType;
    if (unlikely(sync))
    {
        MarkDone();
    }
}

void
Ubio::MarkDone(void)
{
    if (unlikely(false == sync))
    {
        POS_TRACE_WARN(EID(UBIO_SYNC_FLAG_NOT_SET), "");
        return;
    }
    if (unlikely(true == syncDone))
    {
        POS_TRACE_WARN(EID(UBIO_ALREADY_SYNC_DONE), "");
        return;
    }

    syncDone = true;
}

void
Ubio::WaitDone(void)
{
    if (unlikely(false == sync))
    {
        POS_TRACE_WARN(EID(UBIO_SYNC_FLAG_NOT_SET), "");
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
}

UBlockDevice*
Ubio::GetUBlock(void)
{
    return uBlock.get();
}

IArrayDevice*
Ubio::GetArrayDev(void)
{
    return arrayDev;
}

const PhysicalBlkAddr
Ubio::GetPba(void)
{
    PhysicalBlkAddr pba = {.lba = this->lba,
        .arrayDev = this->arrayDev};

    return pba;
}

uint64_t
Ubio::GetLba(void)
{
    return lba;
}

bool
Ubio::CheckPbaSet(void)
{
    bool isPbaSet = (arrayDev != nullptr);
    return isPbaSet;
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
        POS_EVENT_ID eventId = EID(UBIO_INVALID_ORIGIN_UBIO);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Invalid original Ubio");
        return nullptr;
    }

    return origin;
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

void
Ubio::SetLba(uint64_t lba)
{
    this->lba = lba;
}

void
Ubio::SetUblock(UblockSharedPtr uBlock)
{
    this->uBlock = uBlock;
}

bool
Ubio::NeedRecovery(void) // TODO: will be moved. AWIBOF-2751
{
    if (CheckRecoveryAllowed() == false)
    {
        return false;
    }

    ArrayDeviceState devState = arrayDev->GetState();

    if (devState == ArrayDeviceState::FAULT)
    {
        return true;
    }
    else if (devState == ArrayDeviceState::REBUILD && dir == UbioDir::Read)
    {
        return true;
    }

    uBlock = arrayDev->GetUblock();
    if (uBlock == nullptr)
    {
        POS_EVENT_ID eventId = EID(UBIO_INVALID_DEVICE);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Invalid UblockDevice for Ubio");
        assert(0);
    }

    return false;
}

/*std::string&
Ubio::GetArrayName(void)
{
    return arrayName;
}*/

int
Ubio::GetArrayId(void)
{
    return arrayId;
}

} // namespace pos
