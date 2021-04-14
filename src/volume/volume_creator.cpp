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

#include <string>
#include "src/volume/volume_creator.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume.h"
#include "src/volume/volume_name_policy.h"
#include "src/volume/volume_list.h"

namespace pos
{

VolumeCreator::VolumeCreator(VolumeList& volumeList, std::string arrayName)
: VolumeInterface(volumeList, arrayName)
{
}

VolumeCreator::~VolumeCreator(void)
{
}

int
VolumeCreator::Do(string name, uint64_t size, uint64_t maxIops,
        uint64_t maxBw)
{
    VolumeNamePolicy namePolicy;
    int ret = namePolicy.CheckVolumeName(name);
    if (ret != static_cast<int>(POS_EVENT_ID::SUCCESS))
    {
        return ret;
    }

    if (volumeList.GetID(name) >= 0)
    {
        POS_TRACE_WARN(static_cast<int>(POS_EVENT_ID::VOL_NAME_DUPLICATED),
                "Volume name is duplicated");
        return static_cast<int>(POS_EVENT_ID::VOL_NAME_DUPLICATED);
    }

    ret = _CheckVolumeSize(size);
    if (ret != static_cast<int>(POS_EVENT_ID::SUCCESS))
    {
        return ret;
    }

    VolumeBase* vol = new Volume(name, size);
    if (vol == nullptr)
    {
        POS_TRACE_ERROR(static_cast<int>(POS_EVENT_ID::MEM_ALLOC_FAIL),
                "Fail to allocate memory");
        return static_cast<int>(POS_EVENT_ID::MEM_ALLOC_FAIL);
    }

    ret = _SetVolumeQos(vol, maxIops, maxBw);
    if (ret != static_cast<int>(POS_EVENT_ID::SUCCESS))
    {
        delete vol;
        return ret; // error_create_vol_failed;
    }

    ret = volumeList.Add(vol);
    if (ret != static_cast<int>(POS_EVENT_ID::SUCCESS))
    {
        delete vol;
        return ret; // error_create_vol_failed;
    }

    ret = _SaveVolumes();
    if (ret != static_cast<int>(POS_EVENT_ID::SUCCESS))
    {
        volumeList.Remove(vol->ID);
        return ret;
    }

    POS_TRACE_DEBUG(static_cast<int>(POS_EVENT_ID::SUCCESS),
            "Volume meta saved successfully");

    bool res = eventPublisher->NotifyVolumeCreated(name, vol->ID, size,
            vol->MaxIOPS(), vol->MaxBW(), arrayName);

    if (res == false)
    {
        return static_cast<int>(POS_EVENT_ID::DONE_WITH_ERROR);
    }

    return static_cast<int>(POS_EVENT_ID::SUCCESS);
}

} // namespace pos
