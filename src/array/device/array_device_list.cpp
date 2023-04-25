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

#include "array_device_list.h"

#include <algorithm>
#include <functional>

#include "src/device/base/ublock_device.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/helper/enumerable/query.h"

namespace pos
{
ArrayDeviceList::ArrayDeviceList()
{
    mtx = new mutex();
}

ArrayDeviceList::~ArrayDeviceList()
{
    delete mtx;
}

vector<ArrayDevice*>&
ArrayDeviceList::GetDevs()
{
    return devices;
}

int
ArrayDeviceList::Import(vector<ArrayDevice*> devs)
{
    for (auto dev : devs)
    {
        POS_TRACE_INFO(EID(IMPORT_ARRAY_DEV_DEBUG), "type:{}, state:{}, name:{}, serail:{}, index:{}",
            dev->GetType(), dev->GetState(), dev->GetName(), dev->GetSerial(), dev->GetDataIndex());
        if (dev->GetUblock() != nullptr)
        {
            if (dev->GetUblock()->GetClass() != DeviceClass::SYSTEM)
            {
                return EID(UNABLE_TO_ADD_DEV_ALREADY_OCCUPIED);
            }
            dev->GetUblock()->SetClass(DeviceClass::ARRAY);
        }
        devices.push_back(dev);
    }
    return 0;
}

int
ArrayDeviceList::AddSsd(ArrayDevice* dev)
{
    unique_lock<mutex> lock(*mtx);
    if (dev->GetUblock() != nullptr)
    {
        if (dev->GetUblock()->GetClass() != DeviceClass::SYSTEM)
        {
            return EID(UNABLE_TO_ADD_DEV_ALREADY_OCCUPIED);
        }
        dev->GetUblock()->SetClass(DeviceClass::ARRAY);
    }
    devices.push_back(dev);
    POS_TRACE_INFO(EID(ARRAY_DEV_ADDED),
        "dev_name:{}", dev->GetName());
    return 0;
}

int
ArrayDeviceList::RemoveSsd(ArrayDevice* target)
{
    unique_lock<mutex> lock(*mtx);
    for (auto it = devices.begin(); it != devices.end(); ++it)
    {
        auto type = (*it)->GetType();
        if ((*it) == target && type != ArrayDeviceType::NONE && type != ArrayDeviceType::NVM)
        {
            if ((*it)->GetUblock() != nullptr)
            {
                (*it)->GetUblock()->SetClass(DeviceClass::SYSTEM);
            }
            POS_TRACE_INFO(EID(ARRAY_DEV_REMOVED),
                "dev_name:{}", (*it)->GetName());
            devices.erase(it);
            delete target;
            target = nullptr;
            return 0;
        }
    }
    return EID(REMOVE_DEV_SSD_NAME_NOT_FOUND);
}

void
ArrayDeviceList::Clear(void)
{
    unique_lock<mutex> lock(*mtx);
    for (auto dev : devices)
    {
        if (dev->GetUblock() != nullptr)
        {
            dev->GetUblock()->SetClass(DeviceClass::SYSTEM);
        }
        delete dev;
    }
    devices.clear();
    POS_TRACE_INFO(EID(ARRAY_DEV_DEBUG), "Array devices are cleared");
}

int
ArrayDeviceList::SpareToData(ArrayDevice* target, ArrayDevice*& swapOut)
{
    unique_lock<mutex> lock(*mtx);
    for (auto it = devices.begin(); it != devices.end(); ++it)
    {
        ArrayDevice* dev = (*it);
        if (dev->GetType() == ArrayDeviceType::SPARE &&
            dev->GetState() == ArrayDeviceState::NORMAL)
        {
            UblockSharedPtr newUblock = dev->GetUblock();
            UblockSharedPtr oldUblock = target->GetUblock();
            dev->SetUblock(oldUblock);
            swapOut = dev;
            devices.erase(it);
            target->SetUblock(newUblock);
            POS_TRACE_INFO(EID(DATA_SSD_REPLACED_TO_SPARE),
                "{} is replaced to the spare {}({}), swapout:{}({})",
                target->PrevUblockInfo(), target->GetName(), target->GetSerial(),
                swapOut->GetName(), swapOut->GetSerial());
            return 0;
        }
    }
    return EID(NO_SPARE_SSD_TO_REPLACE);
}

} // namespace pos
