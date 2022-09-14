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
    IArrayDevMgr(sysDevMgr),
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
            int eventId = EID(ARRAY_NVM_NOT_FOUND);
            POS_TRACE_WARN(eventId, "devName: {}", devName);
            return eventId;
        }
        ret = devs_->SetNvm(new ArrayDevice(uBlock));
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
            int eventId = EID(ARRAY_SSD_NOT_FOUND);
            POS_TRACE_WARN(eventId, "devName: {}", devName);
            return eventId;
        }
        ret = devs_->AddData((new ArrayDevice(uBlock, ArrayDeviceState::NORMAL, dataIndex)));
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
            int eventId = EID(ARRAY_SSD_NOT_FOUND);
            POS_TRACE_WARN(eventId, "devName: {}", devName);
            return eventId;
        }
        ret = devs_->AddSpare(new ArrayDevice(uBlock));
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
        devs_->SetNvm(new ArrayDevice(uBlock));
    }

    uint32_t dataIndex = 0;
    for (DeviceMeta meta : metaSet.data)
    {
        ArrayDevice* dev = nullptr;
        DevUid uid(meta.uid);

        if (ArrayDeviceState::FAULT == meta.state)
        {
            dev = new ArrayDevice(nullptr, ArrayDeviceState::FAULT, dataIndex);
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

            dev = new ArrayDevice(uBlock, meta.state, dataIndex);
        }
        devs_->AddData(dev);
        dataIndex++;
    }

    ret = _CheckActiveSsdsCount(devs_->GetDevs().data);
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
            devs_->AddSpare(new ArrayDevice(uBlock));
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

    uint64_t baseCapa = _GetBaseCapacity(devs_->GetDevs().data);
    if (baseCapa > spare->GetSize())
    {
        int eid = EID(ADD_SPARE_CAPACITY_IS_TOO_SMALL);
        POS_TRACE_WARN(eid, "devName: {}, minRequiredCapacity:{}, spareCapacity:{}", devName, baseCapa, spare->GetSize());
        return eid;
    }

    return devs_->AddSpare(new ArrayDevice(spare));
}

void
ArrayDeviceManager::Clear(void)
{
    devs_->Clear();
}

DeviceSet<DeviceMeta>
ArrayDeviceManager::ExportToMeta(void)
{
    DeviceSet<ArrayDevice*> _d = devs_->GetDevs();
    DeviceSet<DeviceMeta> metaSet;
    if (_d.nvm.size() > 0)
    {
        string sn = _d.nvm.at(0)->GetUblock()->GetSN();
        DeviceMeta nvmMeta(sn, _d.nvm.at(0)->GetState());
        metaSet.nvm.push_back(nvmMeta);
    }

    for (ArrayDevice* dev : _d.data)
    {
        DeviceMeta deviceMeta;
        deviceMeta.state = dev->GetState();
        if (ArrayDeviceState::FAULT == deviceMeta.state)
        {
            deviceMeta.uid = "";
        }
        else
        {
            if (dev->GetUblock() != nullptr)
            {
                deviceMeta.uid = dev->GetUblock()->GetSN();
            }
            else
            {
                POS_TRACE_WARN(EID(UPDATE_ABR_DEBUG_MSG),
                    "Array device on array {} is not fault state and its state is {}. but there is no ublock.",
                    arrayName_, deviceMeta.state);
                deviceMeta.uid = "";
                deviceMeta.state = ArrayDeviceState::FAULT;
            }
        }
        metaSet.data.push_back(deviceMeta);
    }
    for (ArrayDevice* dev : _d.spares)
    {
        DeviceMeta deviceMeta(dev->GetUblock()->GetSN(), dev->GetState());
        metaSet.spares.push_back(deviceMeta);
    }

    return metaSet;
}

DeviceSet<string>
ArrayDeviceManager::ExportToName(void)
{
    if (devs_ == nullptr)
    {
        return DeviceSet<string>();
    }

    return devs_->ExportNames();
}

DeviceSet<ArrayDevice*>&
ArrayDeviceManager::Export(void)
{
    return devs_->GetDevs();
}

int
ArrayDeviceManager::RemoveSpare(string devName)
{
    ArrayDeviceType devType;
    ArrayDevice* dev = nullptr;
    DevName name(devName);
    UblockSharedPtr uBlock = sysDevMgr_->GetDev(name);
    tie(dev, devType) = this->GetDev(uBlock);
    if (dev == nullptr)
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

vector<IArrayDevice*>
ArrayDeviceManager::GetFaulty(void)
{
    vector<IArrayDevice*> devs;
    for (ArrayDevice* dev : devs_->GetDevs().data)
    {
        if (ArrayDeviceState::FAULT == dev->GetState())
        {
            devs.push_back(dev);
        }
    }
    return devs;
}

vector<IArrayDevice*>
ArrayDeviceManager::GetRebuilding(void)
{
    vector<IArrayDevice*> devs;
    for (ArrayDevice* dev : devs_->GetDevs().data)
    {
        if (ArrayDeviceState::REBUILD == dev->GetState())
        {
            devs.push_back(dev);
        }
    }
    return devs;
}

vector<ArrayDevice*>
ArrayDeviceManager::GetDataDevices(void)
{
    return devs_->GetDevs().data;
}

vector<ArrayDevice*>
ArrayDeviceManager::GetSpareDevices(void)
{
    return devs_->GetDevs().spares;
}

vector<ArrayDevice*>
ArrayDeviceManager::GetAvailableSpareDevices(void)
{
    return Enumerable::Where(devs_->GetDevs().spares,
            [](auto d) { return d->GetState() == ArrayDeviceState::NORMAL; });
}

int
ArrayDeviceManager::_CheckConstraints(ArrayDeviceList* devs)
{
    ArrayDeviceSet devSet = devs->GetDevs();
    int ret = _CheckActiveSsdsCount(devSet.data);
    if (0 != ret)
    {
        return ret;
    }

    if (devSet.nvm.size() > 0)
    {
        ret = _CheckNvmCapacity(devSet);
        if (0 != ret)
        {
            return ret;
        }
    }

    ret = _CheckSsdsCapacity(devSet);
    return ret;
}

int
ArrayDeviceManager::_CheckNvmCapacity(const DeviceSet<ArrayDevice*>& devSet)
{
    ArrayDevice* nvm = devSet.nvm.at(0);
    uint32_t logicalChunkCount = devSet.data.size() - ArrayConfig::PARITY_COUNT;
    uint64_t minNvmSize = _ComputeMinNvmCapacity(logicalChunkCount);

    if (nvm->GetUblock()->GetSize() < minNvmSize)
    {
        int eventId = EID(UNABLE_TO_SET_NVM_CAPACITY_IS_LT_MIN);
        POS_TRACE_WARN(eventId, "currCapa:{}, minCapa:{}", nvm->GetUblock()->GetSize(), minNvmSize);
        return eventId;
    }

    return 0;
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
ArrayDeviceManager::_ComputeMinNvmCapacity(const uint32_t logicalChunkCount)
{
    uint64_t writeBufferStripeSize = 0;
    writeBufferStripeSize = static_cast<uint64_t>(logicalChunkCount) *
        ArrayConfig::BLOCK_SIZE_BYTE * ArrayConfig::BLOCKS_PER_CHUNK;
    uint64_t writeBufferStripeCount = 0;
    writeBufferStripeCount =
        MAX_VOLUME_COUNT   // in volume_list.h
        + MAX_VOLUME_COUNT // for GC stripe
        + 1;               // for Rebuild
    uint64_t minSize = 0;
    minSize =
        ArrayConfig::META_NVM_SIZE + (writeBufferStripeSize * writeBufferStripeCount);
    return minSize;
}

int
ArrayDeviceManager::_CheckSsdsCapacity(const ArrayDeviceSet& devSet)
{
    uint64_t baseCapa = _GetBaseCapacity(devSet.data);

    if (baseCapa < ArrayConfig::MINIMUM_SSD_SIZE_BYTE)
    {
        int eventId = EID(CREATE_ARRAY_SSD_CAPACITY_IS_LT_MIN);
        POS_TRACE_ERROR(eventId, "size(byte): {}", baseCapa);
        return eventId;
    }

    if (devSet.spares.size() > 0)
    {
        uint64_t minSpareCapa = _GetBaseCapacity(devSet.spares);
        if (minSpareCapa < baseCapa)
        {
            int eventId = EID(CREATE_ARRAY_SPARE_CAPACITY_IS_LT_DATA);
            POS_TRACE_ERROR(eventId, "minData:{}, minSpare:{}", baseCapa, minSpareCapa);
            return eventId;
        }
    }

    return 0;
}

tuple<ArrayDevice*, ArrayDeviceType>
ArrayDeviceManager::GetDev(UblockSharedPtr uBlock)
{
    if (uBlock == nullptr)
    {
        return make_tuple(nullptr, ArrayDeviceType::NONE);
    }
    DeviceSet<ArrayDevice*> _d = devs_->GetDevs();
    for (ArrayDevice* dev : _d.nvm)
    {
        if (dev->GetUblock() == uBlock)
        {
            return make_tuple(dev, ArrayDeviceType::NVM);
        }
    }
    for (ArrayDevice* dev : _d.data)
    {
        if (dev->GetUblock() == uBlock)
        {
            return make_tuple(dev, ArrayDeviceType::DATA);
        }
    }
    for (ArrayDevice* dev : _d.spares)
    {
        if (dev->GetUblock() == uBlock)
        {
            return make_tuple(dev, ArrayDeviceType::SPARE);
        }
    }
    return make_tuple(nullptr, ArrayDeviceType::NONE);
}

tuple<ArrayDevice*, ArrayDeviceType>
ArrayDeviceManager::GetDevBySn(string devSn)
{
    DevUid sn(devSn);
    UblockSharedPtr dev = sysDevMgr_->GetDev(sn);
    return GetDev(dev);
}

tuple<ArrayDevice*, ArrayDeviceType>
ArrayDeviceManager::GetDevByName(string devName)
{
    DevName name(devName);
    UblockSharedPtr dev = sysDevMgr_->GetDev(name);
    return GetDev(dev);
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
