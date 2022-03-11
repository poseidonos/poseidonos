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

#include "partition_formatter.h"
#include "src/bio/ubio.h"
#include "src/device/base/ublock_device.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/logger/logger.h"
#include "src/include/pos_event_id.h"

namespace pos
{

int
PartitionFormatter::Format(const PartitionPhysicalSize* size, uint32_t arrayId,
    const vector<ArrayDevice*>& devs, IODispatcher* io)
{
    uint32_t blkCount = size->totalSegments *
        size->stripesPerSegment *
        size->blksPerChunk;
    uint64_t totalUnitCount = blkCount * Ubio::UNITS_PER_BLOCK;
    uint32_t unitCount;
    uint64_t startLba = size->startLba;
    uint32_t ubioUnit = UINT32_MAX;
    int result = 0;
    int trimResult = 0;
    uint8_t dummyBuffer[Ubio::BYTES_PER_UNIT];

    do
    {
        if (totalUnitCount >= ubioUnit)
        {
            unitCount = ubioUnit;
        }
        else
        {
            unitCount = totalUnitCount % ubioUnit;
        }

        UbioSmartPtr ubio(new Ubio(dummyBuffer, unitCount, arrayId));
        ubio->dir = UbioDir::Deallocate;

        for (uint32_t i = 0; i < devs.size(); i++)
        {
            if (devs[i]->GetState() == ArrayDeviceState::FAULT)
            {
                continue;
            }

            PhysicalBlkAddr pba = {
                .lba = startLba,
                .arrayDev = devs[i] };
            ubio->SetPba(pba);
            ubio->SetUblock(devs[i]->GetUblock());
            result = io->Submit(ubio, true);
            POS_TRACE_DEBUG(EID(FORMAT_PARTITION_DEBUG_MSG),
                "Try to trim from {} for {} on {}",
                pba.lba, unitCount, devs[i]->GetUblock()->GetName());

            if (result < 0 || ubio->GetError() != IOErrorType::SUCCESS)
            {
                POS_TRACE_ERROR(EID(FORMAT_PARTITION_DEBUG_MSG),
                    "Trim Failed on {}", devs[i]->GetUblock()->GetName());
                trimResult = 1;
            }
        }

        totalUnitCount -= unitCount;
        startLba += unitCount;
    } while (totalUnitCount != 0);

    if (trimResult == 0)
    {
        result = _CheckTrimValue(size->startLba, arrayId, devs, io);

        if (result == 0)
        {
            POS_TRACE_INFO(EID(FORMAT_PARTITION_DEBUG_MSG),
                "Trim Succeeded from {} ", startLba);
        }
        else
        {
            POS_TRACE_ERROR(EID(FORMAT_PARTITION_DEBUG_MSG),
                "Trim Succeeded with wrong value from {}", startLba);
            return result;
            // To Do : Write All Zeroes
        }
        return 0;
    }
    else
    {
        POS_TRACE_ERROR(EID(FORMAT_PARTITION_DEBUG_MSG), "Trim Failed on some devices");
        // To Do : Write All Zeroes
        return trimResult;
    }
}

int
PartitionFormatter::_CheckTrimValue(uint64_t startLba, uint32_t arrayId,
    const vector<ArrayDevice*>& devs, IODispatcher* io)
{
    uint64_t readUnitCount = 1;
    void* readbuffer = Memory<Ubio::BYTES_PER_UNIT>::Alloc(readUnitCount);
    void* zerobuffer = Memory<Ubio::BYTES_PER_UNIT>::Alloc(readUnitCount);
    memset(zerobuffer, 0, Ubio::BYTES_PER_UNIT);
    int result = 0;
    int nonZeroResult = 0;

    UbioSmartPtr readUbio(new Ubio(readbuffer, readUnitCount, arrayId));
    for (uint32_t i = 0; i < devs.size(); i++)
    {
        PhysicalBlkAddr pba = {
            .lba = startLba,
            .arrayDev = devs[i] };
        readUbio->SetPba(pba);
        io->Submit(readUbio, true);
        result = memcmp(readUbio->GetBuffer(), zerobuffer, Ubio::BYTES_PER_UNIT);

        if (result != 0)
        {
            POS_TRACE_ERROR(EID(FORMAT_PARTITION_DEBUG_MSG),
                "Trim Value is not Zero on {}", devs[i]->GetUblock()->GetName());
            nonZeroResult = 1;
        }
    }

    Memory<Ubio::BYTES_PER_UNIT>::Free(readbuffer);
    Memory<Ubio::BYTES_PER_UNIT>::Free(zerobuffer);

    return nonZeroResult;
}

} // namespace pos
