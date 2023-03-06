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
#include "array_build_policy.h"
#include "src/array/device/array_device_api.h"
#include "src/helper/uuid/uuid_helper.h"
#include "src/helper/time/time_helper.h"
#include "src/logger/logger.h"
#include "src/include/raid_type.h"

namespace pos
{
int
ArrayBuilder::Load(pbr::AteData* ateData, unique_ptr<ArrayBuildInfo>& buildInfo)
{
    POS_TRACE_DEBUG(EID(ARRAY_BUILDER_LOAD_REQUEST), "array_name:{}, uuid:{}",
        ateData->arrayName, ateData->arrayUuid);
    uint32_t buildResult = 0;
    vector<ArrayDevice*> devices;
    vector<Partition*> partitions;

    buildResult = DeviceBuilder::Load(ateData->adeList, devices);
    POS_TRACE_DEBUG(EID(DEVICE_BUILDER_LOAD_RESULT), "array_name:{}, result:{}",
        ateData->arrayName, buildResult);
    if (buildResult == 0)
    {
        buildResult = ArrayDeviceApi::ImportInspection(devices);
        POS_TRACE_DEBUG(EID(IMPORT_DEVICE_INSPECTION_RESULT), "array_name:{}, result:{}",
            ateData->arrayName, buildResult);
    }
    if (buildResult == 0)
    {
        buildResult = PartitionBuilder::Load(ateData->pteList, devices, partitions);
        POS_TRACE_DEBUG(EID(PARTITION_BUILDER_LOAD_RESULT), "array_name:{}, result:{}",
            ateData->arrayName, buildResult);
    }
    if (buildResult != 0)
    {
        POS_TRACE_WARN(buildResult, "");
    }
    else
    {
        buildInfo = make_unique<ArrayBuildInfo>(ArrayBuildType::LOAD, ateData->arrayName,
            ateData->arrayUuid, ateData->createdDateTime, ateData->lastUpdatedDateTime,
            devices, partitions);
    }
    POS_TRACE_INFO(EID(ARRAY_BUILDER_LOAD_RESULT), "array_name:{}, uuid:{}, result:{}",
        ateData->arrayName, ateData->arrayUuid, buildResult);
    return buildResult;
}

int
ArrayBuilder::Create(string name, const DeviceSet<string>& devs,
    string metaRaid, string dataRaid, unique_ptr<ArrayBuildInfo>& buildInfo)
{
    POS_TRACE_DEBUG(EID(ARRAY_BUILDER_CREATE_REQUEST), "array_name:{}", name);
    uint32_t buildResult = 0;
    vector<ArrayDevice*> devices;
    vector<Partition*> partitions;

    buildResult = CheckArrayName(name);
    POS_TRACE_DEBUG(EID(ARRAY_NAME_POLICY_CHECK_RESULT), "array_name:{}, result:{}",
        name, buildResult);
    if (buildResult == 0)
    {
        buildResult = CheckRaidType(metaRaid, dataRaid, devs.spares.size());
        POS_TRACE_DEBUG(EID(ARRAY_BUILDER_RAID_POLICY_CHECK_RESULT),
            "result:{}, array_name:{}, meta_raid_type:{}, data_raid_type:{}",
            buildResult, name, metaRaid, dataRaid);
    }
    if (buildResult == 0)
    {
        buildResult = DeviceBuilder::Create(devs, devices);
        POS_TRACE_DEBUG(EID(DEVICE_BUILDER_CREATE_RESULT), "array_name:{}, result:{}",
            name, buildResult);
        if (buildResult == 0)
        {
            buildResult = ArrayDeviceApi::ImportInspection(devices);
            POS_TRACE_DEBUG(EID(IMPORT_DEVICE_INSPECTION_RESULT),
                "array_name:{}, result:{}", name, buildResult);
        }
        if (buildResult == 0)
        {
            buildResult = PartitionBuilder::Create(devices,
                RaidType(metaRaid), RaidType(dataRaid), partitions);
            POS_TRACE_DEBUG(EID(PARTITION_BUILDER_CREATE_RESULT), "array_name:{}, result:{}",
                name, buildResult);
        }
    }
    if (buildResult != 0)
    {
        POS_TRACE_WARN(buildResult, "");
    }
    else
    {
        uint64_t createdTime = GetCurrentSecondsAsEpoch();
        buildInfo = make_unique<ArrayBuildInfo>(ArrayBuildType::CREATE, name,
            UuidHelper::GenUuid(), createdTime, createdTime,
            devices, partitions);
    }
    POS_TRACE_INFO(EID(ARRAY_BUILDER_CREATE_RESULT), "array_name:{}, result:{}",
        name, buildResult);
    return buildResult;
}
} // namespace pos
