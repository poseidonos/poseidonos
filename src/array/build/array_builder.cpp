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

#include "array_builder.h"
#include "device_builder.h"
#include "partition_builder.h"
#include "src/array/device/array_device_api.h"
#include "src/helper/uuid/uuid_helper.h"
#include "src/helper/time/time_helper.h"
#include "src/array/array_name_policy.h"
#include "src/logger/logger.h"

namespace pos
{
ArrayBuildInfo*
ArrayBuilder::Load(const DeviceSet<DeviceMeta>& devs, string metaRaid, string dataRaid)
{
    ArrayBuildInfo* info = new ArrayBuildInfo();
    info->buildResult = DeviceBuilder::Load(devs, info->devices);
    if (info->buildResult == 0)
    {
        info->buildResult = ArrayDeviceApi::ImportInspection(info->devices);
    }
    if (info->buildResult == 0)
    {
        info->buildResult = PartitionBuilder::Create(info->devices,
            RaidType(metaRaid), RaidType(dataRaid), info->partitions);
    }
    if (info->buildResult != 0)
    {
        POS_TRACE_WARN(info->buildResult, "");
    }
    return info;
}

ArrayBuildInfo*
ArrayBuilder::Create(string name, const DeviceSet<string>& devs,
    string metaRaid, string dataRaid)
{
    ArrayBuildInfo* info = new ArrayBuildInfo();
    info->buildResult = ArrayNamePolicy::CheckArrayName(name);
    if (info->buildResult == 0)
    {
        info->buildResult = DeviceBuilder::Create(devs, info->devices);
        if (info->buildResult == 0)
        {
            info->buildResult = ArrayDeviceApi::ImportInspection(info->devices);
        }
        if (info->buildResult == 0)
        {
            info->buildResult = PartitionBuilder::Create(info->devices,
                RaidType(metaRaid), RaidType(dataRaid), info->partitions);
        }
    }
    if (info->buildResult != 0)
    {
        POS_TRACE_WARN(info->buildResult, "");
    }
    return info;
}
} // namespace pos
