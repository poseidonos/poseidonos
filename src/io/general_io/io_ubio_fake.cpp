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
#include "src/include/pos_event_id.hpp"
#include "src/include/core_const.h"
#include "src/include/memory.h"
#include "src/include/meta_const.h"
#include "src/bio/ubio.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/callback.h"
#include "src/volume/volume_list.h"

namespace pos
{
Ubio::Ubio(void* buffer, uint32_t unitCount, std::string arrayName)
: dir(UbioDir::Read),
  ubioPrivate(nullptr),
  dataBuffer(unitCount * BYTES_PER_UNIT, buffer),
  callback(nullptr),
  retry(false),
  origin(nullptr),
  error(IOErrorType::SUCCESS),
  lba(0),
  uBlock(nullptr),
  arrayDev(nullptr),
  arrayName(arrayName)
{
    SetAsyncMode();
}

Ubio::Ubio(const Ubio& ubio)
: dataBuffer(ubio.dataBuffer),
  callback(nullptr),
  retry(ubio.retry),
  origin(nullptr),
  error(IOErrorType::SUCCESS),
  lba(0),
  uBlock(nullptr),
  arrayDev(nullptr),
  arrayName(ubio.arrayName)
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
Ubio::Complete(IOErrorType errorType)
{
    callback->Execute();
}

void
Ubio::SetPba(PhysicalBlkAddr& pba)
{
    if (unlikely(pba.arrayDev == nullptr))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::UBIO_INVALID_PBA;
        POS_TRACE_ERROR(eventId, PosEventId::GetString(eventId));
        return;
    }

    lba = pba.lba;
    arrayDev = pba.arrayDev;
}

uint32_t
Ubio::GetOriginCore(void)
{
    POS_TRACE_ERROR((int)POS_EVENT_ID::UBIO_INVALID_ORIGINAL_CORE,
        PosEventId::GetString(POS_EVENT_ID::UBIO_INVALID_ORIGINAL_CORE));
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
    PhysicalBlkAddr pba = {.lba = lba,
                           .arrayDev = arrayDev};

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


} // namespace pos
