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

#include <sstream>

#include "src/array/device/array_device.h"
#include "src/include/branch_prediction.h"
#include "src/include/ibof_event_id.hpp"
#include "src/include/memory.h"
#include "src/include/meta_const.h"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"
#include "src/scheduler/callback.h"
#include "src/volume/volume_list.h"

namespace ibofos
{
Ubio::Ubio(void* buffer, uint32_t unitCount)
: dir(UbioDir::Read),
  ubioPrivate(nullptr),
  dataBuffer(unitCount * BYTES_PER_UNIT, buffer),
  callback(nullptr),
  sectorRba(INVALID_RBA),
  retry(false),
  origin(nullptr),
  error(CallbackError::SUCCESS)
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
  error(CallbackError::SUCCESS)
{
    dir = ubio.dir;
    SetAsyncMode();

    ubioPrivate = ubio.ubioPrivate;
}

Ubio::~Ubio(void)
{
}

void
Ubio::SetAsyncMode(void)
{
    sync = false;
}

void
Ubio::Complete(CallbackError errorType, bool executeCallback)
{
    callback->Execute();
}

void
Ubio::SetPba(PhysicalBlkAddr& pbaInput)
{
    if (unlikely(IsInvalidPba(pbaInput)))
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::UBIO_INVALID_PBA,
            IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_PBA));
        return;
    }

    pba = pbaInput;
}

uint32_t
Ubio::GetOriginCore(void)
{
    IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_ORIGINAL_CORE,
        IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_ORIGINAL_CORE));
    return INVALID_CORE;
}

void*
Ubio::GetBuffer(uint32_t blockIndex, uint32_t sectorOffset) const
{
    return dataBuffer.GetAddress(blockIndex, sectorOffset);
}

uint64_t
Ubio::GetSize(void)
{
    return dataBuffer.GetSize();
}

void
Ubio::MarkDone(void)
{
    assert(true == sync);
    assert(false == syncDone);

    syncDone = true;
}

bool
Ubio::IsInvalidPba(PhysicalBlkAddr& inputPba)
{
    return (inputPba.dev == nullptr);
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
    return (false == IsInvalidPba(pba));
}

} // namespace ibofos
