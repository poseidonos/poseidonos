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

#ifndef ARRAY_H_
#define ARRAY_H_

#include <list>
#include <string>

#include "src/array/array_state.h"
#include "src/array/config/array_config.h"
#include "src/array/device/array_device_manager.h"
#include "src/array/meta/array_meta_manager.h"
#include "src/array/partition/partition_manager.h"
#include "src/include/address_type.h"
#include "src/io/general_io/ubio.h"
#include "src/lib/singleton.h"

using namespace std;

namespace ibofos
{
class DeviceManager;
class MbrManager;
class ArrayState;
class UBlockDevice;

class Array
{
    friend class WbtCmdHandler;

public:
    Array(void);
    virtual ~Array(void);
    bool ArrayExist(string arrayName);
    int Load(string arrayName);
    int Create(DeviceSet<string> nameSet, string arrayName, string metaRaidType = "RAID1", string dataRaidType = "RAID5");
    int Mount(void);
    int Unmount(void);
    int Delete(string arrayName);
    int AddSpare(string devName, string arrayName);
    int RemoveSpare(string devName, string arrayName);
    int DetachDevice(UBlockDevice* uBlock);
    DeviceSet<string> GetDevNames(void);
    string
    GetArrayName(void)
    {
        return name_;
    };
    string
    GetMetaRaidType(void)
    {
        return metaRaidtype_;
    };
    string
    GetDataRaidType(void)
    {
        return dataRaidtype_;
    };
    void
    SetArrayName(string arrayName)
    {
        name_ = arrayName;
    };
    void
    SetMetaRaidType(string raidType)
    {
        metaRaidtype_ = raidType;
    };
    void
    SetDataRaidType(string raidType)
    {
        dataRaidtype_ = raidType;
    };

    virtual const PartitionLogicalSize* GetSizeInfo(PartitionType type);
    int Translate(const PartitionType, PhysicalBlkAddr&, const LogicalBlkAddr&);
    int Convert(const PartitionType,
        list<PhysicalWriteEntry>&,
        const LogicalWriteEntry&);
    string GetCurrentStateStr(void);

    bool TryLock(PartitionType type, StripeId stripeId);
    void Unlock(PartitionType type, StripeId stripeId);
    void NotifyIbofosMounted();
    int RebuildRead(UbioSmartPtr ubio);
    int PrepareRebuild(ArrayDevice* target);
    void Rebuild(ArrayDevice* target);
    bool TriggerRebuild(ArrayDevice* target);
    bool IsRecoverableDevice(ArrayDevice* target);
    bool IsRecoverableDevice(ArrayDevice* target, ArrayDeviceState oldState);
    void StopRebuilding();
    uint32_t GetRebuildingProgress();

private:
    int _LoadImpl(string arrayName);
    int _CreatePartitions(void);
    int _Flush(void);
    int _ResumeRebuild(ArrayDevice* target);
    void _RebuildDone(ArrayDevice* target, RebuildState result);
    void _DetachSpare(ArrayDevice* target);
    void _DetachData(ArrayDevice* target);

    string name_ = "";
    string metaRaidtype_ = "";
    string dataRaidtype_ = "";
    ArrayState state_;
    pthread_rwlock_t stateLock;

    ArrayMetaManager metaMgr_;
    ArrayDeviceManager devMgr_;
    PartitionManager ptnMgr_;

    DeviceManager* sysDevMgr;
    static const int LOCK_ACQUIRE_FAILED = -1;
};

using ArraySingleton = Singleton<Array>;

} // namespace ibofos
#endif // ARRAY_H_
