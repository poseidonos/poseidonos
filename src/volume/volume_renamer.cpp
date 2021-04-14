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

#include "src/volume/volume_renamer.h"

#include <string>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/volume/volume_name_policy.h"
#include "src/volume/volume_list.h"

namespace pos
{

VolumeRenamer::VolumeRenamer(VolumeList& volumeList, std::string arrayName)
: VolumeInterface(volumeList, arrayName)
{
}

VolumeRenamer::~VolumeRenamer(void)
{
}

int
VolumeRenamer::Do(string oldname, string newname)
{
    VolumeBase* vol = volumeList.GetVolume(oldname);
    if (vol == nullptr)
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::VOL_NOT_EXIST, "The requested volume does not exist");
        return static_cast<int>(POS_EVENT_ID::VOL_NOT_EXIST);
    }

    if (volumeList.GetID(newname) >= 0)
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::VOL_NAME_DUPLICATED, "Volume name is duplicated");
        return static_cast<int>(POS_EVENT_ID::VOL_NAME_DUPLICATED);
    }

    VolumeNamePolicy namePolicy;
    int ret = namePolicy.CheckVolumeName(newname);
    if (ret != static_cast<int>(POS_EVENT_ID::SUCCESS))
    {
        return ret;
    }

    vol->Rename(newname);

    ret = _SaveVolumes();
    if (ret != static_cast<int>(POS_EVENT_ID::SUCCESS))
    {
        vol->Rename(oldname);
        return ret;
    }

    return static_cast<int>(POS_EVENT_ID::SUCCESS);
}

} // namespace pos
