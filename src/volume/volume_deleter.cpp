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

#include "src/volume/volume_deleter.h"

#include <vector>
#include <string>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume.h"
#include "src/volume/volume_list.h"

namespace pos
{

VolumeDeleter::VolumeDeleter(VolumeList& volumeList, std::string arrayName, int arrayID, VolumeEventPublisher* volumeEventPublisher)
: VolumeInterface(volumeList, arrayName, arrayID, volumeEventPublisher)
{
    vol = nullptr;
}

VolumeDeleter::~VolumeDeleter(void)
{
}

void
VolumeDeleter::_CheckRequestValidity(string name)
{
    vol = volumeList.GetVolume(name);
    if (vol == nullptr)
    {
        POS_TRACE_WARN(static_cast<int>(POS_EVENT_ID::VOL_NOT_EXIST),
                "The requested volume does not exist");
        throw static_cast<int>(POS_EVENT_ID::VOL_NOT_EXIST);
    }

    if (vol->GetStatus() == VolumeStatus::Mounted)
    {
        POS_TRACE_WARN(static_cast<int>(POS_EVENT_ID::DEL_MOUNTED_VOL),
                "Unable to delete mounted volume");
        throw static_cast<int>(POS_EVENT_ID::DEL_MOUNTED_VOL);
    }
}

int
VolumeDeleter::Do(string name)
{
    try
    {
        _CheckRequestValidity(name);

        volumeList.WaitUntilIdle(vol->ID, VolumeStatus::Unmounted);

        _SetVolumeEventBase(vol);
        _SetVolumeArrayInfo();

        bool res = eventPublisher->NotifyVolumeDeleted(&volumeEventBase, &volumeArrayInfo);
        if (res == false)
        {
            throw static_cast<int>(POS_EVENT_ID::DONE_WITH_ERROR);
        }

        vol->SetValid(false); // remove tempo.
        vol->SetSubnqn("");

        int ret = _SaveVolumes();
        if (ret == static_cast<int>(POS_EVENT_ID::SUCCESS))
        {
            volumeList.Remove(volumeEventBase.volId);
        }
        else
        {
            vol->SetValid(true); // undo remove
            vol->SetSubnqn(volumeEventBase.subnqn);
            throw ret;
        }
    }
    catch(int& exceptionEvent)
    {
        return exceptionEvent;
    }

    return static_cast<int>(POS_EVENT_ID::SUCCESS);
}

} // namespace pos
