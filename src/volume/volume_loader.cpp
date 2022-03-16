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

#include "src/volume/volume_loader.h"

#include <string>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_list.h"
#include "src/volume/volume_meta_intf.h"

namespace pos
{
VolumeLoader::VolumeLoader(VolumeList& volumeList, std::string arrayName, int arrayID, VolumeEventPublisher* volumeEventPublisher)
: VolumeInterface(volumeList, arrayName, arrayID, volumeEventPublisher)
{
}

VolumeLoader::~VolumeLoader(void)
{
}

int
VolumeLoader::Do(void)
{
    volumeList.Clear();

    int ret = VolumeMetaIntf::LoadVolumes(volumeList, arrayName, arrayID);
    if (ret != EID(SUCCESS))
    {
        return ret;
    }

    if (volumeList.Count() > 0)
    {
        int idx = -1;
        bool res = true;

        while (true)
        {
            VolumeBase* vol = volumeList.Next(idx);
            if (vol == nullptr)
                break;

            _SetVolumeEventBase(vol);
            _SetVolumeEventPerf(vol);
            _SetVolumeArrayInfo();

            bool ret = eventPublisher->NotifyVolumeLoaded(&volumeEventBase, &volumeEventPerf, &volumeArrayInfo);

            if (ret == false)
            {
                res = false;
            }
        }

        if (res == false)
        {
            POS_TRACE_WARN(EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED),
                "array_name: {}", arrayName);
            return EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED);
        }
    }

    return EID(SUCCESS);
}
} // namespace pos
