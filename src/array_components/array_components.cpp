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
#include "meta_mount_sequence.h"
#include "array_mount_sequence.h"
#include "src/allocator/allocator.h"
#include "src/mapper/mapper.h"
#include "src/state/state_manager.h"
#include "src/metafs/metafs.h"
#include "src/metafs/include/metafs_service.h"
#include "src/io/general_io/rba_state_manager.h"

namespace pos
{
ArrayComponents::ArrayComponents(string array, IArrayRebuilder* rebuilder, IAbrControl* abr)
: arrayName(array),
  arrayRebuilder(rebuilder),
  iAbr(abr)
{
}

ArrayComponents::~ArrayComponents(void)
{
    delete metafs;
    delete arrayMountSequence;
    delete rbaStateMgr;
    delete mapper;
    delete gc;
    delete volMgr;
    delete journal;
    delete allocator;
    delete array;

    StateManagerSingleton::Instance()->RemoveStateControl(arrayName);
}

int ArrayComponents::Create(DeviceSet<string> nameSet, string dataRaidType)
{
    IStateControl* state = StateManagerSingleton::Instance()->CreateStateControl(arrayName);
    array = new Array(arrayName, arrayRebuilder, iAbr, state);
    int ret = array->Create(nameSet, dataRaidType);
    if (ret != 0)
    {
        delete array;
        array = nullptr;
        StateManagerSingleton::Instance()->RemoveStateControl(arrayName);
        return ret;
    }

    metafs = new MetaFs(array, false);
    volMgr = new VolumeManager(array, state);
    gc = new GarbageCollector(array, state);
    mapper = new Mapper(array, state);
    allocator = new Allocator(array, state);
    journal = new JournalManager(array, state);
    rbaStateMgr = new RBAStateManager(array->GetName());

    _SetMountSequence();
    return 0;
}

int ArrayComponents::Load(void)
{
    IStateControl* state = StateManagerSingleton::Instance()->CreateStateControl(arrayName);
    array = new Array(arrayName, arrayRebuilder, iAbr, state);
    int ret = array->Load();
    if (ret != 0)
    {
        delete array;
        array = nullptr;
        StateManagerSingleton::Instance()->RemoveStateControl(arrayName);
        return ret;
    }

    metafs = new MetaFs(array, true);
    journal = new JournalManager(array, state);
    volMgr = new VolumeManager(array, state);
    gc = new GarbageCollector(array, state);
    mapper = new Mapper(array, state);
    allocator = new Allocator(array, state);
    rbaStateMgr = new RBAStateManager(array->GetName());

    _SetMountSequence();
    return 0;
}

int ArrayComponents::Mount(void)
{
    return arrayMountSequence->Mount();
}

int ArrayComponents::Unmount(void)
{
    return arrayMountSequence->Unmount(volMgr);
}

int ArrayComponents::Delete(void)
{
    return array->Delete();
}

int ArrayComponents::PrepareRebuild(void)
{
    IWBStripeAllocator* iWBStripeAllocator = allocator->GetIWBStripeAllocator();

    gc->Pause();
    int ret = iWBStripeAllocator->PrepareRebuild();
    gc->Resume();
    return ret;
}

void ArrayComponents::RebuildDone(void)
{
    IWBStripeAllocator* iWBStripeAllocator = allocator->GetIWBStripeAllocator();
    iWBStripeAllocator->StopRebuilding();
}

void ArrayComponents::_SetMountSequence(void)
{
    mountSequence.push_back(array);
    mountSequence.push_back(metafs);
    mountSequence.push_back(volMgr);
    mountSequence.push_back(new MetaMountSequence(mapper, allocator, journal));
    mountSequence.push_back(gc);

    IStateControl* state = StateManagerSingleton::Instance()->GetStateControl(arrayName);
    arrayMountSequence = new ArrayMountSequence(mountSequence, iAbr, state, arrayName);
}

} // namespace pos
