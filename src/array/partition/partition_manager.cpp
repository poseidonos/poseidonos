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

#include "partition_manager.h"

#include <functional>

#include "nvm_partition.h"
#include "src/array/array_interface.h"
#include "src/array/device/array_device.h"
#include "src/array/ft/raid1.h"
#include "src/array/ft/raid5.h"
#include "src/array/interface/i_abr_control.h"
#include "src/device/base/ublock_device.h"
#include "src/include/array_config.h"
#include "src/logger/logger.h"
#include "stripe_partition.h"

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

namespace pos
{

PartitionManager::PartitionManager(string array, IAbrControl* abr, AffinityManager* affinityManager)
: arrayName_(array),
  affinityManager(affinityManager)
{
    for (uint32_t i = 0; i < partitions_.size(); i++)
    {
        partitions_[i] = nullptr;
    }
    abrControl = abr;
}

PartitionManager::~PartitionManager()
{
}

const PartitionLogicalSize*
PartitionManager::GetSizeInfo(PartitionType type)
{
    if (nullptr == partitions_[type])
    {
        return nullptr;
    }
    else
    {
        return partitions_[type]->GetLogicalSize();
    }
}

int
PartitionManager::CreateAll(vector<ArrayDevice*> buf,
    vector<ArrayDevice*> data, ArrayInterface* intf, uint32_t arrayIndex)
{
    int ret = _CreateMetaSsd(data, intf, arrayIndex);
    if (ret != 0)
    {
        goto error;
    }
    ret = _CreateUserData(data, buf.front(), intf, arrayIndex);
    if (ret != 0)
    {
        goto error;
    }
    ret = _CreateMetaNvm(buf.front(), intf, arrayIndex);
    if (ret != 0)
    {
        goto error;
    }
    ret = _CreateWriteBuffer(buf.front(), intf, arrayIndex);
    if (ret != 0)
    {
        goto error;
    }

    return 0;

error:
    DeleteAll(intf);
    return ret;
}

int
PartitionManager::_CreateMetaNvm(ArrayDevice* dev, ArrayInterface* intf, uint32_t arrayIndex)
{
    const PartitionLogicalSize* metaSsdSize = nullptr;
    PartitionType partType = PartitionType::META_NVM;
    metaSsdSize = GetSizeInfo(PartitionType::META_SSD);
    if (nullptr == metaSsdSize)
    {
        int eventId = (int)POS_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
        POS_TRACE_ERROR(eventId, "Failed to create partition \"META_NVM\"");
        return eventId;
    }

    PartitionPhysicalSize physicalSize;
    physicalSize.startLba = ArrayConfig::NVM_MBR_SIZE_BYTE / ArrayConfig::SECTOR_SIZE_BYTE;
    physicalSize.blksPerChunk = metaSsdSize->blksPerStripe;
    physicalSize.chunksPerStripe = ArrayConfig::NVM_DEVICE_COUNT;
    physicalSize.stripesPerSegment = ArrayConfig::META_NVM_SIZE /
        (ArrayConfig::BLOCK_SIZE_BYTE * physicalSize.blksPerChunk * physicalSize.chunksPerStripe);
    physicalSize.totalSegments = ArrayConfig::NVM_SEGMENT_SIZE;

    vector<ArrayDevice*> nvm;
    nvm.push_back(dev);

    Partition* partition = new NvmPartition(arrayName_, arrayIndex,
        partType, physicalSize, nvm);
    if (nullptr == partition)
    {
        int eventId = (int)POS_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
        POS_TRACE_ERROR(eventId, "Failed to create partition \"META_NVM\"");
        return eventId;
    }
    partitions_[partType] = partition;
    intf->AddTranslator(partType, partition);
    return 0;
}

int
PartitionManager::_CreateWriteBuffer(ArrayDevice* dev, ArrayInterface* intf, uint32_t arrayIndex)
{
    const PartitionLogicalSize* userDataSize = nullptr;
    PartitionType partType = PartitionType::WRITE_BUFFER;
    userDataSize = GetSizeInfo(PartitionType::USER_DATA);
    if (nullptr == userDataSize)
    {
        int eventId = (int)POS_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
        POS_TRACE_ERROR(eventId, "Failed to create partition \"WRITE_BUFFER\"");
        return eventId;
    }
    const PartitionPhysicalSize* metaNvmSize = nullptr;
    metaNvmSize = partitions_[PartitionType::META_NVM]->GetPhysicalSize();
    if (nullptr == metaNvmSize)
    {
        int eventId = (int)POS_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
        POS_TRACE_ERROR(eventId, "Failed to create partition \"WRITE_BUFFER\"");
        return eventId;
    }

    PartitionPhysicalSize physicalSize;
    uint64_t metaNvmTotalBlks = static_cast<uint64_t>(metaNvmSize->blksPerChunk) * metaNvmSize->stripesPerSegment * metaNvmSize->totalSegments;
    physicalSize.startLba = metaNvmSize->startLba + (metaNvmTotalBlks * ArrayConfig::SECTORS_PER_BLOCK);
    physicalSize.blksPerChunk = userDataSize->blksPerStripe;
    physicalSize.chunksPerStripe = ArrayConfig::NVM_DEVICE_COUNT;
    physicalSize.stripesPerSegment = (dev->GetUblock()->GetSize() / ArrayConfig::BLOCK_SIZE_BYTE - DIV_ROUND_UP(physicalSize.startLba, ArrayConfig::SECTORS_PER_BLOCK)) / userDataSize->blksPerStripe;
    physicalSize.totalSegments = ArrayConfig::NVM_SEGMENT_SIZE;

    vector<ArrayDevice*> nvm;
    nvm.push_back(dev);
    Partition* partition = new NvmPartition(arrayName_, arrayIndex,
        partType, physicalSize, nvm);
    if (nullptr == partition)
    {
        int eventId = (int)POS_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
        POS_TRACE_ERROR(eventId, "Failed to create partition \"WRITE_BUFFER\"");
        return eventId;
    }
    partitions_[partType] = partition;
    intf->AddTranslator(partType, partition);
    return 0;
}

int
PartitionManager::_CreateMetaSsd(vector<ArrayDevice*> devs, ArrayInterface* intf, uint32_t arrayIndex)
{
    PartitionType partType = PartitionType::META_SSD;

    if (0 != devs.size() % 2)
    {
        devs.pop_back();
    }

    ArrayDevice* baseline = _GetBaseline(devs);

    PartitionPhysicalSize physicalSize;
    physicalSize.startLba = ArrayConfig::META_SSD_START_LBA;
    physicalSize.blksPerChunk = ArrayConfig::BLOCKS_PER_CHUNK;
    physicalSize.chunksPerStripe = devs.size();
    physicalSize.stripesPerSegment = ArrayConfig::STRIPES_PER_SEGMENT;
    uint64_t ssdTotalSegments =
        baseline->GetUblock()->GetSize() / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;
    physicalSize.totalSegments =
        DIV_ROUND_UP(ssdTotalSegments * ArrayConfig::META_SSD_SIZE_RATIO, 100);

    Method* method = new Raid1(&physicalSize);
    StripePartition* partition = new StripePartition(arrayName_, arrayIndex,
        partType, physicalSize, devs, method);

    if (nullptr == partition)
    {
        int eventId = (int)POS_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
        POS_TRACE_ERROR(eventId, "Failed to create partition \"META_SSD\"");
        return eventId;
    }

    partitions_[partType] = partition;
    intf->AddTranslator(partType, partition);
    intf->AddRecover(partType, partition);
    intf->AddRebuildTarget(partition);
    return 0;
}

int
PartitionManager::_CreateUserData(const vector<ArrayDevice*> devs,
    ArrayDevice* nvm, ArrayInterface* intf, uint32_t arrayIndex)
{
    PartitionType partType = PartitionType::USER_DATA;
    Partition* metaSsd = nullptr;
    metaSsd = partitions_[PartitionType::META_SSD];
    if (nullptr == metaSsd)
    {
        int eventId = (int)POS_EVENT_ID::ARRAY_PARTITION_LOAD_ERROR;
        POS_TRACE_ERROR(eventId, "Failed to create partition \"USER_DATA\"");
        return eventId;
    }
    const PartitionPhysicalSize* metaSsdSize = nullptr;
    metaSsdSize = metaSsd->GetPhysicalSize();

    PartitionPhysicalSize physicalSize;
    physicalSize.startLba = metaSsdSize->startLba +
        (metaSsdSize->totalSegments * ArrayConfig::SSD_SEGMENT_SIZE_BYTE / ArrayConfig::SECTOR_SIZE_BYTE);
    physicalSize.blksPerChunk = ArrayConfig::BLOCKS_PER_CHUNK;
    physicalSize.chunksPerStripe = devs.size();
    physicalSize.stripesPerSegment = ArrayConfig::STRIPES_PER_SEGMENT;
    ArrayDevice* baseline = _GetBaseline(devs);
    uint64_t ssdTotalSegments =
        baseline->GetUblock()->GetSize() / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;
    uint64_t mbrSegments =
        ArrayConfig::MBR_SIZE_BYTE / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;
    physicalSize.totalSegments =
        ssdTotalSegments - mbrSegments - metaSsdSize->totalSegments;

    uint64_t totalNvmBlks = nvm->GetUblock()->GetSize() / ArrayConfig::BLOCK_SIZE_BYTE;
    uint64_t blksPerStripe = static_cast<uint64_t>(physicalSize.blksPerChunk) * physicalSize.chunksPerStripe;
    uint64_t totalNvmStripes = totalNvmBlks / blksPerStripe;
    PartitionType type = PartitionType::USER_DATA;
    Method* method = new Raid5(&physicalSize, totalNvmStripes, affinityManager);
    StripePartition* partition = new StripePartition(arrayName_, arrayIndex, type, physicalSize, devs, method);
    if (nullptr == partition)
    {
        int eventId = (int)POS_EVENT_ID::ARRAY_PARTITION_LOAD_ERROR;
        POS_TRACE_ERROR(eventId, "Failed to create partition \"USER_DATA\"");
        return eventId;
    }

    partitions_[partType] = partition;
    intf->AddTranslator(partType, partition);
    intf->AddRecover(partType, partition);
    intf->AddRebuildTarget(partition);
    return 0;
}

void
PartitionManager::DeleteAll(ArrayInterface* intf)
{
    for (uint32_t i = 0; i < partitions_.size(); i++)
    {
        if (nullptr != partitions_[i])
        {
            delete partitions_[i];
            partitions_[i] = nullptr;
        }
    }
    intf->ClearInterface();
}

ArrayDevice*
PartitionManager::_GetBaseline(const vector<ArrayDevice*>& devs)
{
    ArrayDevice* baseline = nullptr;
    for (ArrayDevice* dev : devs)
    {
        if (ArrayDeviceState::FAULT != dev->GetState())
        {
            baseline = dev;
        }
    }
    if (nullptr == baseline)
    {
        assert(0);
    }

    return baseline;
}

void
PartitionManager::FormatMetaPartition(vector<ArrayDevice*> data, ArrayInterface* intf, uint32_t arrayIndex)
{
    _CreateMetaSsd(data, intf, arrayIndex);
    auto partition = partitions_[PartitionType::META_SSD];
    partition->Format();
    DeleteAll(intf);
}

} // namespace pos
