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

#include "array_components.h"

#include "src/array_components/array_mount_sequence.h"
#include "src/array_components/meta_mount_sequence.h"
#include "src/allocator/allocator.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/metafs/include/metafs_service.h"
#include "src/metafs/metafs.h"
#include "src/state/state_manager.h"

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
    nullptr /*mapper*/,
    nullptr /*allocator*/,
    nullptr /*journal*/,
    nullptr /*rbaStateMgr*/,
    nullptr /*metaFsFactory*/
    )
{
    // object instantiations for prod
    POS_TRACE_DEBUG(SUCCESS, "Instantiating array components for {}", arrayName);
    this->stateMgr = StateManagerSingleton::Instance();
    this->state = stateMgr->CreateStateControl(arrayName);
    this->array = new Array(arrayName, rebuilder, abr, state);
    this->rbaStateMgr = new RBAStateManager(array->GetName());
    this->metaFsFactory = [](Array* arrayPtr, bool isLoaded)
    {
        return new MetaFs(arrayPtr, isLoaded);
    };
    POS_TRACE_DEBUG(SUCCESS, "Instantiated array components for {}", arrayName);
    // meta components need to be instantiated in a specific order:
    // metafs -> volume manager -> mapper/allocator -> journal -> gc.
    // Given that "metafs" object creation depends on runtime information (i.e., array is loaded or not)
    // I'll move those the objection to Create()/Load() instead.
}

ArrayComponents::ArrayComponents(string arrayName,
    IArrayRebuilder* rebuilder,
    IAbrControl* abr,
    StateManager* stateMgr,
    IStateControl* state,
    Array* array,
    VolumeManager* volMgr,
    GarbageCollector* gc,
    Mapper* mapper,
    Allocator* allocator,
    JournalManager* journal,
    RBAStateManager* rbaStateMgr,
    function<MetaFs* (Array*, bool)> metaFsFactory)
: arrayName(arrayName),
  state(state),
  arrayRebuilder(rebuilder),
  iAbr(abr),
  stateMgr(stateMgr),
  array(array),
  gc(gc),
  journal(journal),
  volMgr(volMgr),
  mapper(mapper),
  allocator(allocator),
  rbaStateMgr(rbaStateMgr),
  metaFsFactory(metaFsFactory)
{
    // dependency injection for ut
}

ArrayComponents::~ArrayComponents(void)
{
    POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Deleting array component for {}", arrayName);

    _DestructMetaComponentsInOrder();
    if (arrayMountSequence != nullptr)
    {
        delete arrayMountSequence;
        arrayMountSequence = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "ArrayMountSequence for {} has been deleted.", arrayName);
    }

    if (rbaStateMgr != nullptr)
    {
        delete rbaStateMgr;
        rbaStateMgr = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "RbaStateManager for {} has been deleted.", arrayName);
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

    stateMgr->RemoveStateControl(arrayName);
    POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "StateManager has removed StateControl for {}", arrayName);
}

int
ArrayComponents::Create(DeviceSet<string> nameSet, string dataRaidType)
{
    POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Creating array component for {}", arrayName);
    int ret = array->Create(nameSet, dataRaidType);
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
ArrayComponents::Mount(void)
{
    int ret = arrayMountSequence->Mount();
    if (ret == 0)
    {
        array->MountDone();
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
    IWBStripeAllocator* iWBStripeAllocator = allocator->GetIWBStripeAllocator();
    IContextManager* ctxmgr = allocator->GetIContextManager();
    int ret = 0;
    gc->Pause();

    if (ctxmgr->NeedRebuildAgain())
    {
        resume = true;
    }
    else
    {
        ret = iWBStripeAllocator->PrepareRebuild();
    }
    gc->Resume();
    return ret;
}

void
ArrayComponents::RebuildDone(void)
{
    IWBStripeAllocator* iWBStripeAllocator = allocator->GetIWBStripeAllocator();
    iWBStripeAllocator->StopRebuilding();
}

void
ArrayComponents::_SetMountSequence(void)
{
    mountSequence.push_back(array);
    mountSequence.push_back(metafs);
    mountSequence.push_back(volMgr);
    mountSequence.push_back(metaMountSequence);
    mountSequence.push_back(flowControl);
    mountSequence.push_back(gc);

    IStateControl* state = stateMgr->GetStateControl(arrayName);
    if (arrayMountSequence != nullptr)
    {
        POS_TRACE_WARN(EID(ARRAY_COMPONENTS_LEAK), "Memory leakage found for ArrayMountSequence for " + arrayName);
    }
    arrayMountSequence = new ArrayMountSequence(mountSequence, iAbr, state, arrayName, volMgr, arrayRebuilder);
}

void
ArrayComponents::_InstantiateMetaComponentsAndMountSequenceInOrder(bool isArrayLoaded)
{
    if (metafs != nullptr
        || volMgr != nullptr
        || mapper != nullptr
        || allocator != nullptr
        || journal != nullptr
        || flowControl != nullptr
        || gc != nullptr
        || metaMountSequence != nullptr)
    {
        POS_TRACE_WARN(EID(ARRAY_COMPONENTS_LEAK), "Meta Components exist already. Possible memory leak (or is it a mock?). Skipping.");
        return;
    }

    // Please note that the order of creation should be like the following:
    metafs = metaFsFactory(array, isArrayLoaded);
    volMgr = new VolumeManager(array, state);
    mapper = new Mapper(array, state);
    allocator = new Allocator(array, state);
    journal = new JournalManager(array, state);
    flowControl = new FlowControl(array);
    gc = new GarbageCollector(array, state);
    metaMountSequence = new MetaMountSequence(arrayName, mapper, allocator, journal); // remember the ref to be able to delete during ~ArrayComponents()
}

void
ArrayComponents::_DestructMetaComponentsInOrder(void)
{
    // Please note that the order of creation should be like the following:
    if (metaMountSequence != nullptr)
    {
        delete metaMountSequence;
        metaMountSequence = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "MetaMountSequence for {} has been deleted.", arrayName);
    }

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

    if (journal != nullptr)
    {
        delete journal;
        journal = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "JournalManager for {} has been deleted.", arrayName);
    }

    if (allocator != nullptr)
    {
        delete allocator;
        allocator = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Allocator for {} has been deleted.", arrayName);
    }

    if (mapper != nullptr)
    {
        delete mapper;
        mapper = nullptr;
        POS_TRACE_DEBUG(EID(ARRAY_COMPONENTS_DEBUG_MSG), "Mapper for {} has been deleted.", arrayName);
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
