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

#include "src/include/ibof_event_id.h"
#include "src/include/memory.h"
#include "src/logger/logger.h"
#include "src/volume/volume_base.h"

namespace ibofos
{
VolumeBase::VolumeBase(std::string volName, uint64_t volSizeByte)
{
    name = volName;
    status = VolumeStatus::Unmounted;
    totalSize = volSizeByte;
    ID = -1;
    IBOF_TRACE_INFO((int)IBOF_EVENT_ID::VOL_CREATED, "Volume name:{} size:{} created", name, totalSize);
}

VolumeBase::VolumeBase(std::string volName, uint64_t volSizeByte, uint64_t _maxiops, uint64_t _maxbw)
{
    name = volName;
    status = VolumeStatus::Unmounted;
    totalSize = volSizeByte;
    maxiops = _maxiops;
    maxbw = _maxbw;
    ID = -1;
    IBOF_TRACE_INFO((int)IBOF_EVENT_ID::VOL_CREATED, "Volume name:{} size:{} iops:{} bw:{} created",
        name, totalSize, maxiops, maxbw);
}

VolumeBase::~VolumeBase(void)
{
}

int
VolumeBase::Mount()
{
    std::unique_lock<std::mutex> lock(statusMutex);
    if (status != VolumeStatus::Mounted)
    {
        status = VolumeStatus::Mounted;
        IBOF_TRACE_INFO((int)IBOF_EVENT_ID::VOL_MOUNTED, "Volume mounted name: {}", name);
        return (int)IBOF_EVENT_ID::SUCCESS;
    }

    IBOF_TRACE_WARN((int)IBOF_EVENT_ID::VOL_ALD_MOUNTED, "The volume already mounted");
    return (int)IBOF_EVENT_ID::VOL_ALD_MOUNTED;
}

int
VolumeBase::Unmount()
{
    std::unique_lock<std::mutex> lock(statusMutex);
    if (status != VolumeStatus::Unmounted)
    {
        status = VolumeStatus::Unmounted;
        IBOF_TRACE_INFO((int)IBOF_EVENT_ID::VOL_UNMOUNTED, "Volume unmounted name: {}", name);
        return (int)IBOF_EVENT_ID::SUCCESS;
    }
    IBOF_TRACE_WARN((int)IBOF_EVENT_ID::VOL_ALD_UNMOUNTED, "The volume already unmounted");
    return (int)IBOF_EVENT_ID::VOL_ALD_UNMOUNTED;
}

int
VolumeBase::SetMaxIOPS(uint64_t val)
{
    if ((val != 0 && val < MIN_IOPS_LIMIT) || val > MAX_IOPS_LIMIT)
    {
        return (int)IBOF_EVENT_ID::OUT_OF_QOS_RANGE;
    }
    maxiops = val * KIOPS;
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeBase::SetMaxBW(uint64_t val)
{
    if ((val != 0 && val < MIN_BW_LIMIT) || val > MAX_BW_LIMIT)
    {
        return (int)IBOF_EVENT_ID::OUT_OF_QOS_RANGE;
    }
    maxbw = val;
    return (int)IBOF_EVENT_ID::SUCCESS;
}

uint64_t
VolumeBase::TotalSize()
{
    return totalSize;
}

uint64_t
VolumeBase::UsedSize()
{
    return totalSize / 2;
}

uint64_t
VolumeBase::RemainingSize()
{
    return TotalSize() - UsedSize();
}
} // namespace ibofos
