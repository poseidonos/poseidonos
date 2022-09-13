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

#include "array_metrics_publisher.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/sys_info/space_info.h"
#include "src/include/pos_event_id.h"

using namespace std;

namespace pos
{
ArrayMetricsPublisher::ArrayMetricsPublisher(IArrayInfo* arrayInfo, IArrayStateSubscription* subscription)
: VolumeEvent("ArrayMetricsPublisher", arrayInfo->GetName(), arrayInfo->GetIndex()),
  arrayInfo(arrayInfo), subscription(subscription)
{
    spaceInfoPublisher = new SpaceInfoPublisher();
    spaceInfoPublisher->Register(arrayInfo->GetIndex());
    arrayStatePublisher = new ArrayStatePublisher();
    arrayStatePublisher->Register(to_string(arrayInfo->GetUniqueId()));
    VolumeEventPublisherSingleton::Instance()->RegisterSubscriber(this, arrayInfo->GetName(), arrayInfo->GetIndex());
    subscription->Register(this);
}

ArrayMetricsPublisher::~ArrayMetricsPublisher()
{
    subscription->Unregister(this);
    VolumeEventPublisherSingleton::Instance()->RemoveSubscriber(this, arrayInfo->GetName(), arrayInfo->GetIndex());
    delete arrayStatePublisher;
    arrayStatePublisher = nullptr;
    delete spaceInfoPublisher;
    spaceInfoPublisher = nullptr;
}

void ArrayMetricsPublisher::StateChanged(const ArrayStateType& oldState, const ArrayStateType& newState)
{
    // just mounted
    if (newState >= ArrayStateEnum::NORMAL && oldState < ArrayStateEnum::NORMAL)
    {
        uint32_t arrayIndex = arrayInfo->GetIndex();
        spaceInfoPublisher->PublishTotalCapacity(arrayIndex, SpaceInfo::TotalCapacity(arrayIndex));
    }
    arrayStatePublisher->PublishArrayState(newState);
}

int ArrayMetricsPublisher::VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    uint32_t arrayIndex = arrayInfo->GetIndex();
    spaceInfoPublisher->PublishUsedCapacity(arrayIndex, SpaceInfo::Used(arrayIndex));
    return EID(VOL_EVENT_OK);
}

int ArrayMetricsPublisher::VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    uint32_t arrayIndex = arrayInfo->GetIndex();
    spaceInfoPublisher->PublishUsedCapacity(arrayIndex, SpaceInfo::Used(arrayIndex));
    return EID(VOL_EVENT_OK);
}

int ArrayMetricsPublisher::VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    return EID(VOL_EVENT_OK);
}

int ArrayMetricsPublisher::VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo)
{
    return EID(VOL_EVENT_OK);
}

int ArrayMetricsPublisher::VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    uint32_t arrayIndex = arrayInfo->GetIndex();
    spaceInfoPublisher->PublishUsedCapacity(arrayIndex, SpaceInfo::Used(arrayIndex));
    return EID(VOL_EVENT_OK);
}

int ArrayMetricsPublisher::VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo)
{
    return EID(VOL_EVENT_OK);
}

int ArrayMetricsPublisher::VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo)
{
    return EID(VOL_EVENT_OK);
}

} // namespace pos
