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

#include "device_builder.h"
#include "src/array/device/array_device.h"
#include "src/logger/logger.h"

namespace pos
{

int
DeviceBuilder::Create(const DeviceSet<string>& nameSet,
    vector<ArrayDevice*>& devs, IDevInfo* getDev)
{
    uint32_t index = 0;
    for (string devName : nameSet.data)
    {
        DevName name(devName);
        UblockSharedPtr ublock = getDev->GetDev(name);
        if (ublock == nullptr || ublock->GetType() != DeviceType::SSD)
        {
            int eid = EID(CREATE_ARRAY_SSD_NAME_NOT_FOUND);
            POS_TRACE_WARN(eid, "ssd_name: {}", devName);
            return eid;
        }
        ArrayDevice* dev = new ArrayDevice(ublock, ArrayDeviceState::NORMAL,
            index, ArrayDeviceType::DATA);
        devs.push_back(dev);
        index++;
    }

    for (string devName : nameSet.nvm)
    {
        DevName name(devName);
        UblockSharedPtr ublock = getDev->GetDev(name);
        if (ublock == nullptr || ublock->GetType() != DeviceType::NVRAM)
        {
            int eid = EID(CREATE_ARRAY_NVM_NAME_NOT_FOUND);
            POS_TRACE_WARN(eid, "nvm_name: {}", devName);
            return eid;
        }
        ArrayDevice* dev = new ArrayDevice(ublock, ArrayDeviceState::NORMAL,
            0, ArrayDeviceType::NVM);
        devs.push_back(dev);
    }

    for (string devName : nameSet.spares)
    {
        DevName name(devName);
        UblockSharedPtr ublock = getDev->GetDev(name);
        if (ublock == nullptr || ublock->GetType() != DeviceType::SSD)
        {
            int eid = EID(CREATE_ARRAY_SSD_NAME_NOT_FOUND);
            POS_TRACE_WARN(eid, "ssd_name: {}", devName);
            return eid;
        }
        ArrayDevice* dev = new ArrayDevice(ublock, ArrayDeviceState::NORMAL,
            0, ArrayDeviceType::SPARE);
        devs.push_back(dev);
    }
    return 0;
}

int
DeviceBuilder::Load(const vector<pbr::AdeData*>& adeList,
        vector<ArrayDevice*>& devs, IDevInfo* getDev)
{
    for (pbr::AdeData* adeData : adeList)
    {
        DevUid uid(adeData->devSn);
        ArrayDeviceType type = ArrayDeviceType(adeData->devType);
        ArrayDeviceState state = ArrayDeviceState(adeData->devState);
        UblockSharedPtr uBlock = nullptr;
        if (state != ArrayDeviceState::FAULT)
        {
            uBlock = getDev->GetDev(uid);
        }
        ArrayDevice* dev = new ArrayDevice(uBlock,
            state, adeData->devIndex, type);
        devs.push_back(dev);
    }
    return 0;
}
} // namespace pos
