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

#include "partition.h"

#include "../config/array_config.h"
#include "../device/array_device.h"
#include "src/array/ft/method.h"
#include "src/array/ft/stripe_locker.h"
#include "src/device/ublock_device.h"

namespace ibofos
{
Partition::Partition(PartitionType type,
    PartitionPhysicalSize physicalSize,
    vector<ArrayDevice*> devs,
    Method* method)
: type_(type),
  physicalSize_(physicalSize),
  devs_(devs),
  method_(method)
{
    logicalSize_ = {
        .minWriteBlkCnt = 1,
        .blksPerChunk = physicalSize_.blksPerChunk,
        .blksPerStripe =
            physicalSize_.blksPerChunk * physicalSize_.chunksPerStripe,
        .chunksPerStripe = physicalSize_.chunksPerStripe,
        .stripesPerSegment = physicalSize_.stripesPerSegment,
        .totalStripes =
            physicalSize_.stripesPerSegment * physicalSize_.totalSegments,
        .totalSegments = physicalSize_.totalSegments};
}

Partition::~Partition()
{
    delete method_;
    method_ = nullptr;
}

int
Partition::FindDevice(ArrayDevice* target)
{
    int i = 0;
    for (ArrayDevice* dev : devs_)
    {
        if (dev == target)
        {
            return i;
        }
        i++;
    }

    return -1;
}

const PartitionLogicalSize*
Partition::GetLogicalSize()
{
    return &logicalSize_;
}

const PartitionPhysicalSize*
Partition::GetPhysicalSize()
{
    return &physicalSize_;
}

bool
Partition::IsValidLba(uint64_t lba)
{
    uint64_t endLba = 0;
    endLba = physicalSize_.startLba + static_cast<uint64_t>(ArrayConfig::SECTORS_PER_BLOCK) * physicalSize_.blksPerChunk * physicalSize_.stripesPerSegment * physicalSize_.totalSegments;

    if (physicalSize_.startLba > lba || endLba <= lba)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool
Partition::_IsValidAddress(const LogicalBlkAddr& lsa)
{
    if (lsa.stripeId < logicalSize_.totalStripes && lsa.offset < logicalSize_.blksPerStripe)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool
Partition::_IsValidEntry(const LogicalWriteEntry& entry)
{
    if (entry.addr.stripeId < logicalSize_.totalStripes && (entry.addr.offset + entry.blkCnt) <= logicalSize_.blksPerStripe && entry.blkCnt >= logicalSize_.minWriteBlkCnt)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool
Partition::TryLock(StripeId stripeId)
{
    if (method_ != nullptr)
    {
        StripeLocker* locker = method_->GetStripeLocker();

        if (locker != nullptr)
        {
            return locker->TryLock(stripeId);
        }
    }

    return true;
}

void
Partition::Unlock(StripeId stripeId)
{
    if (method_ != nullptr)
    {
        StripeLocker* locker = method_->GetStripeLocker();

        if (locker != nullptr)
        {
            locker->Unlock(stripeId);
        }
    }
}

} // namespace ibofos
