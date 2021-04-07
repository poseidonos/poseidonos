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

#ifndef ARRAY_DEVICE_MANAGER_H_
#define ARRAY_DEVICE_MANAGER_H_

#include <string>
#include <tuple>
#include <vector>

#include "array_device.h"
#include "array_device_list.h"
#include "device_set.h"
#include "src/array/meta/device_meta.h"
#include "src/device/device_identifier.h"

using namespace std;

namespace ibofos
{
class DeviceManager;
class DeviceMeta;

class ArrayDeviceManager
{
public:
    ArrayDeviceManager(DeviceManager* sysDevMgr);
    virtual ~ArrayDeviceManager();
    int Import(DeviceSet<string> nameSet);
    int Import(DeviceSet<DeviceMeta> metaSet,
        uint32_t& missingCnt,
        uint32_t& brokenCnt);
    DeviceSet<ArrayDevice*>& Export(void);
    DeviceSet<string> ExportToName(void);
    DeviceSet<DeviceMeta> ExportToMeta(void);
    void Clear();
    int AddSpare(string devName);
    int RemoveSpare(string devName);
    int RemoveSpare(ArrayDevice* dev);
    int ReplaceWithSpare(ArrayDevice* target);

    tuple<ArrayDevice*, ArrayDeviceType> GetDev(UBlockDevice* uBlock);
    ArrayDevice* GetFaulty();
    ArrayDevice* GetRebuilding();

private:
    int _CheckDevs(const ArrayDeviceSet& devSet);
    int _CheckConstraints(ArrayDeviceList* devs);
    int _CheckDevsCount(ArrayDeviceSet devSet);
    int _CheckFaultTolerance(ArrayDeviceSet devSet);
    int _CheckSsdsCapacity(const ArrayDeviceSet& devSet);
    int _CheckNvmCapacity(const ArrayDeviceSet& devSet);
    uint64_t _ComputeMinNvmCapacity(const uint32_t logicalChunkCount);
    ArrayDevice* _GetBaseline(const vector<ArrayDevice*>& devs);

    ArrayDeviceList* devs_ = nullptr;
    DeviceManager* sysDevMgr_;
};

} // namespace ibofos

#endif // ARRAY_DEVICE_MANAGER_H_
