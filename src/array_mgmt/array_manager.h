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

#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <mutex>

#include "src/include/array_mgmt_policy.h"
#include "src/array_components/array_components.h"
#include "src/array_mgmt/interface/i_array_mgmt.h"
#include "src/device/i_device_event.h"
#include "src/lib/singleton.h"
#include "src/rebuild/array_rebuilder.h"
#include "src/rebuild/interface/i_rebuild_notification.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"
#include "src/cli/command_processor.h"
#include "src/array/build/array_builder_adapter.h"

using namespace std;

namespace pos_cli
{
class StopRebuildingCommand;
}  // namespace pos_cli

namespace pos
{
struct ArrayBootRecord;
class ArrayManager : public IArrayMgmt, public IDeviceEvent, public IRebuildNotification
{
    friend class FlushAllUserDataWbtCommand;
    friend class GcWbtCommand;
    friend class ParityLocationWbtCommand;
    friend class ::pos_cli::StopRebuildingCommand;
    friend class ::CommandProcessor;

public:
    ArrayManager();
    ArrayManager(ArrayRebuilder* arrayRebuilder,
        DeviceManager* deviceManager, TelemetryClient* telClient,
        function<ArrayComponents*(string, IArrayRebuilder*)> arrayComponentsFactory,
        ArrayBuilderAdapter* arrayBuilderAdapter = nullptr);
    virtual ~ArrayManager();
    virtual int Load(void);
    virtual int Create(string name, DeviceSet<string> devs, string metaFt, string dataFt) override;
    virtual int Delete(string name) override;
    virtual int Mount(string name, bool isWTEnabled) override;
    virtual int Unmount(string name) override;
    virtual int Stop(void) override;
    virtual int AddDevice(string name, string dev) override;
    virtual int RemoveDevice(string name, string dev) override;
    virtual int ReplaceDevice(string name, string dev) override;
    virtual int Rebuild(string name) override;
    virtual void SetTargetAddress(string name, string targetAddress) override;
    virtual int GetTargetAddress(string name, string& address /* OUT PARAM */) override;
    virtual ComponentsInfo* GetInfo(string name) override;
    virtual ComponentsInfo* GetInfo(uint32_t arrayIdx) override;
    virtual vector<const ComponentsInfo*> GetInfo(void) override;
    virtual void UnmountAllArrayAndStop(void);

    virtual int DeviceDetached(UblockSharedPtr dev) override;
    virtual void DeviceAttached(UblockSharedPtr dev) override;

    virtual int PrepareRebuild(string name, bool& resume) override;
    virtual void RebuildDone(string name) override;
    virtual int ResetPbr(void);

    // UT helper funcs, not meant for Prod usage
    void SetArrayComponentMap(const map<string, ArrayComponents*>& arrayCompMap);
    const map<string, ArrayComponents*>& GetArrayComponentMap(void);
    ////

private:
    ArrayComponents* _FindArray(string name);
    int _Import(ArrayBuildInfo* arrayBuildInfo);
    int _ExecuteOrHandleErrors(std::function<int(ArrayComponents* array)> f, string name, int eid);
    int _DeleteFaultArray(string arrayName);
    uint32_t _AllocArrayIndex(string arrayName);
    void _ReleaseArrayIndex(string arrayName);
    string arrayIndexMap[ArrayMgmtPolicy::MAX_ARRAY_CNT] {"", };
    map<string, ArrayComponents*> arrayList;
    ArrayRebuilder* arrayRebuilder = nullptr;
    DeviceManager* deviceManager = nullptr;
    TelemetryClient* telClient = nullptr;
    function<ArrayComponents*(string, IArrayRebuilder*)> arrayComponentsFactory = nullptr;
    pthread_rwlock_t arrayListLock;
    ArrayBuilderAdapter* arrayBuilderAdapter = nullptr;
};
// Note that we do not recommend direct access to ArrayManagerSingleton.
using ArrayManagerSingleton = Singleton<ArrayManager>;
inline IArrayMgmt*
ArrayMgr(void)
{
    return ArrayManagerSingleton::Instance();
}

} // namespace pos
