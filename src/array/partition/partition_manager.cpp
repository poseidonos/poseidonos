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

#include "../config/array_config.h"
#include "../device/array_device.h"
#include "nvm_partition.h"
#include "src/array/ft/raid1.h"
#include "src/array/ft/raid5.h"
#include "src/device/ublock_device.h"
#include "src/logger/logger.h"
#include "src/master_context/mbr_manager.h"
#include "stripe_partition.h"

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

namespace ibofos
{
PartitionManager::PartitionManager()
{
    for (uint32_t i = 0; i < partitions_.size(); i++)
    {
        partitions_[i] = nullptr;
    }
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
PartitionManager::CreateAll(ArrayDevice* nvm,
    vector<ArrayDevice*> dataDevs)
{
    int ret = _CreateMetaSsd(dataDevs);
    if (ret != 0)
    {
        goto error;
    }
    ret = _CreateUserData(dataDevs, nvm);
    if (ret != 0)
    {
        goto error;
    }
    ret = _CreateMetaNvm(nvm);
    if (ret != 0)
    {
        goto error;
    }
    ret = _CreateWriteBuffer(nvm);
    if (ret != 0)
    {
        goto error;
    }
    return 0;

error:
    DeleteAll();
    return ret;
}

int
PartitionManager::_CreateMetaNvm(ArrayDevice* dev)
{
    const PartitionLogicalSize* metaSsdSize = nullptr;
    metaSsdSize = GetSizeInfo(PartitionType::META_SSD);
    if (nullptr == metaSsdSize)
    {
        int eventId = (int)IBOF_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
        IBOF_TRACE_ERROR(eventId, "Failed to create partition \"META_NVM\"");
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

    Partition* partition = new NvmPartition(PartitionType::META_NVM, physicalSize, nvm);
    if (nullptr == partition)
    {
        int eventId = (int)IBOF_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
        IBOF_TRACE_ERROR(eventId, "Failed to create partition \"META_NVM\"");
        return eventId;
    }
    partitions_[PartitionType::META_NVM] = partition;
    return 0;
}

int
PartitionManager::_CreateWriteBuffer(ArrayDevice* dev)
{
    const PartitionLogicalSize* userDataSize = nullptr;
    userDataSize = GetSizeInfo(PartitionType::USER_DATA);
    if (nullptr == userDataSize)
    {
        int eventId = (int)IBOF_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
        IBOF_TRACE_ERROR(eventId, "Failed to create partition \"WRITE_BUFFER\"");
        return eventId;
    }
    const PartitionPhysicalSize* metaNvmSize = nullptr;
    metaNvmSize = partitions_[PartitionType::META_NVM]->GetPhysicalSize();
    if (nullptr == metaNvmSize)
    {
        int eventId = (int)IBOF_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
        IBOF_TRACE_ERROR(eventId, "Failed to create partition \"WRITE_BUFFER\"");
        return eventId;
    }

    PartitionPhysicalSize physicalSize;
    uint64_t metaNvmTotalBlks = static_cast<uint64_t>(metaNvmSize->blksPerChunk) * metaNvmSize->stripesPerSegment * metaNvmSize->totalSegments;
    physicalSize.startLba = metaNvmSize->startLba + (metaNvmTotalBlks * ArrayConfig::SECTORS_PER_BLOCK);
    physicalSize.blksPerChunk = userDataSize->blksPerStripe;
    physicalSize.chunksPerStripe = ArrayConfig::NVM_DEVICE_COUNT;
    physicalSize.stripesPerSegment = (dev->uBlock->GetSize() / ArrayConfig::BLOCK_SIZE_BYTE - DIV_ROUND_UP(physicalSize.startLba, ArrayConfig::SECTORS_PER_BLOCK)) / userDataSize->blksPerStripe;
    physicalSize.totalSegments = ArrayConfig::NVM_SEGMENT_SIZE;

    vector<ArrayDevice*> nvm;
    nvm.push_back(dev);
    Partition* partition = new NvmPartition(PartitionType::WRITE_BUFFER,
        physicalSize, nvm);
    if (nullptr == partition)
    {
        int eventId = (int)IBOF_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
        IBOF_TRACE_ERROR(eventId, "Failed to create partition \"WRITE_BUFFER\"");
        return eventId;
    }
    partitions_[PartitionType::WRITE_BUFFER] = partition;
    return 0;
}

int
PartitionManager::_CreateMetaSsd(vector<ArrayDevice*> devs)
{
    MbrManager& mbrManager = *MbrManagerSingleton::Instance();
    int arrayNum = 0;
    bool isInitialized = mbrManager.GetMfsInit(arrayNum);

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
        baseline->uBlock->GetSize() / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;
    physicalSize.totalSegments =
        DIV_ROUND_UP(ssdTotalSegments * ArrayConfig::META_SSD_SIZE_RATIO, 100);

    PartitionType type = PartitionType::META_SSD;
    Method* method = new Raid1(&physicalSize);
    Partition* partition = new StripePartition(type, physicalSize, devs, method);

    if (nullptr == partition)
    {
        int eventId = (int)IBOF_EVENT_ID::ARRAY_PARTITION_CREATION_ERROR;
        IBOF_TRACE_ERROR(eventId, "Failed to create partition \"META_SSD\"");
        return eventId;
    }

    if (isInitialized == false)
    {
        partition->Format();
    }
    partitions_[type] = partition;

    return 0;
}

int
PartitionManager::_CreateUserData(const vector<ArrayDevice*> devs,
    const ArrayDevice* nvm)
{
    Partition* metaSsd = nullptr;
    metaSsd = partitions_[PartitionType::META_SSD];
    if (nullptr == metaSsd)
    {
        int eventId = (int)IBOF_EVENT_ID::ARRAY_PARTITION_LOAD_ERROR;
        IBOF_TRACE_ERROR(eventId, "Failed to create partition \"USER_DATA\"");
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
        baseline->uBlock->GetSize() / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;
    uint64_t mbrSegments =
        ArrayConfig::MBR_SIZE_BYTE / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;
    physicalSize.totalSegments =
        ssdTotalSegments - mbrSegments - metaSsdSize->totalSegments;

    uint64_t totalNvmBlks = nvm->uBlock->GetSize() / ArrayConfig::BLOCK_SIZE_BYTE;
    uint64_t blksPerStripe = physicalSize.blksPerChunk * physicalSize.chunksPerStripe;
    uint64_t totalNvmStripes = totalNvmBlks / blksPerStripe;
    PartitionType type = PartitionType::USER_DATA;
    Method* method = new Raid5(&physicalSize, totalNvmStripes);
    Partition* partition = new StripePartition(type, physicalSize, devs, method);
    if (nullptr == partition)
    {
        int eventId = (int)IBOF_EVENT_ID::ARRAY_PARTITION_LOAD_ERROR;
        IBOF_TRACE_ERROR(eventId, "Failed to create partition \"USER_DATA\"");
        return eventId;
    }

    partitions_[type] = partition;
    return 0;
}

void
PartitionManager::DeleteAll()
{
    for (uint32_t i = 0; i < partitions_.size(); i++)
    {
        if (nullptr != partitions_[i])
        {
            delete partitions_[i];
            partitions_[i] = nullptr;
        }
    }
}

int
PartitionManager::Translate(const PartitionType type,
    PhysicalBlkAddr& dst,
    const LogicalBlkAddr& src)
{
    return partitions_[type]->Translate(dst, src);
}

int
PartitionManager::Convert(const PartitionType type,
    list<PhysicalWriteEntry>& dst,
    const LogicalWriteEntry& src)
{
    return partitions_[type]->Convert(dst, src);
}

void
PartitionManager::Rebuild(ArrayDevice* target, RebuildResultCallback cb)
{
    rebuildJobs.progress = new RebuildProgress();
    rebuildJobs.resultCb = cb;
    uint32_t totalStripes = 0;
    rebuildMtx.lock();
    for (Partition* p : partitions_)
    {
        if (p != nullptr)
        {
            int devIdx = p->FindDevice(target);
            if (devIdx >= 0)
            {
                Method* method = p->GetMethod();
                RebuildBehavior* behavior = method->GetRebuildBehavior();
                RebuildContext* ctx = behavior->GetContext();
                ctx->faultIdx = devIdx;
                ctx->prog = rebuildJobs.progress;
                ctx->completeHandler = bind(&PartitionManager::_RebuildDone, this);
                RebuildJob* job = new RebuildJob(p, behavior);
                rebuildJobs.jobList.push_back(job);
                totalStripes += job->GetSize();
            }
        }
    }
    rebuildMtx.unlock();

    if (!rebuildJobs.jobList.empty())
    {
        rebuildJobs.progress->SetTotal(totalStripes);
        rebuildJobs.state = RebuildState::READY;
        _DoRebuilding(rebuildJobs.jobList.front());
    }
    else
    {
        delete rebuildJobs.progress;
        rebuildJobs.progress = nullptr;
        rebuildJobs.state = RebuildState::NO_TARGET;
        _RebuildDone();
    }
}

void
PartitionManager::_DoRebuilding(RebuildJob* job)
{
    if (job->GetResult() == RebuildState::READY)
    {
        job->Start();
    }
    else
    {
        _RebuildDone();
    }
}

void
PartitionManager::StopRebuilding()
{
    for (RebuildJob* job : rebuildJobs.jobList)
    {
        job->Stop();
    }

    _WaitRebuildingDone();
}

int
PartitionManager::RebuildRead(UbioSmartPtr ubio)
{
    if (true == ubio->IsRetry())
    {
        return -1;
    }

    ubio->SetRetry(true);

    uint64_t lba = ubio->GetLba();
    ArrayDevice* dev = ubio->GetDev();

    for (uint32_t i = 0; i < partitions_.size(); i++)
    {
        Partition* ptn = partitions_[i];
        if (nullptr != ptn && ptn->IsValidLba(lba) && (ptn->FindDevice(dev) >= 0))
        {
            return ptn->RebuildRead(ubio);
        }
    }

    return -1;
}

bool
PartitionManager::TryLock(PartitionType type, StripeId stripeId)
{
    if (partitions_[type] != nullptr)
    {
        return partitions_[type]->TryLock(stripeId);
    }
    return false;
}

void
PartitionManager::Unlock(PartitionType type, StripeId stripeId)
{
    if (partitions_[type] != nullptr)
    {
        partitions_[type]->Unlock(stripeId);
    }
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
PartitionManager::_RebuildDone()
{
    rebuildMtx.lock();
    if (!rebuildJobs.jobList.empty())
    {
        RebuildJob* job = rebuildJobs.jobList.front();
        if (rebuildJobs.state < job->GetResult())
        {
            rebuildJobs.state = job->GetResult();
        }
        job->Complete();
        delete job;
        rebuildJobs.jobList.pop_front();
    }
    rebuildMtx.unlock();

    if (rebuildJobs.jobList.empty() == true)
    {
        delete rebuildJobs.progress;
        rebuildJobs.progress = nullptr;
        RebuildState result = rebuildJobs.state;
        switch (result)
        {
            case RebuildState::PASS:
                IBOF_TRACE_INFO((int)IBOF_EVENT_ID::REBUILD_RESULT_PASS,
                    "rebuild complete sucessfully");
                break;
            case RebuildState::FAIL:
                IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::REBUILD_FAILED,
                    "rebuild failure");
                break;
            case RebuildState::CANCELLED:
                IBOF_TRACE_WARN((int)IBOF_EVENT_ID::REBUILD_RESULT_CANCELLED,
                    "rebuild cancelled");
                break;
            case RebuildState::NO_TARGET:
                IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::REBUILD_DEBUG_MSG,
                    "the device detached is not belong to any partition");
                break;
            default:
                break;
        }
        rebuildJobs.resultCb(result);
        std::unique_lock<std::mutex> lock(rebuildMtx);
        rebuildCv.notify_one();
    }
    else if (rebuildJobs.state == RebuildState::FAIL)
    {
        _RebuildDone();
    }
    else
    {
        _DoRebuilding(rebuildJobs.jobList.front());
    }
}

void
PartitionManager::_WaitRebuildingDone()
{
    std::unique_lock<std::mutex> lock(rebuildMtx);
    while (rebuildJobs.jobList.empty() != true)
    {
        rebuildCv.wait(lock);
    }
}

uint32_t
PartitionManager::GetRebuildingProgress()
{
    if (rebuildJobs.progress != nullptr)
    {
        return rebuildJobs.progress->Current();
    }

    return 0;
}
} // namespace ibofos
