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

#include "partition_manager.h"
#include "partition_factory.h"

#include "src/array/partition/partition_services.h"
#include "src/array/device/array_device.h"
#include "src/device/base/ublock_device.h"
#include "src/include/array_config.h"
#include "src/logger/logger.h"
#include "src/helper/enumerable/query.h"
#include "src/array/partition/partition.h"

#include <functional>
#include <algorithm>

namespace pos
{

PartitionManager::PartitionManager(void)
{
    partitions.fill(nullptr);
}

PartitionManager::~PartitionManager(void)
{
    DeletePartitions();
}

const PartitionLogicalSize*
PartitionManager::GetSizeInfo(PartitionType type)
{
    if (nullptr == partitions[type])
    {
        return nullptr;
    }
    else
    {
        return partitions[type]->GetLogicalSize();
    }
}

const PartitionPhysicalSize*
PartitionManager::GetPhysicalSize(PartitionType type)
{
    if (nullptr == partitions[type])
    {
        return nullptr;
    }
    else
    {
        return partitions[type]->GetPhysicalSize();
    }
}

int
PartitionManager::CreatePartitions(ArrayDevice* nvm, vector<ArrayDevice*> data,
    RaidTypeEnum metaRaid, RaidTypeEnum dataRaid, IPartitionServices* svc)
{
    POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "PartitionManager::CreatePartition, MetaRaid:{}, DataRaid:{}",
        RaidType(metaRaid).ToString(), RaidType(dataRaid).ToString());
    Partitions out;
    out.fill(nullptr);
    int ret = PartitionFactory::CreatePartitions(nvm, data, metaRaid, dataRaid, out);
    if (ret == 0)
    {
        for (Partition* part : out)
        {
            if (part != nullptr)
            {
                PartitionType partType = part->GetType();
                partitions[partType] = part;
                part->RegisterService(svc);
            }
        }
    }
    else
    {
        POS_TRACE_WARN(ret, "Error occured during the creating partitions");
    }

    return ret;
}

void
PartitionManager::DeletePartitions()
{
    for (size_t i = 0; i < partitions.size(); i++)
    {
        if (nullptr != partitions[i])
        {
            delete partitions[i];
            partitions[i] = nullptr;
        }
    }
}

void
PartitionManager::FormatPartition(PartitionType type, uint32_t arrayId, IODispatcher* io)
{
    Partition* partition = partitions[type];
    if (partition != nullptr)
    {
        PartitionFormatter::Format(partition->GetPhysicalSize(), arrayId,
            partition->GetDevs(), io);
    }
    else
    {
        POS_TRACE_ERROR(EID(CREATE_ARRAY_DEBUG_MSG), "Failed to format {} partition", PARTITION_TYPE_STR[type]);
    }
}

RaidState
PartitionManager::GetRaidState(void)
{
    RaidState metaRs = partitions[PartitionType::META_SSD]->GetRaidState();
    RaidState dataRs = partitions[PartitionType::USER_DATA]->GetRaidState();
    RaidState res = max(metaRs, dataRs);
    POS_TRACE_INFO(EID(RAID_DEBUG_MSG), "Meta RS: {} , Data RS: {}, Res: {}", metaRs, dataRs, res);
    return res;
}

RaidTypeEnum
PartitionManager::GetRaidType(PartitionType type)
{
    if (partitions[type] != nullptr)
    {
        return partitions[type]->GetRaidType();
    }
    return RaidTypeEnum::NONE;
}
} // namespace pos
