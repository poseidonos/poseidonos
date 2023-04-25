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

#include "array_device_manager.h"
#include "array_device_api.h"

#include <algorithm>
#include <vector>

#include "src/device/base/ublock_device.h"
#include "src/device/device_manager.h"
#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/volume/volume_list.h"

namespace pos
{
ArrayDeviceManager::ArrayDeviceManager(DeviceManager* sysDevMgr, string arrayName)
: sysDevMgr_(sysDevMgr),
  arrayName_(arrayName)
{
    devs_ = new ArrayDeviceList();
}

ArrayDeviceManager::~ArrayDeviceManager(void)
{
    delete devs_;
}

void
ArrayDeviceManager::Clear(void)
{
    devs_->Clear();
}

int
ArrayDeviceManager::Import(vector<ArrayDevice*> devs)
{
    return devs_->Import(devs);
}

int
ArrayDeviceManager::AddSpare(string devName)
{
    DevName name(devName);
    UblockSharedPtr spare = sysDevMgr_->GetDev(name);

    if (spare == nullptr)
    {
        int eid = EID(ADD_SPARE_SSD_NAME_NOT_FOUND);
        POS_TRACE_WARN(eid, "devName: {}", devName);
        return eid;
    }
    if (spare->GetClass() != DeviceClass::SYSTEM)
    {
        int eid = EID(UNABLE_TO_ADD_DEV_ALREADY_OCCUPIED);
        POS_TRACE_WARN(eid, "devName: {}", devName);
        return eid;
    }
    uint64_t baseCapa = ArrayDeviceApi::GetMinimumCapacity(
            ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::DATA, devs_->GetDevs()));
    if (baseCapa > spare->GetSize())
    {
        int eid = EID(ADD_SPARE_CAPACITY_IS_TOO_SMALL);
        POS_TRACE_WARN(eid, "devName: {}, minRequiredCapacity:{}, spareCapacity:{}",
            devName, baseCapa, spare->GetSize());
        return eid;
    }
    return devs_->AddSsd(new ArrayDevice(spare, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE));
}

int
ArrayDeviceManager::RemoveSpare(string devName)
{
    ArrayDevice* dev = ArrayDeviceApi::FindDevByName(devName, devs_->GetDevs());
    if (dev == nullptr || dev->GetType() != ArrayDeviceType::SPARE)
    {
        int eventId = EID(REMOVE_DEV_SSD_NAME_NOT_FOUND);
        POS_TRACE_WARN(eventId, "devName:{}", devName);
        return eventId;
    }
    return devs_->RemoveSsd(dev);
}

int
ArrayDeviceManager::ReplaceWithSpare(ArrayDevice* target, ArrayDevice*& swapOut)
{
    return devs_->SpareToData(target, swapOut);
}

vector<ArrayDevice*>&
ArrayDeviceManager::GetDevs(void)
{
    return devs_->GetDevs();
}

void
ArrayDeviceManager::SetArrayDeviceList(ArrayDeviceList* arrayDeviceList)
{
    this->devs_ = arrayDeviceList;
}
} // namespace pos
