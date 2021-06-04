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

#include "src/volume/volume_detacher.h"

#include <vector>
#include <algorithm>
#include <string>

#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume.h"
#include "src/volume/volume_list.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/network/nvmf_volume_pos.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{

VolumeDetacher::VolumeDetacher(VolumeList& volumeList, std::string arrayName)
: VolumeInterface(volumeList, arrayName)
{
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
            if (target.CheckVolumeAttached(vol->ID, arrayName) == true)
            {
                volumeList.WaitUntilIdle(vol->ID, VolumeStatus::Mounted);
                mountedVols.push_back(vol->ID);
            }
        }
        vol = volumeList.Next(idx);
    }
    eventPublisher->NotifyVolumeDetached(mountedVols, arrayName);
    bool res = NvmfVolumePos::WaitRequestedVolumesDetached(mountedVols.size());
    if (res == false)
    {
        int ret = (int)POS_EVENT_ID::VOL_DETACH_FAIL;
        POS_TRACE_ERROR(ret,
            "Detach volume failed due to internal error or unmount timeout. Only some of them might be unmounted");
        return;
    }

    for (auto volId : mountedVols)
    {
        vol = volumeList.GetVolume(volId);
        vol->Unmount();
        vol->SetSubnqn("");
    }
}
} // namespace pos
