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

#include "array_components.h"

#include "src/array_components/array_mount_sequence.h"
#include "src/include/array_mgmt_policy.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/logger/logger.h"
#include "src/metafs/include/metafs_service.h"
#include "src/metafs/metafs.h"
#include "src/state/state_manager.h"
#include "src/metadata/metadata.h"
#include "src/admin/smart_log_mgr.h"
namespace pos
{
ArrayComponents::ArrayComponents(string arrayName, IArrayRebuilder* rebuilder, IAbrControl* abr)
: ArrayComponents(arrayName,
    rebuilder,
    abr,
    nullptr /*stateMgr*/,
    nullptr /*state*/,
    nullptr /*array*/,
    nullptr /*volMgr*/,
    nullptr /*gc*/,
    nullptr /*metadata*/,
    nullptr /*rbaStateMgr*/,
    nullptr /*metaFsFactory*/,
    nullptr /*nvmf*/,
    nullptr /*smartLogMetaIo*/,
    nullptr /*arrayMountSequence*/
    )
{
    // object instantiations for prod
    POS_TRACE_DEBUG(SUCCESS, "Instantiating array components for {}", arrayName);
    this->stateMgr = StateManagerSingleton::Instance();
    this->state = stateMgr->CreateStateControl(arrayName);
    this->array = new Array(arrayName, rebuilder, abr, state);
    this->metaFsFactory = [](Array* arrayPtr, bool isLoaded)
    {
        return new MetaFs(arrayPtr, isLoaded);
    };
    POS_TRACE_DEBUG(SUCCESS, "Instantiated array components for {}", arrayName);
    // meta components need to be instantiated in a specific order:
    // metafs -> volume manager -> mapper/allocator -> journal -> gc.
    // Given that "metafs" object creation depends on runtime information (i.e., array is loaded or not)
    // I'll move those the objection to Create()/Load() instead.
    this->telPublisher = new TelemetryPublisher("Array");
    telPublisher->AddDefaultLabel("array_name", arrayName);
}

ArrayComponents::ArrayComponents(string arrayName,
    IArrayRebuilder* rebuilder,
    IAbrControl* abr,
    StateManager* stateMgr,
    IStateControl* state,
    Array* array,
    VolumeManager* volMgr,
    GarbageCollector* gc,
    Metadata* meta,
    RBAStateManager* rbaStateMgr,
    function<MetaFs* (Array*, bool)> metaFsFactory,
    Nvmf* nvmf,
    SmartLogMetaIo* smartLogMetaIo,
    ArrayMountSequence* arrayMountSequence)
: arrayName(arrayName),
  state(state),
  arrayRebuilder(rebuilder),
  iAbr(abr),
  stateMgr(stateMgr),
  array(array),
  gc(gc),
  meta(meta),
  volMgr(volMgr),
  rbaStateMgr(rbaStateMgr),
  nvmf(nvmf),
  smartLogMetaIo(smartLogMetaIo),
  arrayMountSequence(arrayMountSequence),
  metaFsFactory(metaFsFactory)
{
    // dependency injection for ut
}

ArrayComponents::~ArrayComponents(void)
{
    POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Deleting array component for {}", arrayName);

    if (info != nullptr)
    {
        delete info;
        info = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "ComponentsInfo for {} has been deleted.", arrayName);
    }
    _DestructMetaComponentsInOrder();
    if (arrayMountSequence != nullptr)
    {
        delete arrayMountSequence;
        arrayMountSequence = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "ArrayMountSequence for {} has been deleted.", arrayName);
    }

    if (array != nullptr)
    {
        delete array;
        array = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Array for {} has been deleted.", arrayName);
    }

    if (state != nullptr)
    {
        state = nullptr; // we don't do "delete state" since it should be done by StateManager
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "StateControl for {} has been nullified.", arrayName);
    }

    if (telPublisher != nullptr)
    {
        delete telPublisher;
        telPublisher = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "telemetryPublisher for {} has been nullified.", arrayName);
    }

    stateMgr->RemoveStateControl(arrayName);
    POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "StateManager has removed StateControl for {}", arrayName);
}

ComponentsInfo*
ArrayComponents::GetInfo(void)
{
    return info;
}

int
ArrayComponents::Create(DeviceSet<string> nameSet, string metaFt, string dataFt)
{
    POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Creating array component for {}", arrayName);
    int ret = array->Create(nameSet, metaFt, dataFt);
    if (ret != 0)
    {
        return ret;
    }

    _InstantiateMetaComponentsAndMountSequenceInOrder(false/* array has not been loaded yet*/);
    _SetMountSequence();

    POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Array components for {} have been created.", arrayName);
    return 0;
}

int
ArrayComponents::Load(void)
{
    POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Loading array components for " + arrayName);
    int ret = array->Load();
    if (ret != 0)
    {
        return ret;
    }

    _InstantiateMetaComponentsAndMountSequenceInOrder(true/* array has been loaded already*/);
    _SetMountSequence();

    POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Array components for {} have been loaded.", arrayName);
    return 0;
}

int
ArrayComponents::Mount(bool isWTEnabled)
{
    int ret = arrayMountSequence->Mount();
    if (ret == 0)
    {
        array->MountDone(isWTEnabled);
    }
    return ret;
}

int
ArrayComponents::Unmount(void)
{
    return arrayMountSequence->Unmount();
}

int
ArrayComponents::Delete(void)
{
    return array->Delete();
}

int
ArrayComponents::PrepareRebuild(bool& resume)
{
    StateContext* stateCtx = state->GetState();
    string currSitu = "null";
    if (stateCtx != nullptr)
    {
        currSitu = stateCtx->GetSituation().ToString();
        if (stateCtx->GetSituation() == SituationEnum::REBUILDING)
        {
            int ret = 0;
            gc->Pause();

            if (meta->NeedRebuildAgain())
            {
                resume = true;
            }
            else
            {
                ret = meta->PrepareRebuild();
            }
            gc->Resume();
            return ret;
        }
    }
    POS_TRACE_WARN(EID(REBUILD_INVALIDATED),
        "Rebuild invalidated. Current situation: {} ", currSitu);
    return EID(REBUILD_INVALIDATED);
}

void
ArrayComponents::RebuildDone(void)
{
    meta->StopRebuilding();
}

void
ArrayComponents::_SetMountSequence(void)
{
    mountSequence.push_back(array);
    mountSequence.push_back(nvmf);
    mountSequence.push_back(metafs);
    mountSequence.push_back(volMgr);
    mountSequence.push_back(meta);
    mountSequence.push_back(rbaStateMgr);
    mountSequence.push_back(flowControl);
    mountSequence.push_back(gc);
    mountSequence.push_back(smartLogMetaIo);
    IStateControl* state = stateMgr->GetStateControl(arrayName);
    if (arrayMountSequence != nullptr)
    {
        POS_TRACE_WARN(EID(ARRAY_COMPONENTS_LEAK), "Memory leakage found for ArrayMountSequence for " + arrayName);
    }
    arrayMountSequence = new ArrayMountSequence(mountSequence, state, arrayName, volMgr, arrayRebuilder);
}

void
ArrayComponents::_InstantiateMetaComponentsAndMountSequenceInOrder(bool isArrayLoaded)
{
    if (metafs != nullptr
        || volMgr != nullptr
        || nvmf != nullptr
        || meta != nullptr
        || rbaStateMgr != nullptr
        || flowControl != nullptr
        || gc != nullptr
        || info != nullptr
        || smartLogMetaIo != nullptr)
    {
        POS_TRACE_WARN(EID(ARRAY_COMPONENTS_LEAK), "Meta Components exist already. Possible memory leak (or is it a mock?). Skipping.");
        return;
    }

    // Please note that the order of creation should be like the following:
    metafs = metaFsFactory(array, isArrayLoaded);
    volMgr = new VolumeManager(array, state);
    nvmf = new Nvmf(array->GetName(), array->GetIndex());
    meta = new Metadata(array, state);
    rbaStateMgr = new RBAStateManager(array->GetName(), array->GetIndex());
    flowControl = new FlowControl(array);
    gc = new GarbageCollector(array, state);
    smartLogMetaIo = new SmartLogMetaIo(array->GetIndex(), SmartLogMgrSingleton::Instance());
    info = new ComponentsInfo(array, gc);
}

void
ArrayComponents::_DestructMetaComponentsInOrder(void)
{
    // Please note that the order of creation should be like the following:
    if (gc != nullptr)
    {
        delete gc;
        gc = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "GarbageCollector for {} has been deleted.", arrayName);
    }

    if (flowControl != nullptr)
    {
        delete flowControl;
        flowControl = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "FlowControl for {} has been deleted.", arrayName);
    }

    if (rbaStateMgr != nullptr)
    {
        delete rbaStateMgr;
        rbaStateMgr = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "RbaStateManager for {} has been deleted.", arrayName);
    }

    if (meta != nullptr)
    {
        delete meta;
        meta = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Metadata for {} has been deleted.", arrayName);
    }

    if (nvmf != nullptr)
    {
        delete nvmf;
        nvmf = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Nvmf for {} has been deleted.", arrayName);
    }

    if (volMgr != nullptr)
    {
        delete volMgr;
        volMgr = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "VolumeManager for {} has been deleted.", arrayName);
    }

    if (metafs != nullptr)
    {
        delete metafs;
        metafs = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "MetaFs for {} has been deleted.", arrayName);
    }
}

} // namespace pos
