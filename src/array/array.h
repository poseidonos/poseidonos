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

#ifndef ARRAY_H_
#define ARRAY_H_

#include <list>
#include <string>
#include <vector>

#include "src/array/partition/partition_services.h"
#include "src/array/device/array_device_manager.h"
#include "src/array/meta/array_meta.h"
#include "src/array/partition/partition_manager.h"
#include "src/array/rebuild/i_array_rebuilder.h"
#include "src/array/service/io_device_checker/i_device_checker.h"
#include "src/array/state/array_state.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/array_models/interface/i_mount_sequence.h"
#include "src/bio/ubio.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/address_type.h"
#include "src/include/array_config.h"
#include "src/array/service/array_service_layer.h"
#include "src/array/device/i_array_device_manager.h"
#include "src/array/array_metrics_publisher.h"

using namespace std;

namespace pos
{
class DeviceManager;
class MbrManager;
class UBlockDevice;
class IAbrControl;
class IStateControl;
class TelemetryPublishser;

class Array : public IArrayInfo, public IMountSequence, public IDeviceChecker
{
    friend class ParityLocationWbtCommand;
    friend class GcWbtCommand;

public:
    Array(string name, IArrayRebuilder* rbdr, IAbrControl* abr, IStateControl* iState);
    Array(string name, IArrayRebuilder* rbdr, IAbrControl* abr, ArrayDeviceManager* devMgr, DeviceManager* sysDevMgr,
        PartitionManager* ptnMgr, ArrayState* arrayState, PartitionServices* svc, EventScheduler* eventScheduler,
        ArrayServiceLayer* arrayservice);
    virtual ~Array(void);
    virtual int Init(void) override;
    virtual void Dispose(void) override;
    virtual void Shutdown(void) override;
    virtual int Load(void);
    virtual int Create(DeviceSet<string> nameSet, string metaFt, string dataFt);
    virtual void Flush(void) override;
    virtual int Delete(void);
    virtual int AddSpare(string devName);
    virtual int RemoveSpare(string devName);
    virtual int ReplaceDevice(string devName);
    virtual int Rebuild(void);
    virtual int DetachDevice(UblockSharedPtr uBlock);
    virtual void MountDone(void);
    virtual int CheckUnmountable(void);
    virtual string Serialize(void);

    const PartitionLogicalSize* GetSizeInfo(PartitionType type) override;
    DeviceSet<string> GetDevNames(void) override;
    string GetName(void) override;
    unsigned int GetIndex(void) override;
    string GetMetaRaidType(void) override;
    string GetDataRaidType(void) override;
    string GetCreateDatetime(void) override;
    string GetUpdateDatetime(void) override;
    id_t GetUniqueId(void) override;
    ArrayStateType GetState(void) override;
    StateContext* GetStateCtx(void) override;
    uint32_t GetRebuildingProgress(void) override;
    IArrayDevMgr* GetArrayManager(void) override;
    bool IsWriteThroughEnabled(void) override;
    int IsRecoverable(IArrayDevice* target, UBlockDevice* uBlock) override;
    IArrayDevice* FindDevice(string devSn) override;
    virtual void InvokeRebuild(vector<IArrayDevice*> targets, bool isResume, bool force = false);
    virtual bool TriggerRebuild(vector<IArrayDevice*> targets);
    virtual bool ResumeRebuild(vector<IArrayDevice*> targets);
    virtual void DoRebuildAsync(vector<IArrayDevice*> dst, vector<IArrayDevice*> src, RebuildTypeEnum rt);
    virtual void SetPreferences(bool isWT);
    virtual void SetTargetAddress(string targetAddress);
    virtual string GetTargetAddress();

private:
    int _LoadImpl(void);
    int _CreatePartitions(RaidTypeEnum metaRaid, RaidTypeEnum dataRaid);
    void _DeletePartitions(void);
    int _Flush(void);
    int _Flush(ArrayMeta& meta);
    void _RebuildDone(vector<IArrayDevice*> dst, vector<IArrayDevice*> src, RebuildResult result);
    void _DetachSpare(ArrayDevice* target);
    void _DetachData(ArrayDevice* target);
    int _RegisterService(void);
    void _UnregisterService(void);
    bool _CanAddSpare(void);

    ArrayState* state = nullptr;
    PartitionServices* svc = nullptr;
    PartitionManager* ptnMgr = nullptr;

    string name_;
    unsigned int index_ = 0;
    pthread_rwlock_t stateLock;
    ArrayDeviceManager* devMgr_;
    DeviceManager* sysDevMgr = nullptr;
    IArrayRebuilder* rebuilder = nullptr;
    static const int LOCK_ACQUIRE_FAILED;
    IAbrControl* abrControl = nullptr;
    EventScheduler* eventScheduler = nullptr;
    ArrayServiceLayer* arrayService = nullptr;
    ArrayMetricsPublisher* publisher = nullptr;
    id_t uniqueId = 0;
    bool isWTEnabled = false;
    string targetAddress = "";
};
} // namespace pos
#endif // ARRAY_H_
