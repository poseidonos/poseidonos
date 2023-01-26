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
#include "src/array/device/array_device_api.h"

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

vector<ArrayDevice*>
ArrayDeviceList::GetDevs(void)
{
    return devs;
}

int
ArrayDeviceList::SetNvm(ArrayDevice* nvm)
{
    if (nvm->GetType() != ArrayDeviceType::NVM)
    {
        int eventId = EID(ARRAY_DEVICE_TYPE_NOT_MATCHED);
        POS_TRACE_WARN(eventId, "type:{}", nvm->GetType());
        return eventId;
    }
    unique_lock<mutex> lock(*mtx);
    auto nvms = ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::NVM, devs);
    if (nvms.size() > 0)
    {
        int eventId = EID(UNABLE_TO_SET_NVM_MORE_THAN_ONE);
        POS_TRACE_WARN(eventId, "");
        return eventId;
    }

    if (nvm == nullptr || nvm->GetUblock() == nullptr)
    {
        int eventId = EID(UNABLE_TO_SET_NVM_NO_OR_NULL);
        POS_TRACE_WARN(eventId, "");
        return eventId;
    }

    auto existing = ArrayDeviceApi::FindDevByName(nvm->GetName(), devs);
    if (existing != nullptr)
    {
        int eventId = EID(UNABLE_TO_SET_NVM_ALREADY_OCCUPIED);
        POS_TRACE_WARN(eventId, "devName: {}", nvm->GetName());
        return eventId;
    }
    nvm->GetUblock()->SetClass(DeviceClass::ARRAY);
    
    devs.push_back(nvm);
    return 0;
}

int
ArrayDeviceList::AddSsd(ArrayDevice* dev)
{
    if (dev->GetType() != ArrayDeviceType::DATA &&
        dev->GetType() != ArrayDeviceType::SPARE)
    {
        int eventId = EID(ARRAY_DEVICE_TYPE_NOT_MATCHED);
        POS_TRACE_WARN(eventId, "type:{}", dev->GetType());
        return eventId;
    }
    unique_lock<mutex> lock(*mtx);
    if (dev->GetUblock() != nullptr)
    {
        auto existing = ArrayDeviceApi::FindDevByName(dev->GetName(), devs);
        if (existing != nullptr)
        {
            int eventId = EID(UNABLE_TO_ADD_SSD_ALREADY_OCCUPIED);
            POS_TRACE_WARN(eventId, "devName: {}", dev->GetName());
            return eventId;
        }
        dev->GetUblock()->SetClass(DeviceClass::ARRAY);
    }
    devs.push_back(dev);
    return 0;
}

int
ArrayDeviceList::RemoveSpare(ArrayDevice* target)
{
    unique_lock<mutex> lock(*mtx);
    auto it = _FindDev(target);
    if (it != devs.end())
    {
        ArrayDevice* dev = *it;
        if (dev->GetType() == ArrayDeviceType::SPARE)
        {
            if (dev->GetUblock() != nullptr)
            {
                dev->GetUblock()->SetClass(DeviceClass::SYSTEM);
            }
            delete dev;
            devs.erase(it);
            return 0;
        }
    }
    return EID(REMOVE_DEV_SSD_NAME_NOT_FOUND);
}

void
ArrayDeviceList::Clear()
{
    unique_lock<mutex> lock(*mtx);

    for (auto dev : devs)
    {
        if (dev->GetUblock() != nullptr)
        {
            dev->GetUblock()->SetClass(DeviceClass::SYSTEM);
        }
        delete dev;
    }
    devs.clear();
}

int
ArrayDeviceList::SpareToData(ArrayDevice* target, ArrayDevice*& swapOut)
{
    unique_lock<mutex> lock(*mtx);
    int noSpareToReplace = EID(NO_SPARE_SSD_TO_REPLACE);
    auto activeSpareDevs = ArrayDeviceApi::ExtractDevicesByState(ArrayDeviceState::NORMAL,
        ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::SPARE, devs));
    if (activeSpareDevs.size() > 0)
    {
        ArrayDevice* spare = activeSpareDevs.front();
        if (spare != nullptr)
        {
            UblockSharedPtr newUblock = spare->GetUblock();
            if (newUblock != nullptr)
            {
                UblockSharedPtr oldUblock = target->GetUblock();
                spare->SetUblock(oldUblock);
                swapOut = spare;
                auto it = _FindDev(spare);
                devs.erase(it);
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

vector<ArrayDevice*>::iterator
ArrayDeviceList::_FindDev(ArrayDevice* dev)
{
    for (auto it = devs.begin(); it != devs.end(); ++it)
    {
        if (dev == *it)
        {
            return it;
        }
    }
    return devs.end();
}
} // namespace pos
