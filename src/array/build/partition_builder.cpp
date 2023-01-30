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

#include "partition_builder.h"
#include "src/helper/enumerable/query.h"
#include "src/array/partition/stripe_partition.h"
#include "src/array/partition/nvm_partition.h"
#include "src/array/partition/partition_factory.h"
#include "src/logger/logger.h"

namespace pos
{

int
PartitionBuilder::Create(const vector<ArrayDevice*>& devs, RaidType metaRaid,
    RaidType dataRaid, vector<Partition*>& partitions)
{
    if (devs.size() == 0)
    {
        // TODO : temporary code to ignore unit-test path
        return 0;
    }
    ArrayDevice* nvm = Enumerable::First(devs,
        [](auto d) { return d->GetType() == ArrayDeviceType::NVM; });
    vector<ArrayDevice*> dataSsds = Enumerable::Where(devs,
        [](auto d) { return d->GetType() == ArrayDeviceType::DATA; });

    int ret = PartitionFactory::CreateSsdPartitions(dataSsds, nvm->GetSize(),
            metaRaid, dataRaid, partitions);
    if (ret == 0)
    {
        ret = _CreateNvmPartitions(nvm, partitions);
    }
    return ret;
}

int
PartitionBuilder::Load(const vector<pbr::PteData*>& pteList,
    const vector<ArrayDevice*>& devs, vector<Partition*>& partitions)
{
    int ret = 0;
    ArrayDevice* nvm = Enumerable::First(devs,
        [](auto d) { return d->GetType() == ArrayDeviceType::NVM; });
    vector<ArrayDevice*> dataSsds = Enumerable::Where(devs,
        [](auto d) { return d->GetType() == ArrayDeviceType::DATA; });

    if (nvm == nullptr)
    {
        return -1;
    }

    uint64_t totalNvmBlks = nvm->GetSize() / ArrayConfig::BLOCK_SIZE_BYTE;
    for (pbr::PteData* pteData : pteList)
    {
        PartitionType partType = PartitionType(pteData->partType);
        RaidType raidType = RaidType(pteData->raidType);
        StripePartition* partition = new StripePartition(partType,
            dataSsds, raidType);
        ret = partition->Create(pteData->startLba, pteData->lastLba, totalNvmBlks);
        if (ret != 0)
        {
            return ret;
        }
        partitions.push_back(partition);
    }
    ret = _CreateNvmPartitions(nvm, partitions);
    return ret;
}

int
PartitionBuilder::_CreateNvmPartitions(ArrayDevice* nvm, vector<Partition*>& partitions)
{
    uint32_t blksPerStripeOfMetaPart = 0;
    uint32_t blksPerStripeOfDataPart = 0;

    for (Partition* part : partitions)
    {
        if (part->GetType() == PartitionType::META_SSD)
        {
            blksPerStripeOfMetaPart = part->GetLogicalSize()->blksPerStripe;
        }
        else if (part->GetType() == PartitionType::USER_DATA)
        {
            blksPerStripeOfDataPart = part->GetLogicalSize()->blksPerStripe;
        }
    }

    int ret = PartitionFactory::CreateNvmPartitions(nvm, partitions,
        blksPerStripeOfMetaPart, blksPerStripeOfDataPart);
    return ret;
}


} // namespace pos
