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

#include "array_device_api.h"
#include "src/include/pos_event_id.h"
#include "src/helper/enumerable/query.h"
#include "src/include/array_config.h"
#include "src/include/smart_ptr_type.h"

using namespace std;

namespace pos
{
ArrayDevice*
ArrayDeviceApi::FindDevByName(string devName, const vector<ArrayDevice*>& devs)
{
    return Enumerable::First(devs,
        [devName](auto d) { return d->GetName() == devName; });
}

ArrayDevice*
ArrayDeviceApi::FindDevBySn(string serialNumber, const vector<ArrayDevice*>& devs)
{
    return Enumerable::First(devs,
        [serialNumber](auto d) { return d->GetSerial() == serialNumber; });
}

vector<IArrayDevice*>
ArrayDeviceApi::ConvertToInterface(const vector<ArrayDevice*>& devs)
{
    vector<IArrayDevice*> converted;
    for (auto dev : devs)
    {
        converted.push_back(dev);
    }
    return converted;
}

vector<UblockSharedPtr>
ArrayDeviceApi::ConvertToUblock(const vector<ArrayDevice*>& devs)
{
    vector<UblockSharedPtr> converted;
    for (auto dev : devs)
    {
        converted.push_back(dev->GetUblock());
    }
    return converted;
}

vector<ArrayDevice*>
ArrayDeviceApi::ExtractDevicesByState(ArrayDeviceState state,
    const vector<ArrayDevice*>& devs)
{
    return Enumerable::Where(devs,
        [state](auto d) { return d->GetState() == state; });
}

vector<ArrayDevice*>
ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType type,
    const vector<ArrayDevice*>& devs)
{
    return Enumerable::Where(devs,
        [type](auto d) { return d->GetType() == type; });
}

vector<ArrayDevice*>
ArrayDeviceApi::ExtractDevicesByTypeAndState(ArrayDeviceType type, ArrayDeviceState state,
    const vector<ArrayDevice*>& devs)
{
    return Enumerable::Where(devs,
        [type, state](auto d) { return d->GetType() == type && d->GetState() == state; });
}

uint64_t
ArrayDeviceApi::GetMinimumCapacity(const vector<ArrayDevice*>& devs)
{
    auto activeDevs = ExtractDevicesByState(ArrayDeviceState::NORMAL, devs);
    ArrayDevice* base = Enumerable::Minimum(activeDevs,
        [](auto d) { return d->GetSize(); });
    if (base != nullptr)
    {
        return base->GetSize();
    }
    return 0;
}

int
ArrayDeviceApi::ImportInspection(const vector<ArrayDevice*>& devs)
{
    auto nvms = ExtractDevicesByType(ArrayDeviceType::NVM, devs);
    if (nvms.size() == 0 || nvms.front()->GetUblock() == nullptr)
    {
        return EID(IMPORT_DEVICE_NVM_DOES_NOT_EXIST);
    }

    auto activeDataSsds = ExtractDevicesByState(ArrayDeviceState::NORMAL,
        ExtractDevicesByType(ArrayDeviceType::DATA, devs));
    auto activeSpareSsds = ExtractDevicesByState(ArrayDeviceState::NORMAL,
        ExtractDevicesByType(ArrayDeviceType::SPARE, devs));

    if (activeDataSsds.size() == 0)
    {
        return EID(IMPORT_DEVICE_NO_AVAILABLE_DEVICE);
    }

    uint64_t minDataCapacity = GetMinimumCapacity(activeDataSsds);
    if (minDataCapacity < ArrayConfig::MINIMUM_SSD_SIZE_BYTE)
    {
        return EID(IMPORT_DEVICE_SSD_CAPACITY_IS_LT_MIN);
    }

    if (activeSpareSsds.size() > 0)
    {
        uint64_t minSpareCapacity = GetMinimumCapacity(activeSpareSsds);
        if (minSpareCapacity < minDataCapacity)
        {
            return EID(IMPORT_DEVICE_SPARE_CAPACITY_IS_LT_DATA);
        }
    }
    return 0;
}

} // namespace pos
