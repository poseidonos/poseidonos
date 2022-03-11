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

#include "ssd_partition_builder.h"
#include "src/device/base/ublock_device.h"
#include "src/array/partition/stripe_partition.h"
#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/helper/enumerable/query.h"
#include "src/helper/calc/calc.h"

namespace pos
{

int
SsdPartitionBuilder::Build(uint64_t startLba, Partitions& out)
{
    POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "SsdPartitionBuilder::Build, devCnt: {}", option.devices.size());
    POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "Create StripePartition ({})", PARTITION_TYPE_STR[option.partitionType]);
    Partition* base = nullptr;
    StripePartition* impl = new StripePartition(option.partitionType, option.devices, option.raidType);
    uint64_t totalNvmBlks = 0;
    if (option.nvm != nullptr)
    {
        totalNvmBlks = option.nvm->GetUblock()->GetSize() / ArrayConfig::BLOCK_SIZE_BYTE;
    }
    int ret = impl->Create(startLba, _GetSegmentCount(), totalNvmBlks);
    base = impl;
    if (ret != 0)
    {
        return ret;
    }
    else
    {
        out[option.partitionType] = base;
        if (next != nullptr)
        {
            uint64_t nextLba = base->GetLastLba();
            return next->Build(nextLba, out);
        }
    }

    return 0;
}

uint32_t
SsdPartitionBuilder::_GetSegmentCount(void)
{
    if (option.partitionType == PartitionType::JOURNAL_SSD)
    {
        return ArrayConfig::JOURNAL_PART_SEGMENT_SIZE;
    }

    uint64_t baseCapa = _GetMinCapacity(option.devices);
    uint64_t maxCapa = _GetMaxCapacity(option.devices);
    if (baseCapa != maxCapa)
    {
        POS_TRACE_WARN(EID(CREATE_ARRAY_DEBUG_MSG), "Partitions are constructed with hetero device sizes, resulting in truncation. max:{}, min:{}",
            maxCapa, baseCapa);
    }

    uint64_t ssdTotalSegments = baseCapa / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;
    uint64_t metaSegCnt = DIV_ROUND_UP(ssdTotalSegments * ArrayConfig::META_SSD_SIZE_RATIO, (uint64_t)(100));

    if (option.partitionType == PartitionType::META_SSD)
    {
        return metaSegCnt;
    }
    else if (option.partitionType == PartitionType::USER_DATA)
    {
        uint64_t mbrSegments = ArrayConfig::MBR_SIZE_BYTE / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;
        return ssdTotalSegments - mbrSegments - metaSegCnt;
    }
    return 0;
}

uint64_t
SsdPartitionBuilder::_GetMinCapacity(const vector<ArrayDevice*>& devs)
{
    auto&& devList = Enumerable::Where(devs,
        [](auto d) { return d->GetState() != ArrayDeviceState::FAULT; });

    ArrayDevice* min = Enumerable::Minimum(devList,
        [](auto d) { return d->GetUblock()->GetSize(); });

    return min->GetUblock()->GetSize();
}

uint64_t
SsdPartitionBuilder::_GetMaxCapacity(const vector<ArrayDevice*>& devs)
{
    auto&& devList = Enumerable::Where(devs,
        [](auto d) { return d->GetState() != ArrayDeviceState::FAULT; });

    ArrayDevice* max = Enumerable::Maximum(devList,
        [](auto d) { return d->GetUblock()->GetSize(); });

    return max->GetUblock()->GetSize();
}

} // namespace pos
