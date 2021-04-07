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


#include <iostream>

#include "io_submit_handler_test.h"
#include "src/array/array.h"
#include "src/array/device/array_device.h"
#include "src/array/partition/partition.h"
#include "src/array/partition/partition_size_info.h"

namespace ibofos
{
static PartitionLogicalSize userDataPartitionLogicalSize;

static ArrayDevice* arrayDevice[IOSubmitHandlerTest::SSD_COUNT];

Array::Array()
: metaMgr_(nullptr),
  devMgr_(nullptr)
{
    userDataPartitionLogicalSize.blksPerChunk = BLOCKS_IN_CHUNK;
    userDataPartitionLogicalSize.blksPerStripe =
        BLOCKS_IN_CHUNK * IOSubmitHandlerTest::SSD_COUNT;
}

Array::~Array()
{
    for (ArrayDevice* device : arrayDevice)
    {
        if (nullptr != device)
        {
            delete device;
        }
    }
}

const PartitionLogicalSize*
Array::GetSizeInfo(PartitionType type)
{
    const PartitionLogicalSize* sizeInfo = &userDataPartitionLogicalSize;
    return sizeInfo;
}

int
Array::Translate(const PartitionType type, PhysicalBlkAddr& dst,
    const LogicalBlkAddr& src)
{
    uint32_t deviceIndex = src.offset / BLOCKS_IN_CHUNK;

    if (nullptr == arrayDevice[deviceIndex])
    {
        arrayDevice[deviceIndex] = new ArrayDevice(
            reinterpret_cast<UBlockDevice*>(deviceIndex));
    }
    arrayDevice[deviceIndex]->uBlock =
        reinterpret_cast<UBlockDevice*>(deviceIndex);

    dst =
        {
            .dev = arrayDevice[deviceIndex],
            .lba = ChangeBlockToSector(
                src.stripeId * userDataPartitionLogicalSize.blksPerStripe),
        };
    dst.lba += ChangeBlockToSector(src.offset);

    return 0;
}

int
Array::Convert(const PartitionType type, list<PhysicalWriteEntry>& dst,
    const LogicalWriteEntry& src)
{
    LogicalBlkAddr startLSA = src.addr;
    for (BufferEntry buffer : *src.buffers)
    {
        PhysicalWriteEntry physicalWriteEntry;
        Translate(type, physicalWriteEntry.addr, startLSA);

        physicalWriteEntry.blkCnt = buffer.GetBlkCnt();
        physicalWriteEntry.buffers.push_back(buffer);
        dst.push_back(physicalWriteEntry);

        startLSA.offset += buffer.GetBlkCnt();
    }

    return 0;
}

int
Array::DetachDevice(UBlockDevice* uBlock)
{
    return 0;
}

int
Array::RebuildRead(UbioSmartPtr ubio)
{
    return 0;
}

bool
Array::TryLock(PartitionType type, StripeId stripeId)
{
    return true;
}

void
Array::Unlock(PartitionType type, StripeId stripeId)
{
}

} // namespace ibofos
