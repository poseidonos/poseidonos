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

VolumeBase::VolumeBase(int arrayIdx, std::string arrayName, DataAttribute dataAttribute,
            std::string volName, uint64_t volSizeByte, uint32_t nsid,
            ReplicationRole voluemRole)
: VolumeAttribute(arrayIdx, arrayName, dataAttribute),
  StatusProperty(volName, volSizeByte),
  NetworkProperty(nsid),
  PerfomanceProperty(0, 0, 0, 0),
  ReplicationProperty(voluemRole)
{
    ID = INVALID_VOL_ID;
    POS_TRACE_INFO(EID(CREATE_VOL_DEBUG_MSG), "Volume name:{} size:{} created", volName, volSizeByte);
}

VolumeBase::VolumeBase(int arrayIdx, std::string arrayName, DataAttribute dataAttribute, std::string inputUuid,
            std::string volName, uint64_t volSizeByte, uint32_t nsid,
            uint64_t _maxiops, uint64_t _miniops, uint64_t _maxbw, uint64_t _minbw,
            ReplicationRole voluemRole)
: VolumeAttribute(arrayIdx, arrayName, dataAttribute, inputUuid),
  StatusProperty(volName, volSizeByte),
  NetworkProperty(nsid),
  PerfomanceProperty(_maxiops, _miniops, _maxbw, _minbw),
  ReplicationProperty(voluemRole)
{
    ID = INVALID_VOL_ID;
    POS_TRACE_INFO(EID(CREATE_VOL_DEBUG_MSG), "Volume name:{} uuid:{} size:{} iops:{} bw:{} attribute:{}, (0:User 1:WAL) created",
        volName, inputUuid, volSizeByte, _maxiops, _maxbw, dataAttribute);
}

VolumeBase::~VolumeBase(void)
{
}

int
VolumeBase::Mount(void)
{
    int errorCode = EID(SUCCESS);
    if (VolumeMountStatus::Mounted != GetVolumeMountStatus())
    {
        if (ID == INVALID_VOL_ID)
        {
            errorCode = EID(VOL_INTERNAL_INVALID_ID);
            POS_TRACE_WARN(errorCode, "invalid vol id. vol name : {}", GetVolumeName());
            return errorCode;
        }
        SetVolumeMountStatus(VolumeMountStatus::Mounted);
        POS_TRACE_INFO(EID(MOUNT_VOL_DEBUG_MSG),
            "Volume mounted name: {}", GetVolumeName());
    }
    else
    {
        errorCode = EID(MOUNT_VOL_ALREADY_MOUNTED);
        POS_TRACE_WARN(errorCode, "vol_name: {}", GetVolumeName());
    }

    return errorCode;
}

int
VolumeBase::Unmount(void)
{
    int errorCode = EID(SUCCESS);
    if (VolumeMountStatus::Unmounted != GetVolumeMountStatus())
    {
        SetVolumeMountStatus(VolumeMountStatus::Unmounted);
        POS_TRACE_INFO(EID(UNMOUNT_VOL_DEBUG_MSG),
            "Volume unmounted name: {}", GetVolumeName());
    }
    else
    {
        errorCode = EID(UNMOUNT_VOL_ALREADY_UNMOUNTED);
        POS_TRACE_WARN(errorCode, "vol_name: {}", GetVolumeName());
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

uint64_t
VolumeBase::UsedSize(void)
{
    IVSAMap* iVSAMap = MapperServiceSingleton::Instance()->GetIVSAMap(GetArrayName());
    return iVSAMap->GetNumUsedBlks(ID) * BLOCK_SIZE;
}

uint64_t
VolumeBase::RemainingSize(void)
{
    uint64_t totalSize = GetTotalSize();
    uint64_t usedSize = UsedSize();
    if (usedSize > totalSize)
    {
        POS_TRACE_ERROR(EID(CREATE_VOL_SIZE_NOT_ALIGNED),
            "[NUSE ERROR] Volume:{}'s UsedSize:{} exceeds TotalSize:{}", ID, usedSize, totalSize);
        assert(false);
        return -1;
    }
    else
    {
        POS_TRACE_INFO(EID(CREATE_VOL_SIZE_NOT_ALIGNED),
            "[NUSE Volume:{}] UsedSize:{}, TotalSize:{}", ID, usedSize, totalSize);
        return (totalSize - usedSize);
    }
}

} // namespace pos
