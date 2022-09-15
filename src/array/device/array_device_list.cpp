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

DeviceSet<string>
ArrayDeviceList::ExportNames()
{
    unique_lock<mutex> lock(*mtx);
    DeviceSet<string> devices;

    for_each(devSet_.nvm.begin(), devSet_.nvm.end(), [&](ArrayDevice* dev)
    {
        devices.nvm.push_back(dev->GetUblock()->GetName());
    });

    for_each(devSet_.data.begin(), devSet_.data.end(), [&](ArrayDevice* dev)
    {
        if (ArrayDeviceState::FAULT == dev->GetState())
        {
            string oldName = "[REMOVED]" + dev->PrevUblockInfo();
            devices.data.push_back(oldName);
        }
        else
        {
            devices.data.push_back(dev->GetUblock()->GetName());
        }
    });

    for_each(devSet_.spares.begin(), devSet_.spares.end(), [&](ArrayDevice* dev)
    {
        devices.spares.push_back(dev->GetUblock()->GetName());
    });

    return devices;
}

DeviceSet<ArrayDevice*>&
ArrayDeviceList::GetDevs()
{
    return devSet_;
}

int
ArrayDeviceList::SetNvm(ArrayDevice* nvm)
{
    unique_lock<mutex> lock(*mtx);
    if (devSet_.nvm.size() > 0)
    {
        int eventId = EID(UNABLE_TO_SET_NVM_MORE_THAN_ONE);
        ArrayDevice* dev = devSet_.nvm.front();
        string oriNvmName = "";
        if (dev->GetUblock() != nullptr)
        {
            oriNvmName = dev->GetUblock()->GetName();
        }
        POS_TRACE_WARN(eventId, "currWriteBufferName: {}", oriNvmName);
        return eventId;
    }

    if (nvm == nullptr || nvm->GetUblock() == nullptr)
    {
        int eventId = EID(UNABLE_TO_SET_NULL_NVM);
        POS_TRACE_WARN(eventId, "");
        return eventId;
    }
    else if (nvm->GetUblock() != nullptr)
    {
        if (Exists(nvm->GetUblock()->GetName()) != ArrayDeviceType::NONE)
        {
            int eventId = EID(UNABLE_TO_SET_NVM_ALREADY_OCCUPIED);
            POS_TRACE_WARN(eventId, "devName: {}", nvm->GetUblock()->GetName());
            return eventId;
        }
        nvm->GetUblock()->SetClass(DeviceClass::ARRAY);
    }
    devSet_.nvm.push_back(nvm);
    POS_TRACE_INFO(EID(ARRAY_DEV_DEBUG_MSG),
        "the device {} added to the Array as NVM",
        nvm->GetUblock()->GetName());
    return 0;
}

int
ArrayDeviceList::AddData(ArrayDevice* dev)
{
    unique_lock<mutex> lock(*mtx);
    if (dev->GetUblock() != nullptr)
    {
        if (Exists(dev->GetUblock()->GetName()) != ArrayDeviceType::NONE)
        {
            int eventId = EID(UNABLE_TO_ADD_SSD_ALREADY_OCCUPIED);
            POS_TRACE_WARN(eventId, "devName: {}", dev->GetUblock()->GetName());
            return eventId;
        }
        dev->GetUblock()->SetClass(DeviceClass::ARRAY);
    }
    devSet_.data.push_back(dev);
    POS_TRACE_INFO(EID(ARRAY_DEV_DEBUG_MSG),
        "the device added to the Array as DATA");
    return 0;
}

int
ArrayDeviceList::AddSpare(ArrayDevice* dev)
{
    unique_lock<mutex> lock(*mtx);
    ArrayDeviceType existType = Exists(dev->GetUblock()->GetName());
    if (existType != ArrayDeviceType::NONE)
    {
        int eventId = EID(UNABLE_TO_ADD_SSD_ALREADY_OCCUPIED);
        POS_TRACE_WARN(eventId, "devName: {}",dev->GetUblock()->GetName());
        return eventId;
    }
    dev->GetUblock()->SetClass(DeviceClass::ARRAY);
    devSet_.spares.push_back(dev);
    POS_TRACE_INFO(EID(ARRAY_DEV_DEBUG_MSG),
        "the device {} added to the Array as SPARE",
        dev->GetUblock()->GetName());
    return 0;
}

int
ArrayDeviceList::RemoveSpare(ArrayDevice* target)
{
    unique_lock<mutex> lock(*mtx);
    auto it = FindSpare(target);
    if (it == devSet_.spares.end())
    {
        int eventId = EID(REMOVE_DEV_SSD_NAME_NOT_FOUND);
        POS_TRACE_WARN(eventId, "");
        return eventId;
    }
    if ((*it)->GetUblock() != nullptr)
    {
        (*it)->GetUblock()->SetClass(DeviceClass::SYSTEM);
    }
    delete target;
    target = nullptr;
    devSet_.spares.erase(it);
    return 0;
}

void
ArrayDeviceList::Clear()
{
    unique_lock<mutex> lock(*mtx);

    for (ArrayDevice* dev : devSet_.nvm)
    {
        if (dev->GetUblock() != nullptr)
        {
            dev->GetUblock()->SetClass(DeviceClass::SYSTEM);
        }
        delete dev;
    }
    devSet_.nvm.clear();

    for (ArrayDevice* dev : devSet_.data)
    {
        if (dev->GetUblock() != nullptr)
        {
            dev->GetUblock()->SetClass(DeviceClass::SYSTEM);
        }
        delete dev;
    }
    devSet_.data.clear();

    for (ArrayDevice* dev : devSet_.spares)
    {
        if (dev->GetUblock() != nullptr)
        {
            dev->GetUblock()->SetClass(DeviceClass::SYSTEM);
        }
        delete dev;
    }
    devSet_.spares.clear();

    POS_TRACE_INFO(EID(ARRAY_DEV_DEBUG_MSG), "Array device list is cleared");
}

vector<ArrayDevice*>::iterator
ArrayDeviceList::FindNvm(string devName)
{
    auto it = find_if(devSet_.nvm.begin(), devSet_.nvm.end(), [&](ArrayDevice* dev) -> bool {
        return dev->GetUblock() != nullptr && dev->GetUblock()->GetName() == devName;
    });

    return it;
}

vector<ArrayDevice*>::iterator
ArrayDeviceList::FindData(string devName)
{
    auto it = find_if(devSet_.data.begin(), devSet_.data.end(), [&](ArrayDevice* dev) -> bool {
        return dev->GetUblock() != nullptr && dev->GetUblock()->GetName() == devName;
    });

    return it;
}

vector<ArrayDevice*>::iterator
ArrayDeviceList::FindSpare(string devName)
{
    auto it = find_if(devSet_.spares.begin(), devSet_.spares.end(), [&](ArrayDevice* dev) -> bool {
        return dev->GetUblock()->GetName() == devName;
    });

    return it;
}

vector<ArrayDevice*>::iterator
ArrayDeviceList::FindSpare(ArrayDevice* target)
{
    if (target == nullptr)
    {
        return devSet_.spares.end();
    }

    auto it = find_if(devSet_.spares.begin(), devSet_.spares.end(), [&](ArrayDevice* dev) -> bool {
        return dev == target;
    });

    return it;
}

ArrayDeviceType
ArrayDeviceList::Exists(string devName)
{
    if (FindNvm(devName) != devSet_.nvm.end())
    {
        return ArrayDeviceType::NVM;
    }

    else if (FindData(devName) != devSet_.data.end())
    {
        return ArrayDeviceType::DATA;
    }

    else if (FindSpare(devName) != devSet_.spares.end())
    {
        return ArrayDeviceType::SPARE;
    }

    return ArrayDeviceType::NONE;
}

int
ArrayDeviceList::SpareToData(ArrayDevice* target, ArrayDevice*& swapOut)
{
    unique_lock<mutex> lock(*mtx);
    int noSpareToReplace = EID(NO_SPARE_SSD_TO_REPLACE);
    if (devSet_.spares.size() > 0)
    {
        ArrayDevice* spare = Enumerable::First(devSet_.spares,
            [](auto d) { return d->GetState() == ArrayDeviceState::NORMAL; });
        if (spare != nullptr)
        {
            UblockSharedPtr newUblock = spare->GetUblock();
            if (newUblock != nullptr)
            {
                UblockSharedPtr oldUblock = target->GetUblock();
                spare->SetUblock(oldUblock);
                swapOut = spare;
                auto it = FindSpare(spare);
                devSet_.spares.erase(it);
                target->SetUblock(newUblock);
                POS_TRACE_INFO(EID(ARRAY_EVENT_SSD_REPLACED),
                    "{} is replaced to the spare {}({}), swapout:{}({})",
                    target->PrevUblockInfo(), target->GetName(), target->GetSerial(),
                    swapOut->GetName(), swapOut->GetSerial());
                return 0;
            }
        }
    }
    POS_TRACE_WARN(noSpareToReplace, "There is no spare device available.");
    return noSpareToReplace;
}

} // namespace pos
