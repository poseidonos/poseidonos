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

#include <algorithm>
#include <vector>

#include "array_device_api.h"
#include "src/device/base/ublock_device.h"
#include "src/device/device_manager.h"
#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/volume/volume_list.h"
#include "src/helper/enumerable/query.h"

namespace pos
{
ArrayDeviceManager::ArrayDeviceManager(DeviceManager* sysDevMgr, string arrayName)
:
    sysDevMgr_(sysDevMgr),
    arrayName_(arrayName)
{
    devs_ = new ArrayDeviceList();
}

ArrayDeviceManager::~ArrayDeviceManager(void)
{
    delete devs_;
}

int
ArrayDeviceManager::ImportByName(DeviceSet<string> nameSet)
{
    int ret = 0;

    for (string devName : nameSet.nvm)
    {
        DevName name(devName);
        UblockSharedPtr uBlock = sysDevMgr_->GetDev(name);
        if (nullptr == uBlock || uBlock->GetType() != DeviceType::NVRAM)
        {
            int eventId = EID(CREATE_ARRAY_NVM_NAME_NOT_FOUND);
            POS_TRACE_WARN(eventId, "devName: {}", devName);
            return eventId;
        }
        ret = devs_->SetNvm(new ArrayDevice(uBlock, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::NVM));
        if (ret != 0)
        {
            return ret;
        }
    }
    uint32_t dataIndex = 0;
    for (string devName : nameSet.data)
    {
        DevName name(devName);
        UblockSharedPtr uBlock = sysDevMgr_->GetDev(name);
        if (nullptr == uBlock || uBlock->GetType() != DeviceType::SSD)
        {
            int eventId = EID(CREATE_ARRAY_SSD_NAME_NOT_FOUND);
            POS_TRACE_WARN(eventId, "devName: {}", devName);
            return eventId;
        }
        ret = devs_->AddSsd((new ArrayDevice(uBlock, ArrayDeviceState::NORMAL, dataIndex, ArrayDeviceType::DATA)));
        if (ret != 0)
        {
            return ret;
        }
        dataIndex++;
    }
    for (string devName : nameSet.spares)
    {
        DevName name(devName);
        UblockSharedPtr uBlock = sysDevMgr_->GetDev(name);
        if (nullptr == uBlock || uBlock->GetType() != DeviceType::SSD)
        {
            int eventId = EID(CREATE_ARRAY_SSD_NAME_NOT_FOUND);
            POS_TRACE_WARN(eventId, "devName: {}", devName);
            return eventId;
        }
        ret = devs_->AddSsd(new ArrayDevice(uBlock, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE));
        if (ret != 0)
        {
            return ret;
        }
    }
    ret = _CheckConstraints(devs_);
    return ret;
}

int
ArrayDeviceManager::Import(DeviceSet<DeviceMeta> metaSet)
{
    int ret = 0;
    for (DeviceMeta meta : metaSet.nvm)
    {
        DevUid uid(meta.uid);
        UblockSharedPtr uBlock = sysDevMgr_->GetDev(uid);
        if (nullptr == uBlock)
        {
            int eventId = EID(ARRAY_NVM_NOT_FOUND);
            POS_TRACE_WARN(eventId, "devUid: {}", meta.uid);
            return eventId;
        }
        devs_->SetNvm(new ArrayDevice(uBlock, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::NVM));
    }

    uint32_t dataIndex = 0;
    for (DeviceMeta meta : metaSet.data)
    {
        ArrayDevice* dev = nullptr;
        DevUid uid(meta.uid);

        if (ArrayDeviceState::FAULT == meta.state)
        {
            dev = new ArrayDevice(nullptr, ArrayDeviceState::FAULT, dataIndex, ArrayDeviceType::DATA);
        }
        else
        {
            UblockSharedPtr uBlock = sysDevMgr_->GetDev(uid);
            if (uBlock == nullptr)
            {
                meta.state = ArrayDeviceState::FAULT;
            }
            else if (ArrayDeviceState::REBUILD == meta.state)
            {
                POS_TRACE_DEBUG(EID(ARRAY_DEV_DEBUG_MSG),
                    "Rebuilding device found {}", meta.uid);
            }

            dev = new ArrayDevice(uBlock, meta.state, dataIndex, ArrayDeviceType::DATA);
        }
        devs_->AddSsd(dev);
        dataIndex++;
    }

    ret = _CheckActiveSsdsCount(ArrayDeviceApi::ExtractDevicesByType(
        ArrayDeviceType::DATA, devs_->GetDevs()));
    if (0 != ret)
    {
        return ret;
    }

    for (DeviceMeta meta : metaSet.spares)
    {
        DevUid uid(meta.uid);
        UblockSharedPtr uBlock = sysDevMgr_->GetDev(uid);
        if (nullptr != uBlock)
        {
            devs_->AddSsd(new ArrayDevice(uBlock, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE));
        }
    }

    return ret;
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
        int eid = EID(UNABLE_TO_ADD_SSD_ALREADY_OCCUPIED);
        POS_TRACE_WARN(eid, "devName: {}", devName);
        return eid;
    }

    uint64_t baseCapa = _GetBaseCapacity(
        ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::DATA, devs_->GetDevs()));
    if (baseCapa > spare->GetSize())
    {
        int eid = EID(ADD_SPARE_CAPACITY_IS_TOO_SMALL);
        POS_TRACE_WARN(eid, "devName: {}, minRequiredCapacity:{}, spareCapacity:{}", devName, baseCapa, spare->GetSize());
        return eid;
    }

    return devs_->AddSsd(new ArrayDevice(spare, ArrayDeviceState::NORMAL, 0, ArrayDeviceType::SPARE));
}

void
ArrayDeviceManager::Clear(void)
{
    devs_->Clear();
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
    return devs_->RemoveSpare(dev);
}

int
ArrayDeviceManager::ReplaceWithSpare(ArrayDevice* target, ArrayDevice*& swapOut)
{
    return devs_->SpareToData(target, swapOut);
}

vector<ArrayDevice*>
ArrayDeviceManager::GetDevs(void)
{
    return devs_->GetDevs();
}

vector<ArrayDevice*>
ArrayDeviceManager::GetFaulty(void)
{
    auto faultDevs = ArrayDeviceApi::ExtractDevicesByState(ArrayDeviceState::FAULT,
        ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::DATA, devs_->GetDevs()));
    return faultDevs;
}

vector<ArrayDevice*>
ArrayDeviceManager::GetRebuilding(void)
{
    return ArrayDeviceApi::ExtractDevicesByState(ArrayDeviceState::REBUILD,
        ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::DATA, devs_->GetDevs()));
}

vector<ArrayDevice*>
ArrayDeviceManager::GetDataDevices(void)
{
    return ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::DATA, devs_->GetDevs());
}

vector<ArrayDevice*>
ArrayDeviceManager::GetSpareDevices(void)
{
    return ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::SPARE, devs_->GetDevs());
}

vector<ArrayDevice*>
ArrayDeviceManager::GetAvailableSpareDevices(void)
{
    return ArrayDeviceApi::ExtractDevicesByState(ArrayDeviceState::NORMAL,
        ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::SPARE, devs_->GetDevs()));
}

int
ArrayDeviceManager::_CheckConstraints(ArrayDeviceList* devList)
{
    auto devs = devList->GetDevs();
    auto dataDevs = ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::DATA, devs);
    auto spareDevs = ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::SPARE, devs);
    auto nvms = ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::NVM, devs);
    int ret = _CheckActiveSsdsCount(dataDevs);
    if (0 != ret)
    {
        return ret;
    }

    uint64_t baseCapa = _GetBaseCapacity(dataDevs);
    if (baseCapa < ArrayConfig::MINIMUM_SSD_SIZE_BYTE)
    {
        int eventId = EID(CREATE_ARRAY_SSD_CAPACITY_IS_LT_MIN);
        POS_TRACE_ERROR(eventId, "size(byte): {}", baseCapa);
        return eventId;
    }

    if (spareDevs.size() > 0)
    {
        uint64_t minSpareCapa = _GetBaseCapacity(spareDevs);
        if (minSpareCapa < baseCapa)
        {
            int eventId = EID(CREATE_ARRAY_SPARE_CAPACITY_IS_LT_DATA);
            POS_TRACE_ERROR(eventId, "minData:{}, minSpare:{}", baseCapa, minSpareCapa);
            return eventId;
        }
    }
    return ret;
}

int
ArrayDeviceManager::_CheckActiveSsdsCount(const vector<ArrayDevice*>& devs)
{
    const int errorId = EID(CREATE_ARRAY_NO_AVAILABLE_DEVICE);
    if (devs.size() > 0)
    {
        auto&& devList = Enumerable::Where(devs,
            [](auto d) { return d->GetState() == ArrayDeviceState::NORMAL; });
        if (devList.size() > 0)
        {
            return 0;
        }
    }
    POS_TRACE_WARN(errorId, "num of active SSDs: {}", devs.size());
    return errorId;
}

uint64_t
ArrayDeviceManager::_GetBaseCapacity(const vector<ArrayDevice*>& devs)
{
    auto&& devList = Enumerable::Where(devs,
        [](auto d) { return d->GetState() != ArrayDeviceState::FAULT; });

    ArrayDevice* base = Enumerable::Minimum(devList,
        [](auto d) { return d->GetUblock()->GetSize(); });

    if (base == nullptr)
    {
        POS_TRACE_WARN(EID(CREATE_ARRAY_DEBUG_MSG), "Failed to acquire base capaicty, device cnt: {}", devList.size());
        return 0;
    }
    return base->GetUblock()->GetSize();
}

void
ArrayDeviceManager::SetArrayDeviceList(ArrayDeviceList* arrayDeviceList)
{
    this->devs_ = arrayDeviceList;
}
} // namespace pos
