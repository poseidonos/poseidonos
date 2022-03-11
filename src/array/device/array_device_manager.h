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

#ifndef ARRAY_DEVICE_MANAGER_H_
#define ARRAY_DEVICE_MANAGER_H_

#include <string>
#include <tuple>
#include <vector>

#include "array_device.h"
#include "array_device_list.h"
#include "src/array/meta/device_meta.h"
#include "src/array_models/dto/device_set.h"
#include "src/device/device_identifier.h"
#include "src/array/device/i_array_device_manager.h"
using namespace std;

namespace pos
{
class DeviceManager;
class DeviceMeta;

class ArrayDeviceManager : public IArrayDevMgr
{
public:
    ArrayDeviceManager(DeviceManager* sysDevMgr, string arrayName);
    virtual ~ArrayDeviceManager(void);
    virtual int ImportByName(DeviceSet<string> nameSet);
    virtual int Import(DeviceSet<DeviceMeta> metaSet);
    virtual DeviceSet<ArrayDevice*>& Export(void);
    virtual DeviceSet<string> ExportToName(void);
    virtual DeviceSet<DeviceMeta> ExportToMeta(void);
    virtual void Clear(void);
    virtual int AddSpare(string devName);
    virtual int RemoveSpare(string devName);
    virtual int RemoveSpare(ArrayDevice* dev);
    virtual int ReplaceWithSpare(ArrayDevice* target);

    virtual tuple<ArrayDevice*, ArrayDeviceType> GetDev(UblockSharedPtr uBlock);
    virtual tuple<ArrayDevice*, ArrayDeviceType> GetDev(string devSn);
    virtual ArrayDevice* GetFaulty(void);
    virtual ArrayDevice* GetRebuilding(void);
    virtual vector<ArrayDevice*> GetDataDevices(void);

    // This is UT helper method and doesn't need to be inherited. This isn't for production use.
    void SetArrayDeviceList(ArrayDeviceList* arrayDeviceList);

private:
    int _CheckConstraints(ArrayDeviceList* devs);
    int _CheckSsdsCapacity(const ArrayDeviceSet& devSet);
    int _CheckNvmCapacity(const ArrayDeviceSet& devSet);
    int _CheckActiveSsdsCount(const vector<ArrayDevice*>& devs);
    uint64_t _ComputeMinNvmCapacity(const uint32_t logicalChunkCount);
    uint64_t _GetBaseCapacity(const vector<ArrayDevice*>& devs);

    ArrayDeviceList* devs_ = nullptr;
    DeviceManager* sysDevMgr_;
    string arrayName_;
};

} // namespace pos

#endif // ARRAY_DEVICE_MANAGER_H_
