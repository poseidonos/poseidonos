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

#include "src/volume/volume_detacher.h"

#include <algorithm>
#include <string>
#include <vector>

#include "src/event_scheduler/event_scheduler.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume.h"
#include "src/volume/volume_list.h"

namespace pos
{
VolumeDetacher::VolumeDetacher(VolumeList& volumeList, std::string arrayName, int arrayID, VolumeEventPublisher* volumeEventPublisher, NvmfTarget* nvmfTarget_)
: VolumeInterface(volumeList, arrayName, arrayID, volumeEventPublisher)
{
    if (nvmfTarget_ == nullptr)
    {
        nvmfTarget = NvmfTargetSingleton::Instance();
    }
    else
    {
        nvmfTarget = nvmfTarget_;
    }
}

VolumeDetacher::~VolumeDetacher(void)
{
}

void
VolumeDetacher::DoAll(void)
{
    int vol_cnt = volumeList.Count();
    if (vol_cnt == 0)
    {
        return;
    }
    vector<int> mountedVols;
    int idx = -1;
    VolumeBase* vol = volumeList.Next(idx);
    while (vol != nullptr)
    {
        if (vol->GetStatus() != VolumeStatus::Unmounted)
        {
            if (nvmfTarget->CheckVolumeAttached(vol->ID, arrayName) == true)
            {
                volumeList.WaitUntilIdleUserIo(vol->ID);
                mountedVols.push_back(vol->ID);
            }
        }
        vol = volumeList.Next(idx);
    }

    _SetVolumeArrayInfo();

    eventPublisher->NotifyVolumeDetached(mountedVols, &volumeArrayInfo);

    for (auto volId : mountedVols)
    {
        vol = volumeList.GetVolume(volId);
        vol->Unmount();
        string subnqn = vol->GetSubnqn();
        nvmfTarget->RemoveSubsystemArrayName(subnqn);
        vol->SetSubnqn("");
    }
}
} // namespace pos
