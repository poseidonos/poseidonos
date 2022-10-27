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

#include <functional>
#include <string>
#include <vector>

#include "components_info.h"
#include "src/array/array.h"
#include "src/gc/flow_control/flow_control.h"
#include "src/gc/garbage_collector.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/io_scheduler/io_dispatcher_submission.h"
#include "src/volume/volume_manager.h"
#include "src/metafs/metafs.h"
#include "src/network/nvmf.h"
#include "src/admin/smart_log_meta_io.h"
#include "src/pos_replicator/replicator_volume_subscriber.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

using namespace std;

namespace pos
{
class IArrayInfo;
class IArrayRebuilder;
class ArrayMountSequence;
class RBAStateManager;
class Metadata;

class ArrayComponents
{
    friend class GcWbtCommand;

public:
    ArrayComponents(string name, IArrayRebuilder* rebuilder, IAbrControl* abr);
    ArrayComponents(string arrayName,
        IArrayRebuilder* rebuilder,
        IAbrControl* abr,
        StateManager* stateMgr,
        IStateControl* state,
        Array* array,
        VolumeManager* volMgr,
        ReplicatorVolumeSubscriber* replicatorVolumeSubscriber,
        GarbageCollector* gc,
        Metadata* meta,
        RBAStateManager* rbaStateMgr,
        function<MetaFs* (Array*, bool)> metaFsFactory,
        Nvmf* nvmf,
        SmartLogMetaIo* smartLogMetaIo,
        StateObserverForIO* stateObserverForIO,
        ArrayMountSequence* mountSequence = nullptr);
    virtual ~ArrayComponents(void);
    virtual ComponentsInfo* GetInfo(void);
    virtual int Create(DeviceSet<string> nameSet, string metaFt, string dataFt);
    virtual int Load(void);
    virtual int Mount(bool isWTEnabled);
    virtual int Unmount(void);
    virtual int Delete(void);
    virtual int PrepareRebuild(bool& resume);
    virtual void RebuildDone(void);
    virtual Array* GetArray(void) { return array; }
    virtual TelemetryPublisher* GetTelemetryPublisher(void) { return telPublisher; }
    virtual void SetTargetAddress(string targetAddress);
    virtual string GetTargetAddress(void);

private:
    void _SetMountSequence(void);
    void _InstantiateMetaComponentsAndMountSequenceInOrder(bool isArrayLoaded);
    void _DestructMetaComponentsInOrder(void);

    string arrayName = "";

    // injected from outside
    IStateControl* state = nullptr;
    IArrayRebuilder* arrayRebuilder = nullptr;
    IAbrControl* iAbr = nullptr;
    StateManager* stateMgr = nullptr;
    Array* array = nullptr;
    FlowControl* flowControl = nullptr;
    GarbageCollector* gc = nullptr;
    Metadata* meta = nullptr;
    VolumeManager* volMgr = nullptr;
    ReplicatorVolumeSubscriber* replicatorVolumeSubscriber = nullptr;
    MetaFs* metafs = nullptr;
    RBAStateManager* rbaStateMgr = nullptr;
    Nvmf* nvmf = nullptr;
    SmartLogMetaIo* smartLogMetaIo = nullptr;
    // instantiated internally
    vector<IMountSequence*> mountSequence;
    ArrayMountSequence* arrayMountSequence = nullptr;

    // MetaFs factory: MetaFs creation is not determined during ArrayComponents construction. Hence, we need a lambda.
    function<MetaFs* (Array*, bool)> metaFsFactory = nullptr;
    StateObserverForIO* stateObserverForIO = nullptr;

    // telemetry
    TelemetryPublisher* telPublisher = nullptr;
    ComponentsInfo* info = nullptr;
};
} // namespace pos
