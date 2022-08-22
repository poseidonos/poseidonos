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

#include "src/volume/volume_replicate_property_updater.h"

#include <string>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume_list.h"

namespace pos
{
VolumeReplicatePropertyUpdater::VolumeReplicatePropertyUpdater(VolumeList& volumeList, std::string arrayName, int arrayID, VolumeEventPublisher* volumeEventPublisher)
: VolumeInterface(volumeList, arrayName, arrayID, volumeEventPublisher)
{
}

VolumeReplicatePropertyUpdater::~VolumeReplicatePropertyUpdater(void)
{
}


int
VolumeReplicatePropertyUpdater::Do(string name, VolumeReplicationState state)
{
    VolumeBase* vol = volumeList.GetVolume(name);

    VolumeReplicationState originState = vol->GetReplicationState();

    vol->SetReplicationState(state);

    int ret = _SaveVolumes();
    if (ret != EID(SUCCESS))
    {
        vol->SetReplicationState(originState);
        return ret;
    }

    return EID(SUCCESS);
}

int
VolumeReplicatePropertyUpdater::Do(string name, VolumeReplicationRoleProperty nodeProperty)
{
    VolumeBase* vol = volumeList.GetVolume(name);

    VolumeReplicationRoleProperty originProperty = vol->GetReplicateRoleProperty();

    vol->SetReplicateRoleProperty(nodeProperty);

    _SetVolumeEventBase(vol);
    _SetVolumeEventPerf(vol);
    _SetVolumeArrayInfo();

    bool res = eventPublisher->NotifyVolumeUpdated(&volumeEventBase, &volumeEventPerf, &volumeArrayInfo);

    if (res == false)
    {
        POS_TRACE_WARN(EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED),
            "vol_name:{}, array_name: {}", name, arrayName);
        return EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED);
    }

    int ret = _SaveVolumes();
    if (ret != EID(SUCCESS))
    {
        vol->SetReplicateRoleProperty(originProperty);
        return ret;
    }

    return EID(SUCCESS);
}


} // namespace pos
