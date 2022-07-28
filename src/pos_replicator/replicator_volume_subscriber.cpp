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

#include "replicator_volume_subscriber.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/pos_replicator/posreplicator_manager.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume_service.h"

namespace pos
{
ReplicatorVolumeSubscriber::ReplicatorVolumeSubscriber(IArrayInfo* info)
: VolumeEvent("ReplicatorVolumeSubscriber", info->GetName(), info->GetIndex()),
arrayInfo(info)
{
    volumeManager = nullptr;
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "ReplicatorVolumeSubscriber has been constructed");
}

ReplicatorVolumeSubscriber::~ReplicatorVolumeSubscriber(void)
{
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "ReplicatorVolumeSubscriber has been destructed");
}

int
ReplicatorVolumeSubscriber::Init(void)
{
    int ret = PosReplicatorManagerSingleton::Instance()->Register(arrayId, this);
    volumeManager = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayId);
    VolumeEventPublisherSingleton::Instance()->RegisterSubscriber(this, arrayName, arrayId);
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "ReplicatorVolumeSubscriber has been initialized (arrayId = {}, arrayName = {})",
        arrayId, arrayName);
    return ret;
}

void
ReplicatorVolumeSubscriber::Dispose(void)
{
    PosReplicatorManagerSingleton::Instance()->Unregister(arrayId);
    VolumeEventPublisherSingleton::Instance()->RemoveSubscriber(this, arrayName, arrayId);
    POS_TRACE_INFO(EID(HA_DEBUG_MSG), "ReplicatorVolumeSubscriber has been disposed (arrayId = {}, arrayName = {})",
        arrayId, arrayName);
}

void
ReplicatorVolumeSubscriber::Shutdown(void)
{

}

void
ReplicatorVolumeSubscriber::Flush(void)
{

}

int
ReplicatorVolumeSubscriber::VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

int
ReplicatorVolumeSubscriber::VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

int
ReplicatorVolumeSubscriber::VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

int
ReplicatorVolumeSubscriber::VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

int
ReplicatorVolumeSubscriber::VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

int
ReplicatorVolumeSubscriber::VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

int
ReplicatorVolumeSubscriber::VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo)
{
    return (int)POS_EVENT_ID::VOL_EVENT_OK;
}

IVolumeEventManager*
ReplicatorVolumeSubscriber::GetVolumeManager(void)
{
    return volumeManager;    
}

std::string
ReplicatorVolumeSubscriber::GetArrayName(void)
{
    return arrayInfo->GetName();
}


} // namespace pos

