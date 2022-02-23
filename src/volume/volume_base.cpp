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

#include "volume_base.h"

#include "src/include/branch_prediction.h"
#include "src/include/memory.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"

namespace pos
{

VolumeBase::VolumeBase(std::string arrayName, int arrayIdx, std::string volName, uint64_t volSizeByte)
{
    array = arrayName;
    arrayId = arrayIdx;
    name = volName;
    uuid = "";
    attribute = VolumeAttribute::UserData;
    status = VolumeStatus::Unmounted;
    totalSize = volSizeByte;
    ID = INVALID_VOL_ID;
    POS_TRACE_INFO(POS_EVENT_ID::VOL_CREATED, "Volume name:{} size:{} created", name, totalSize);
}

VolumeBase::VolumeBase(std::string arrayName, int arrayIdx, std::string volName, std::string inputUuid, uint64_t volSizeByte, uint64_t _maxiops, uint64_t _maxbw)
{
    array = arrayName;
    arrayId = arrayIdx;
    name = volName;
    uuid = inputUuid;
    attribute = VolumeAttribute::UserData;
    status = VolumeStatus::Unmounted;
    totalSize = volSizeByte;
    maxiops = _maxiops;
    maxbw = _maxbw;
    miniops = 0;
    minbw = 0;
    ID = INVALID_VOL_ID;
    POS_TRACE_INFO(POS_EVENT_ID::VOL_CREATED, "Volume name:{} uuid:{} size:{} iops:{} bw:{} created",
        name, uuid, totalSize, maxiops, maxbw);
}

VolumeBase::~VolumeBase(void)
{
}

int
VolumeBase::Mount(void)
{
    int errorCode = static_cast<int>(POS_EVENT_ID::SUCCESS);
    if (VolumeStatus::Mounted != status)
    {
        if (ID == INVALID_VOL_ID)
        {
            errorCode = static_cast<int>(POS_EVENT_ID::INVALID_VOL_ID_ERROR);
            POS_TRACE_WARN(errorCode, "invalid vol id. vol name : {}", name);
            return errorCode;
        }
        status = VolumeStatus::Mounted;
        POS_TRACE_INFO(POS_EVENT_ID::VOL_MOUNTED,
            "Volume mounted name: {}", name);
    }
    else
    {
        errorCode = static_cast<int>(POS_EVENT_ID::VOL_ALD_MOUNTED);
        POS_TRACE_WARN(errorCode, "The volume already mounted: {}", name);
    }

    return errorCode;
}

int
VolumeBase::Unmount(void)
{
    int errorCode = static_cast<int>(POS_EVENT_ID::SUCCESS);
    if (VolumeStatus::Unmounted != status)
    {
        status = VolumeStatus::Unmounted;
        POS_TRACE_INFO(POS_EVENT_ID::VOL_UNMOUNTED,
            "Volume unmounted name: {}", name);
    }
    else
    {
        errorCode = static_cast<int>(POS_EVENT_ID::VOL_ALD_UNMOUNTED);
        POS_TRACE_WARN(errorCode, "The volume already unmounted: {}", name);
    }

    return errorCode;
}

void
VolumeBase::LockStatus(void)
{
    statusMutex.lock();
}

void
VolumeBase::UnlockStatus(void)
{
    statusMutex.unlock();
}

void
VolumeBase::SetSubnqn(std::string inputSubNqn)
{
    if (subNqn.empty() == false && inputSubNqn.empty() == false)
    {
        POS_TRACE_INFO(POS_EVENT_ID::VOL_ALD_SET_SUBNQN,
            "The volume already has set subsystem {}, replace to {}",
            subNqn, inputSubNqn);
    }
    subNqn = inputSubNqn;
}

void
VolumeBase::SetUuid(std::string inputUuid)
{
    if (uuid.empty() == false)
    {
        POS_TRACE_INFO(POS_EVENT_ID::VOL_ALD_SET_SUBNQN,
            "The volume already has set uuid {}", uuid);
    }

    if (inputUuid.empty() == false)
    {
        POS_TRACE_INFO(POS_EVENT_ID::VOL_ALD_SET_SUBNQN,
            "The volume has set uuid {}", inputUuid);
    }
    uuid = inputUuid;
}

int
VolumeBase::SetMaxIOPS(uint64_t val)
{
    if ((val != 0 && val < MIN_IOPS_LIMIT) || val > MAX_IOPS_LIMIT)
    {
        return static_cast<int>(POS_EVENT_ID::OUT_OF_QOS_RANGE);
    }
    maxiops = val;
    return static_cast<int>(POS_EVENT_ID::SUCCESS);
}

int
VolumeBase::SetMaxBW(uint64_t val)
{
    if ((val != 0 && val < MIN_BW_LIMIT) || val > MAX_BW_LIMIT)
    {
        return static_cast<int>(POS_EVENT_ID::OUT_OF_QOS_RANGE);
    }
    maxbw = val;
    return static_cast<int>(POS_EVENT_ID::SUCCESS);
}

int
VolumeBase::SetMinIOPS(uint64_t val)
{
    if (val != 0 && val > MAX_IOPS_LIMIT)
    {
        return static_cast<int>(POS_EVENT_ID::OUT_OF_QOS_RANGE);
    }
    miniops = val;
    return static_cast<int>(POS_EVENT_ID::SUCCESS);
}

int
VolumeBase::SetMinBW(uint64_t val)
{
    if (val != 0 && val > MAX_BW_LIMIT)
    {
        return static_cast<int>(POS_EVENT_ID::OUT_OF_QOS_RANGE);
    }
    minbw = val;
    return static_cast<int>(POS_EVENT_ID::SUCCESS);
}

uint64_t
VolumeBase::TotalSize(void)
{
    return totalSize;
}

uint64_t
VolumeBase::UsedSize(void)
{
    IVSAMap* iVSAMap = MapperServiceSingleton::Instance()->GetIVSAMap(array);
    return iVSAMap->GetNumUsedBlks(ID) * BLOCK_SIZE;
}

uint64_t
VolumeBase::RemainingSize(void)
{
    uint64_t totalSize = TotalSize();
    uint64_t usedSize = UsedSize();
    if (usedSize > totalSize)
    {
        POS_TRACE_ERROR(POS_EVENT_ID::VOL_SIZE_NOT_ALIGNED,
            "[NUSE ERROR] Volume:{}'s UsedSize:{} exceeds TotalSize:{}", ID, usedSize, totalSize);
        assert(false);
        return -1;
    }
    else
    {
        POS_TRACE_INFO(POS_EVENT_ID::VOL_SIZE_NOT_ALIGNED,
            "[NUSE Volume:{}] UsedSize:{}, TotalSize:{}", ID, usedSize, totalSize);
        return (totalSize - usedSize);
    }
}

} // namespace pos
