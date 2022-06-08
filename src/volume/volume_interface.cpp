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

#include <string>
#include "src/volume/volume_interface.h"
#include "src/include/memory.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/sys_info/space_info.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_meta_intf.h"

namespace pos
{

VolumeInterface::VolumeInterface(VolumeList& volumeList, std::string arrayName, int arrayID, VolumeEventPublisher* volumeEventPublisher)
: volumeList(volumeList),
  arrayName(arrayName),
  arrayID(arrayID)
{
    if (volumeEventPublisher == nullptr)
    {
        eventPublisher = VolumeEventPublisherSingleton::Instance();
    }
    else
    {
        eventPublisher = volumeEventPublisher;
    }

    volumeEventBase = {0, 0, "", "", ""};
    volumeEventPerf = {0, 0};
    volumeArrayInfo = {0, ""};
}

VolumeInterface::~VolumeInterface(void)
{
}

void
VolumeInterface::_CheckVolumeSize(uint64_t volumeSize)
{
    if (volumeSize % SZ_1MB != 0 || volumeSize == 0)
    {
        POS_TRACE_WARN(EID(CREATE_VOL_SIZE_NOT_ALIGNED),
            "The requested size, {} is not aligned to MB", volumeSize);
        throw EID(CREATE_VOL_SIZE_NOT_ALIGNED);
    }

    if (SpaceInfo::IsEnough(arrayID, volumeSize) == false)
    {
        POS_TRACE_WARN(EID(CREATE_VOL_SIZE_EXCEEDED),
            "The requested volume size is larger than the remaining space");

        throw EID(CREATE_VOL_SIZE_EXCEEDED);
    }
}

void
VolumeInterface::_SetVolumeQos(VolumeBase* volume, uint64_t maxIops,
        uint64_t maxBw, uint64_t minIops, uint64_t minBw)
{
    if (volume == nullptr)
    {
        POS_TRACE_WARN(EID(VOL_NOT_FOUND), "null");
        throw EID(VOL_NOT_FOUND);
    }

    volume->SetMaxIOPS(maxIops);
    volume->SetMaxBW(maxBw);
    volume->SetMinIOPS(minIops);
    volume->SetMinBW(minBw);
}

void
VolumeInterface::_PrintLogVolumeQos(VolumeBase* volume, uint64_t originalMaxIops, uint64_t originalMaxBw,
        uint64_t originalMinIops, uint64_t originalMinBw)
{
    uint64_t maxIops = volume->MaxIOPS();
    uint64_t maxBw = volume->MaxBW();
    uint64_t minIops = volume->MinIOPS();
    uint64_t minBw = volume->MinBW();

    if (maxIops != originalMaxIops)
    {
        POS_TRACE_INFO(EID(SUCCESS),
            "Max iops is set on volume {} ({}->{})", volume->GetName(),
            originalMaxIops, maxIops);
    }

    if (maxBw != originalMaxBw)
    {
        POS_TRACE_INFO(EID(SUCCESS),
            "Max bandwidth is set on volume {} ({}->{})",
            volume->GetName(), originalMaxBw, maxBw);
    }
    if (minIops != originalMinIops)
    {
        POS_TRACE_INFO(EID(SUCCESS),
            "Min iops is set on volume {} ({}->{})", volume->GetName(),
            originalMinIops, minIops);
    }

    if (minBw != originalMinBw)
    {
        POS_TRACE_INFO(EID(SUCCESS),
            "Min bandwidth is set on volume {} ({}->{})",
            volume->GetName(), originalMinBw, minBw);
    }
}

int
VolumeInterface::_SaveVolumes(void)
{
    return VolumeMetaIntf::SaveVolumes(volumeList, arrayName, arrayID);
}

void
VolumeInterface::_SetVolumeEventBase(VolumeBase* volume, std::string subnqn)
{
    volumeEventBase.volId = volume->ID;
    volumeEventBase.volName = volume->GetName();
    volumeEventBase.volSizeByte = volume->TotalSize();
    if (subnqn.empty() == false)
    {
        volumeEventBase.subnqn = subnqn;
    }
    else
    {
        volumeEventBase.subnqn = volume->GetSubnqn();
    }
    volumeEventBase.uuid = volume->GetUuid();
}

void
VolumeInterface::_SetVolumeEventPerf(VolumeBase* volume)
{
    volumeEventPerf.maxbw = volume->MaxBW();
    volumeEventPerf.maxiops = volume->MaxIOPS();
}

void
VolumeInterface::_SetVolumeArrayInfo(void)
{
    volumeArrayInfo.arrayId = arrayID;
    volumeArrayInfo.arrayName = arrayName;
}

} // namespace pos
