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

#include "partition_factory.h"
#include "nvm_partition.h"
#include "stripe_partition.h"
#include "ssd_partition_builder.h"
#include "nvm_partition_builder.h"
#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"

namespace pos
{
int
PartitionFactory::CreateSsdPartitions(vector<ArrayDevice*> devs, uint64_t nvmSizeInByte,
    RaidTypeEnum metaRaid, RaidTypeEnum dataRaid, vector<Partition*>& partitions)
{
    POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "Try to split SSD partitions");
    vector<SsdPartitionBuilder*> builders;
    {
        POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "Prepare Partition Builder for JOURNAL_SSD");
        RaidTypeEnum journalRaid = metaRaid;
        SsdPartitionOptions option(PartitionType::JOURNAL_SSD, journalRaid, devs, nvmSizeInByte);
        builders.push_back(new SsdPartitionBuilder(option));
    }
    {
        POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "Prepare Partition Builder for META_SSD");
        SsdPartitionOptions option(PartitionType::META_SSD, metaRaid, devs, nvmSizeInByte);
        builders.push_back(new SsdPartitionBuilder(option));
    }
    {
        POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "Prepare Partition Builder for USER_DATA");
        SsdPartitionOptions option(PartitionType::USER_DATA, dataRaid, devs, nvmSizeInByte);
        builders.push_back(new SsdPartitionBuilder(option));
    }

    SsdPartitionBuilder* prev = builders.front();
    for (size_t i = 1; i < builders.size(); i++)
    {
        prev->SetNext(builders[i]);
        prev = builders[i];
    }
    int ret = builders.front()->Build(ArrayConfig::SSD_PARTITION_START_LBA, partitions);
    if (ret != 0)
    {
        POS_TRACE_WARN(ret, "Failed to invoke a chain of partition builders. Cleaning up the partition objects");
        for (Partition* part : partitions)
        {
            if (part != nullptr)
            {
                delete part;
            }
        }
        partitions.clear();
    }

    for (auto builder : builders)
    {
        delete builder;
    }
    builders.clear();

    POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "SSD partitions are created");
    return ret;
}

int
PartitionFactory::CreateNvmPartitions(ArrayDevice* nvm, vector<Partition*>& partitions,
    uint32_t blksPerStripeOfMetaPart, uint32_t blksPerStripeOfDataPart)
{
    vector<NvmPartitionBuilder*> builders;
    NvmPartitionOptions metaOpt(PartitionType::META_NVM, nvm, blksPerStripeOfMetaPart);
    builders.push_back(new NvmPartitionBuilder(metaOpt));

    NvmPartitionOptions wbOpt(PartitionType::WRITE_BUFFER, nvm, blksPerStripeOfDataPart);
    builders.push_back(new NvmPartitionBuilder(wbOpt));

    NvmPartitionBuilder* prev = builders.front();
    for (size_t i = 1; i < builders.size(); i++)
    {
        prev->SetNext(builders[i]);
        prev = builders[i];
    }

    int ret = builders.front()->Build(ArrayConfig::NVM_MBR_SIZE_BYTE / ArrayConfig::SECTOR_SIZE_BYTE, partitions);
    if (ret == 0)
    {
        POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "NVM partitions are created");
    }

    for (auto builder : builders)
    {
        delete builder;
    }
    return ret;
}

} // namespace pos
