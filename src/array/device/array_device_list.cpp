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

#include "array_device_list.h"

#include <algorithm>
#include <functional>

#include "src/device/ublock_device.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
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

    for_each(devSet_.nvm.begin(), devSet_.nvm.end(), [&](ArrayDevice* dev) {
        devices.nvm.push_back(dev->uBlock->GetName());
    });

    for_each(devSet_.data.begin(), devSet_.data.end(), [&](ArrayDevice* dev) {
        const string faultyDevName = "Faulty Device";
        if (ArrayDeviceState::FAULT == dev->GetState())
        {
            devices.data.push_back(faultyDevName);
        }
        else
        {
            devices.data.push_back(dev->uBlock->GetName());
        }
    });

    for_each(devSet_.spares.begin(), devSet_.spares.end(), [&](ArrayDevice* dev) {
        devices.spares.push_back(dev->uBlock->GetName());
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
    if (devSet_.nvm.size() != 0)
    {
        int eventId = (int)IBOF_EVENT_ID::ARRAY_DEVICE_ADD_FAIL;
        IBOF_TRACE_WARN(eventId,
            "set nvm device without reset previous nvm");
        return eventId;
    }

    if (nvm == nullptr || nvm->uBlock == nullptr)
    {
        int eventId = (int)IBOF_EVENT_ID::ARRAY_DEVICE_ADD_FAIL;
        IBOF_TRACE_WARN(eventId,
            "failed to add buffer device to the Array");
        return eventId;
    }
    else if (nvm->uBlock != nullptr)
    {
        if (Exists(nvm->uBlock->GetName()) != ArrayDeviceType::NONE)
        {
            int eventId = (int)IBOF_EVENT_ID::ARRAY_DEVICE_ADD_FAIL;
            IBOF_TRACE_WARN(eventId,
                "failed to add the device {} to the Array",
                nvm->uBlock->GetName());
            return eventId;
        }
        nvm->uBlock->SetClass(DeviceClass::ARRAY);
    }
    devSet_.nvm.push_back(nvm);
    IBOF_TRACE_INFO(IBOF_EVENT_ID::ARRAY_DEVICE_ADDED,
        "the device {} added to the Array as NVM",
        nvm->uBlock->GetName());
    return 0;
}

int
ArrayDeviceList::AddData(ArrayDevice* dev)
{
    unique_lock<mutex> lock(*mtx);
    if (dev->uBlock != nullptr)
    {
        if (Exists(dev->uBlock->GetName()) != ArrayDeviceType::NONE)
        {
            int eventId = (int)IBOF_EVENT_ID::ARRAY_DEVICE_ADD_FAIL;
            IBOF_TRACE_WARN(eventId,
                "failed to add the device {} to the Array, Duplicated",
                dev->uBlock->GetName());
            return eventId; // already exists
        }
        dev->uBlock->SetClass(DeviceClass::ARRAY);
    }
    devSet_.data.push_back(dev);
    IBOF_TRACE_INFO(IBOF_EVENT_ID::ARRAY_DEVICE_ADDED,
        "the device added to the Array as DATA");
    return 0;
}

int
ArrayDeviceList::AddSpare(ArrayDevice* dev)
{
    unique_lock<mutex> lock(*mtx);
    ArrayDeviceType existType = Exists(dev->uBlock->GetName());
    if (existType != ArrayDeviceType::NONE)
    {
        int eventId = (int)IBOF_EVENT_ID::ARRAY_DEVICE_ADD_FAIL;
        IBOF_TRACE_WARN(eventId,
            "failed to add the device {} to the Array, Duplicated",
            dev->uBlock->GetName());
        return eventId; // already exists
    }
    dev->uBlock->SetClass(DeviceClass::ARRAY);
    devSet_.spares.push_back(dev);
    IBOF_TRACE_INFO(IBOF_EVENT_ID::ARRAY_DEVICE_ADDED,
        "the device {} added to the Array as SPARE",
        dev->uBlock->GetName());
    return 0;
}

int
ArrayDeviceList::RemoveSpare(ArrayDevice* target)
{
    unique_lock<mutex> lock(*mtx);
    auto it = FindSpare(target);
    if (it == devSet_.spares.end())
    {
        return (int)IBOF_EVENT_ID::ARRAY_DEVICE_REMOVE_FAIL;
    }
    (*it)->uBlock->SetClass(DeviceClass::SYSTEM);
    devSet_.spares.erase(it);
    return 0;
}

void
ArrayDeviceList::Clear()
{
    unique_lock<mutex> lock(*mtx);

    for (ArrayDevice* dev : devSet_.nvm)
    {
        if (dev->uBlock != nullptr)
        {
            dev->uBlock->SetClass(DeviceClass::SYSTEM);
        }
        delete dev;
    }
    devSet_.nvm.clear();

    for (ArrayDevice* dev : devSet_.data)
    {
        if (dev->uBlock != nullptr)
        {
            dev->uBlock->SetClass(DeviceClass::SYSTEM);
        }
        delete dev;
    }
    devSet_.data.clear();

    for (ArrayDevice* dev : devSet_.spares)
    {
        if (dev->uBlock != nullptr)
        {
            dev->uBlock->SetClass(DeviceClass::SYSTEM);
        }
        delete dev;
    }
    devSet_.spares.clear();

    IBOF_TRACE_INFO(IBOF_EVENT_ID::ARRAY_DEVICE_CLEARED,
        "the array devices are cleared");
}

vector<ArrayDevice*>::iterator
ArrayDeviceList::FindNvm(string devName)
{
    auto it = find_if(devSet_.nvm.begin(), devSet_.nvm.end(), [&](ArrayDevice* dev) -> bool {
        return dev->uBlock != nullptr && dev->uBlock->GetName() == devName;
    });

    return it;
}

vector<ArrayDevice*>::iterator
ArrayDeviceList::FindData(string devName)
{
    auto it = find_if(devSet_.data.begin(), devSet_.data.end(), [&](ArrayDevice* dev) -> bool {
        return dev->uBlock != nullptr && dev->uBlock->GetName() == devName;
    });

    return it;
}

vector<ArrayDevice*>::iterator
ArrayDeviceList::FindSpare(string devName)
{
    auto it = find_if(devSet_.spares.begin(), devSet_.spares.end(), [&](ArrayDevice* dev) -> bool {
        return dev->uBlock->GetName() == devName;
    });

    return it;
}

vector<ArrayDevice*>::iterator
ArrayDeviceList::FindSpare(ArrayDevice* target)
{
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
ArrayDeviceList::SpareToData(ArrayDevice* target)
{
    unique_lock<mutex> lock(*mtx);
    if (devSet_.spares.size() == 0)
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::ARRAY_NO_REMAINING_SPARE,
            "No remaining spare device");
        return (int)IBOF_EVENT_ID::ARRAY_NO_REMAINING_SPARE;
    }

    ArrayDevice* spare = devSet_.spares.back();
    IBOF_TRACE_INFO((int)IBOF_EVENT_ID::ARRAY_DEVICE_REPLACED,
        "Faulty device is replaced to the spare {}", spare->uBlock->GetName());
    target->uBlock = spare->uBlock;
    spare->uBlock = nullptr;
    delete spare;
    devSet_.spares.pop_back();

    return 0;
}

} // namespace ibofos
