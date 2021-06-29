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

#include "src/volume/volume_qos_updater.h"

#include <string>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume_list.h"

namespace pos
{
VolumeQosUpdater::VolumeQosUpdater(VolumeList& volumeList, std::string arrayName)
: VolumeInterface(volumeList, arrayName)
{
}

VolumeQosUpdater::~VolumeQosUpdater(void)
{
}


int
VolumeQosUpdater::Do(string name, uint64_t maxiops, uint64_t maxbw)
{
    VolumeBase* vol = volumeList.GetVolume(name);

    uint64_t originalMaxIops = vol->MaxIOPS();
    uint64_t originalMaxBw = vol->MaxBW();

    int ret = _SetVolumeQos(vol, maxiops, maxbw);
    if (ret != static_cast<int>(POS_EVENT_ID::SUCCESS))
    {
        return ret;
    }

    bool res = VolumeEventPublisherSingleton::Instance()->NotifyVolumeUpdated(
            vol->GetName(), vol->ID, vol->MaxIOPS(), vol->MaxBW(), arrayName);

    if (res == false)
    {
        return static_cast<int>(POS_EVENT_ID::DONE_WITH_ERROR);
    }

    ret = _SaveVolumes();
    if (ret != (int)POS_EVENT_ID::SUCCESS)
    {
        vol->SetMaxIOPS(originalMaxIops);
        vol->SetMaxBW(originalMaxBw);
        return ret;
    }

    _PrintLogVolumeQos(vol, originalMaxIops, originalMaxBw);

    return static_cast<int>(POS_EVENT_ID::SUCCESS);
}
} // namespace pos
