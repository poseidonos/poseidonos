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
        if (ublock == nullptr)
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
        if (ublock == nullptr)
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
        if (ublock == nullptr)
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
DeviceBuilder::Load(const DeviceSet<DeviceMeta>& metaSet,
        vector<ArrayDevice*>& devs, IDevInfo* getDev)
{
    uint32_t index = 0;
    for (DeviceMeta meta : metaSet.data)
    {
        ArrayDevice* dev = nullptr;
        DevUid uid(meta.uid);

        if (ArrayDeviceState::FAULT == meta.state)
        {
            dev = new ArrayDevice(nullptr, ArrayDeviceState::FAULT, index, ArrayDeviceType::DATA);
        }
        else
        {
            UblockSharedPtr uBlock = getDev->GetDev(uid);
            if (uBlock == nullptr)
            {
                meta.state = ArrayDeviceState::FAULT;
            }
            else if (ArrayDeviceState::REBUILD == meta.state)
            {
                POS_TRACE_DEBUG(EID(ARRAY_DEV_DEBUG_MSG),
                    "Rebuilding device found {}", meta.uid);
            }

            dev = new ArrayDevice(uBlock, meta.state, index, ArrayDeviceType::DATA);
        }
        devs.push_back(dev);
        index++;
    }

    for (DeviceMeta meta : metaSet.nvm)
    {
        DevUid uid(meta.uid);
        UblockSharedPtr uBlock = getDev->GetDev(uid);
        if (nullptr == uBlock)
        {
            int eventId = EID(ARRAY_NVM_NOT_FOUND);
            POS_TRACE_WARN(eventId, "devUid: {}", meta.uid);
            return eventId;
        }
        devs.push_back(new ArrayDevice(uBlock, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::NVM));
    }

    for (DeviceMeta meta : metaSet.spares)
    {
        DevUid uid(meta.uid);
        UblockSharedPtr uBlock = getDev->GetDev(uid);
        if (nullptr != uBlock)
        {
            devs.push_back(new ArrayDevice(uBlock, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE));
        }
    }
    return 0;
}
} // namespace pos
