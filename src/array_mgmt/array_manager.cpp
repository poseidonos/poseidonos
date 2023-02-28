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

#include "array_manager.h"

#include <list>
#include <unistd.h>
#include <memory>

#include "src/device/device_manager.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/helper/enumerable/query.h"

namespace pos
{
ArrayManager::ArrayManager()
: ArrayManager(new ArrayRebuilder(this), DeviceManagerSingleton::Instance(),
    TelemetryClientSingleton::Instance(),
    [](string name, IArrayRebuilder* arrayRebuilder)
    {
        return new ArrayComponents(name, arrayRebuilder);
    },
    new ArrayBuilderAdapter(),
    new pbr::PbrAdapter())
{
    // delegated to other constructor
}

ArrayManager::ArrayManager(ArrayRebuilder* arrayRebuilder,
    DeviceManager* deviceManager, TelemetryClient* telClient,
    function<ArrayComponents*(string, IArrayRebuilder*)> arrayComponentsFactory,
    ArrayBuilderAdapter* arrayBuilderAdapter,
    pbr::PbrAdapter* pbrAdapter)
: arrayRebuilder(arrayRebuilder),
  deviceManager(deviceManager),
  telClient(telClient),
  arrayComponentsFactory(arrayComponentsFactory),
  arrayBuilderAdapter(arrayBuilderAdapter),
  pbrAdapter(pbrAdapter)
{
    pthread_rwlock_init(&arrayListLock, nullptr);
    if (deviceManager != nullptr)
    {
        deviceManager->SetDeviceEventCallback(this);
    }
    else
    {
        POS_TRACE_DEBUG(EID(ARRAY_MGR_DEBUG_MSG), "Ignoring DeviceManager's subscription to DeviceEvent. It's not okay if it's for prod code (not unit test)");
    }
}

ArrayManager::~ArrayManager()
{
    delete pbrAdapter;
    delete arrayBuilderAdapter;
    delete arrayRebuilder;
    for (auto iter : arrayList)
    {
        ArrayComponents* array = _FindArray(iter.first);
        if (array != nullptr)
        {
            POS_TRACE_INFO(EID(ARRAY_MGR_DEBUG_MSG), "Deleting ArrayComponents for {}", iter.first);
            delete array;
        }
        else
        {
            POS_TRACE_WARN(EID(ARRAY_MGR_DEBUG_MSG), "Couldn't find ArrayComponents for {}", iter.first);
        }
    }
}

int
ArrayManager::Load(void)
{
    vector<unique_ptr<pbr::AteData>> arrayTableEntries;
    auto devs = Enumerable::Where(deviceManager->GetDevs(),
        [](auto d) { return d != nullptr && d->GetType() == DeviceType::SSD; });
    int ret = pbrAdapter->Load(devs, arrayTableEntries);
    if (ret == 0)
    {
        for (auto& ate : arrayTableEntries)
        {
            POS_TRACE_INFO(EID(LOAD_ARRAY_DEBUG), "array_name:{}", ate->arrayName);
            ArrayBuildInfo* arrayBuildInfo = arrayBuilderAdapter->Load(ate.get());
            ret = _Import(arrayBuildInfo);
            delete arrayBuildInfo;
            if (ret != 0)
            {
                break;
            }
        }
    }
    else
    {
        POS_TRACE_WARN(ret, "");
    }
    arrayTableEntries.clear();
    return ret;
}

int
ArrayManager::Create(string name, DeviceSet<string> devs, string metaFt, string dataFt)
{
    pthread_rwlock_wrlock(&arrayListLock);
    int ret = 0;
    if (_FindArray(name) != nullptr)
    {
        ret = EID(CREATE_ARRAY_SAME_ARRAY_NAME_EXISTS);
        POS_TRACE_WARN(ret, "array_name: {}", name);
    }
    if (ret == 0)
    {
        if (arrayList.size() >= ArrayMgmtPolicy::MAX_ARRAY_CNT)
        {
            ret = EID(CREATE_ARRAY_EXCEED_MAX_NUM_OF_ARRAYS);
            POS_TRACE_WARN(ret, "num_of_arrays: {}", arrayList.size());
        }
    }
    if (ret == 0)
    {
        ArrayBuildInfo* arrayBuildInfo = arrayBuilderAdapter->Create(name, devs, metaFt, dataFt);
        ret = _Import(arrayBuildInfo);
        delete arrayBuildInfo;
    }
    pthread_rwlock_unlock(&arrayListLock);
    return ret;
}

int
ArrayManager::_Import(ArrayBuildInfo* arrayBuildInfo)
{
    int ret = arrayBuildInfo->buildResult;
    if (ret == 0)
    {
        arrayBuildInfo->arrayIndex = _AllocArrayIndex(arrayBuildInfo->arrayName);
        if (arrayBuildInfo->arrayIndex < ArrayMgmtPolicy::MAX_ARRAY_CNT)
        {
            ArrayComponents* array = arrayComponentsFactory(arrayBuildInfo->arrayName, arrayRebuilder);
            ret = array->Import(arrayBuildInfo);
            if (ret == 0)
            {
                arrayList.emplace(arrayBuildInfo->arrayName, array);
            }
            else
            {
                _ReleaseArrayIndex(arrayBuildInfo->arrayName);
                delete array;
            }
        }
    }
    if (ret != 0)
    {
        arrayBuildInfo->Dispose();
        POS_TRACE_WARN(ret, "array_name:{}", arrayBuildInfo->arrayName);
    }
    else
    {
        POS_TRACE_INFO(EID(POS_TRACE_ARRAY_LOADED),
            "array_name:{}, num_of_arrays:{}",
            arrayBuildInfo->arrayName, arrayList.size());
    }
    return ret;
}

int
ArrayManager::Delete(string name)
{
    pthread_rwlock_wrlock(&arrayListLock);
    ArrayComponents* array = _FindArray(name);
    if (array == nullptr)
    {
        pthread_rwlock_unlock(&arrayListLock);
        int ret = EID(DELETE_ARRAY_ARRAY_NAME_DOES_NOT_EXIST);
        POS_TRACE_WARN(ret, "array_name: {}", name);
        return ret;
    }
    int ret = array->Delete();
    if (ret == EID(SUCCESS))
    {
        delete array;
        arrayList.erase(name);
        _ReleaseArrayIndex(name);
    }
    pthread_rwlock_unlock(&arrayListLock);
    return ret;
}

int
ArrayManager::Mount(string name, bool isWTEnabled)
{
    return _ExecuteOrHandleErrors([&](ArrayComponents* array)
    {
        telClient->RegisterPublisher(array->GetTelemetryPublisher());
        int ret = array->Mount(isWTEnabled);
        if (ret !=  EID(SUCCESS))
        {
            if (array->GetTelemetryPublisher() != nullptr)
            {
                telClient->DeregisterPublisher(array->GetTelemetryPublisher()->GetName());
            }
        }
        return ret;
    }, name, EID(MOUNT_ARRAY_ARRAY_NAME_DOES_NOT_EXIST));
}

int
ArrayManager::Unmount(string name)
{
    return _ExecuteOrHandleErrors([&](ArrayComponents* array)
    {
        int ret = array->Unmount();
        if (ret == EID(SUCCESS))
        {
            if (array->GetTelemetryPublisher() != nullptr)
            {
                telClient->DeregisterPublisher(array->GetTelemetryPublisher()->GetName());
            }
        }
        return ret;
    }, name, EID(UNMOUNT_ARRAY_ARRAY_NAME_DOES_NOT_EXIST));
}


// This function should not be called from reactor and Event worker because of busy waiting.
// CLI context or separate thread (std:: thread) can call this function safely.
// this function should not be called simultaneously with "Unmount / Stop" from other context.
void
ArrayManager::UnmountAllArrayAndStop(void)
{
    int ret = 0;
    do
    {
        pthread_rwlock_rdlock(&arrayListLock);
        for (auto it = arrayList.begin(); it != arrayList.end(); it++)
        {
            ComponentsInfo* info = it->second->GetInfo();
            if (info->arrayInfo->GetState() >= ArrayStateEnum::TRY_MOUNT)
            {
                Unmount(it->first);
            }
        }
        pthread_rwlock_unlock(&arrayListLock);
        // Check if the states of all array is under "Try_mount"
        ret = Stop();
        if (ret == 0)
        {
            break;
        }
        // sleep 300 ms
        usleep(300000);
    }
    while(ret != 0);
}

int
ArrayManager::Stop(void)
{
    pthread_rwlock_rdlock(&arrayListLock);
    int ret = 0;
    for (auto it = arrayList.begin(); it != arrayList.end(); it++)
    {
        ComponentsInfo* info = it->second->GetInfo();
        if (info->arrayInfo->GetState() >= ArrayStateEnum::TRY_MOUNT)
        {
            ret = EID(POS_STOP_FAIULRE_MOUNTED_ARRAY_EXISTS);
            break;
        }
    }
    pthread_rwlock_unlock(&arrayListLock);
    return ret;
}

int
ArrayManager::AddDevice(string name, string dev)
{
    return _ExecuteOrHandleErrors([&dev](ArrayComponents* array)
    {
        return array->GetArray()->AddSpare(dev);
    }, name, EID(ADD_SPARE_ARRAY_NAME_DOES_NOT_EXIST));
}

int
ArrayManager::RemoveDevice(string name, string dev)
{
    return _ExecuteOrHandleErrors([&dev](ArrayComponents* array)
    {
        return array->GetArray()->RemoveSpare(dev);
    }, name, EID(REMOVE_DEV_ARRAY_NAME_DOES_NOT_EXIST));
}

int
ArrayManager::ReplaceDevice(string name, string dev)
{
    return _ExecuteOrHandleErrors([&dev](ArrayComponents* array)
    {
        return array->GetArray()->ReplaceDevice(dev);
    }, name, EID(REPLACE_DEV_ARRAY_NAME_DOES_NOT_EXIST));
}

int
ArrayManager::Rebuild(string name)
{
    return _ExecuteOrHandleErrors([&](ArrayComponents* array)
    {
        return array->GetArray()->Rebuild();
    }, name, EID(REBUILD_ARRAY_NAME_DOES_NOT_EXIST));
}

int
ArrayManager::_ExecuteOrHandleErrors(std::function<int(ArrayComponents*)> f, string name, int eid)
{
    pthread_rwlock_rdlock(&arrayListLock);
    int ret = eid;
    ArrayComponents* array = _FindArray(name);
    if (array != nullptr)
    {
        ret = f(array);
    }
    else
    {
        POS_TRACE_WARN(eid, "array_name:{}", name);
    }
    pthread_rwlock_unlock(&arrayListLock);
    return ret;
}

int
ArrayManager::DeviceDetached(UblockSharedPtr dev)
{
    int ret = 0;
    pthread_rwlock_rdlock(&arrayListLock);
    string devSn = dev->GetSN();
    for (auto it = arrayList.begin(); it != arrayList.end(); it++)
    {
        Array* array = it->second->GetArray();
        IArrayDevice* arrayDev = array->FindDevice(devSn);
        if (arrayDev != nullptr)
        {
            ret = array->DetachDevice(arrayDev);
            pthread_rwlock_unlock(&arrayListLock);
            return ret;
        }
    }

    POS_TRACE_WARN(EID(ARRAY_EVENT_DEV_DETACHED),
        "SSD is detached, but there is no array to which {} ({}) belongs",
        dev->GetName(), dev->GetSN());
    ret = deviceManager->RemoveDevice(dev);
    pthread_rwlock_unlock(&arrayListLock);
    return ret;
}

void
ArrayManager::DeviceAttached(UblockSharedPtr dev)
{
    // todo : initialize device
}

ComponentsInfo*
ArrayManager::GetInfo(string name)
{
    pthread_rwlock_rdlock(&arrayListLock);
    ComponentsInfo* info = nullptr;
    ArrayComponents* array = _FindArray(name);
    if (array != nullptr)
    {
        info = array->GetInfo();
    }
    else
    {
        POS_TRACE_WARN(EID(ARRAY_MGR_NO_ARRAY_MATCHING_NAME), "ArrayName: {}", name);
    }
    pthread_rwlock_unlock(&arrayListLock);
    return info;
}

ComponentsInfo*
ArrayManager::GetInfo(uint32_t arrayIdx)
{
    pthread_rwlock_rdlock(&arrayListLock);
    ComponentsInfo* info = nullptr;
    for (auto iter = arrayList.begin(); iter != arrayList.end(); iter++)
    {
        if (iter->second->GetArray()->GetIndex() == arrayIdx)
        {
            info = iter->second->GetInfo();
        }
    }
    pthread_rwlock_unlock(&arrayListLock);
    return info;
}

vector<const ComponentsInfo*>
ArrayManager::GetInfo(void)
{
    vector<const ComponentsInfo*> info;
    pthread_rwlock_rdlock(&arrayListLock);
    for (auto it : arrayList)
    {
        info.push_back(it.second->GetInfo());
    }
    pthread_rwlock_unlock(&arrayListLock);
    return info;
}

int
ArrayManager::PrepareRebuild(string name, bool& resume)
{
    POS_TRACE_INFO(EID(REBUILD_JOB_PREPARING), "");
    pthread_rwlock_rdlock(&arrayListLock);
    ArrayComponents* array = _FindArray(name);
    if (array == nullptr)
    {
        pthread_rwlock_unlock(&arrayListLock);
        return EID(REBUILD_JOB_PREPARE_FAIL);
    }

    int ret = array->PrepareRebuild(resume);
    pthread_rwlock_unlock(&arrayListLock);
    return ret;
}

void
ArrayManager::RebuildDone(string name)
{
    POS_TRACE_INFO(EID(REBUILD_JOB_COMPLETED), "array_name:{}", name);
    pthread_rwlock_rdlock(&arrayListLock);
    ArrayComponents* array = _FindArray(name);
    if (array != nullptr)
    {
        array->RebuildDone();
    }
    else
    {
        POS_TRACE_WARN(EID(ARRAY_MGR_NO_ARRAY_MATCHING_REQ_NAME), "Cannot invoke RebuildDone because the array {} isn't found", name);
    }
    pthread_rwlock_unlock(&arrayListLock);
}

int
ArrayManager::ResetPbr(void)
{
    POS_TRACE_INFO(EID(PBR_DEBUG_MSG), "");
    pthread_rwlock_wrlock(&arrayListLock);
    int ret = 0;
    for (auto it = arrayList.begin(); it != arrayList.end();)
    {
        ArrayComponents* array = _FindArray(it->first);
        if (array != nullptr)
        {
            string name = array->GetInfo()->arrayInfo->GetName();
            ret = array->Delete();
            if (ret == 0)
            {
                delete array;
                it = arrayList.erase(it);
                _ReleaseArrayIndex(name);
                continue;
            }
        }
        pthread_rwlock_unlock(&arrayListLock);
        POS_TRACE_WARN(ret, "Unable to delete array during reset pbr " + it->first);
        return ret;
    }
    pthread_rwlock_unlock(&arrayListLock);

    auto devs = Enumerable::Where(deviceManager->GetDevs(),
        [](auto d) { return d != nullptr && d->GetType() == DeviceType::SSD; });
    ret = pbrAdapter->Reset(devs);
    if (ret == 0)
    {
        POS_TRACE_INFO(EID(POS_TRACE_PBR_RESET), "");
    }
    else
    {
        POS_TRACE_WARN(ret, "");
    }
    return ret;
}

ArrayComponents*
ArrayManager::_FindArray(string name)
{
    auto it = arrayList.find(name);
    if (it == arrayList.end())
    {
        return nullptr;
    }

    return it->second;
}

void
ArrayManager::SetArrayComponentMap(const map<string, ArrayComponents*>& arrayCompMap)
{
    // The member variable says it's array"List", but it's actually a map.
    pthread_rwlock_wrlock(&arrayListLock);
    this->arrayList = arrayCompMap;
    pthread_rwlock_unlock(&arrayListLock);
}

const map<string, ArrayComponents*>&
ArrayManager::GetArrayComponentMap(void)
{
    return arrayList;
}

void
ArrayManager::SetTargetAddress(string name, string targetAddress)
{
    pthread_rwlock_rdlock(&arrayListLock);
    ArrayComponents* array = _FindArray(name);
    array->SetTargetAddress(targetAddress);
    pthread_rwlock_unlock(&arrayListLock);
}

int
ArrayManager::GetTargetAddress(string name, string& address)
{
    return _ExecuteOrHandleErrors([&address](ArrayComponents* array)
    {
        address = array->GetArray()->GetTargetAddress();
        return 0;
    }, name, EID(ARRAY_MGR_NO_ARRAY_MATCHING_REQ_NAME));
}

uint32_t
ArrayManager::_AllocArrayIndex(string arrayName)
{
    for (uint32_t i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        if (arrayIndexMap[i] == "")
        {
            arrayIndexMap[i] = arrayName;
            return i;
        }
    }
    return ArrayMgmtPolicy::MAX_ARRAY_CNT;
}

void
ArrayManager::_ReleaseArrayIndex(string arrayName)
{
    for (uint32_t i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        if (arrayIndexMap[i] == arrayName)
        {
            arrayIndexMap[i] = "";
            break;
        }
    }
}

} // namespace pos
