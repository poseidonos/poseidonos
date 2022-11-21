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

#include "src/device/device_manager.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/mbr/abr_manager.h"

namespace pos
{
ArrayManager::ArrayManager()
: ArrayManager(new ArrayRebuilder(this), new AbrManager(), DeviceManagerSingleton::Instance(),
    TelemetryClientSingleton::Instance(),
    [](string name, IArrayRebuilder* arrayRebuilder, IAbrControl* iAbrControl)
    {
        return new ArrayComponents(name, arrayRebuilder, iAbrControl);
    })
{
    // delegated to other constructor
}

ArrayManager::ArrayManager(ArrayRebuilder* arrayRebuilder, AbrManager* abrManager,
    DeviceManager* deviceManager, TelemetryClient* telClient,
    function<ArrayComponents*(string, IArrayRebuilder*, IAbrControl*)> arrayComponentsFactory)
: arrayRebuilder(arrayRebuilder),
  abrManager(abrManager),
  deviceManager(deviceManager),
  telClient(telClient),
  arrayComponentsFactory(arrayComponentsFactory)
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
    if (arrayRebuilder != nullptr)
    {
        POS_TRACE_INFO(EID(ARRAY_MGR_DEBUG_MSG), "Deleting ArrayRebuilder");
        delete arrayRebuilder;
    }

    if (abrManager != nullptr)
    {
        POS_TRACE_INFO(EID(ARRAY_MGR_DEBUG_MSG), "Deleting AbrManager");
        delete abrManager;
    }

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
ArrayManager::Create(string name, DeviceSet<string> devs, string metaFt, string dataFt)
{
    pthread_rwlock_wrlock(&arrayListLock);
    if (_FindArray(name) != nullptr)
    {
        int event = EID(CREATE_ARRAY_SAME_ARRAY_NAME_EXISTS);
        POS_TRACE_WARN(event, "name duplicated: {}", name);
        pthread_rwlock_unlock(&arrayListLock);
        return event;
    }

    if (arrayList.size() >= ArrayMgmtPolicy::MAX_ARRAY_CNT)
    {
        int event = EID(CREATE_ARRAY_EXCEED_MAX_NUM_OF_ARRAYS);
        POS_TRACE_WARN(event,
            "Current num of Arrays: {}", arrayList.size());
        pthread_rwlock_unlock(&arrayListLock);
        return event;
    }

    ArrayComponents* array = arrayComponentsFactory(name, arrayRebuilder, abrManager);
    int ret = array->Create(devs, metaFt, dataFt);
    if (ret == EID(SUCCESS))
    {
        arrayList.emplace(name, array);
    }
    else
    {
        POS_TRACE_WARN(EID(CREATE_ARRAY_DEBUG_MSG),
            "ArrayManager is cleaning up ArrayComponents of {} because of array creation failure", name);
        delete array;
    }
    pthread_rwlock_unlock(&arrayListLock);
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
        if (AbrExists(name))
        {
            int result = _DeleteFaultArray(name);
            return result;
        }
        int ret = EID(DELETE_ARRAY_ARRAY_NAME_DOES_NOT_EXIST);
        POS_TRACE_WARN(ret, "array_name: {}", name);
        return ret;
    }

    int ret = array->Delete();
    if (ret == EID(SUCCESS))
    {
        delete array;
        arrayList.erase(name);
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
    pthread_rwlock_rdlock(&arrayListLock);
    ArrayComponents* array = _FindArrayWithDevSN(dev->GetSN());
    int ret = 0;
    if (array != nullptr)
    {
        ret = array->GetArray()->DetachDevice(dev);
    }
    else
    {
        POS_TRACE_WARN(EID(ARRAY_EVENT_DEV_DETACHED),
            "SSD is detached-There is no array to which {} ({}) belongs",
            dev->GetName(), dev->GetSN());
        deviceManager->RemoveDevice(dev);
    }
    pthread_rwlock_unlock(&arrayListLock);
    return ret; // this function will be void type when device lock is removed
}

void
ArrayManager::DeviceAttached(UblockSharedPtr dev)
{
    abrManager->InitDisk(dev);
}

int
ArrayManager::GetAbrList(std::vector<ArrayBootRecord>& abrList)
{
    int result = abrManager->GetAbrList(abrList);
    return result;
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
ArrayManager::Load(list<string>& failedArrayList)
{
    std::vector<ArrayBootRecord> abrList;
    std::vector<ArrayBootRecord>::iterator it;
    int result = abrManager->GetAbrList(abrList);
    int loadResult = 0;
    if (result == 0)
    {
        for (it = abrList.begin(); it != abrList.end(); it++)
        {
            loadResult = _Load(it->arrayName);
            if (loadResult != 0)
            {
                string arrayName(it->arrayName);
                POS_TRACE_ERROR(loadResult, "Array " + arrayName + " load failed");
                failedArrayList.push_back(arrayName);
                result = loadResult;
            }
        }
    }
    else
    {
        POS_TRACE_ERROR(result, "Failed to get abr list");
    }

    return result;
}

int
ArrayManager::_Load(string name)
{
    POS_TRACE_INFO(EID(LOAD_MBR_TRYING), "array_name:{}", name);
    pthread_rwlock_wrlock(&arrayListLock);
    ArrayComponents* array = _FindArray(name);
    if (array != nullptr)
    {
        pthread_rwlock_unlock(&arrayListLock);
        POS_TRACE_WARN(EID(LOAD_ARRAY_ALREADY_LOADED), "arrayname: {}", name);
        return EID(LOAD_ARRAY_ALREADY_LOADED);
    }

    array = arrayComponentsFactory(name, arrayRebuilder, abrManager);
    int ret = array->Load();
    if (ret == EID(SUCCESS))
    {
        POS_TRACE_INFO(EID(LOAD_MBR_COMPLETED), "array_name:{}", name);
        arrayList.emplace(name, array);
    }
    else
    {
        POS_TRACE_WARN(EID(LOAD_MBR_FAILED), "array_name:{}", name);
        delete array;
    }

    pthread_rwlock_unlock(&arrayListLock);
    return ret;
}

int
ArrayManager::ResetMbr(void)
{
    POS_TRACE_INFO(EID(RESET_MBR_TRYING), "");
    pthread_rwlock_wrlock(&arrayListLock);
    int result = 0;
    for (auto iter = arrayList.begin(); iter != arrayList.end();)
    {
        ArrayComponents* array = _FindArray(iter->first);
        if (array != nullptr)
        {
            result = array->Delete();
            if (result == 0)
            {
                delete array;
                iter = arrayList.erase(iter);
                continue;
            }
        }
        pthread_rwlock_unlock(&arrayListLock);
        POS_TRACE_WARN(result, "Unable to delete array during reset mbr " + iter->first);
        return result;
    }

    pthread_rwlock_unlock(&arrayListLock);
    result = abrManager->ResetMbr();
    if (result == 0)
    {
        POS_TRACE_INFO(EID(RESET_MBR_COMPLETED), "");
    }
    else
    {
        POS_TRACE_WARN(EID(RESET_MBR_FAILED), "");
    }
    return result;
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

ArrayComponents*
ArrayManager::_FindArrayWithDevSN(string devSN)
{
    string arrayName = abrManager->FindArrayWithDeviceSN(devSN);
    if (arrayName == "")
    {
        int eventId = EID(ARRAY_MGR_NO_ARRAY_OWNING_REQ_DEV);
        POS_TRACE_INFO(eventId, "There is no array that owns device '{}'", devSN);
        return nullptr;
    }
    ArrayComponents* array = _FindArray(arrayName);

    return array;
}

bool
ArrayManager::AbrExists(string arrayName)
{
    std::vector<ArrayBootRecord> abrList;
    int result = GetAbrList(abrList);

    if (result == 0)
    {
        for (auto const &abr : abrList)
        {
            if (abr.arrayName == arrayName)
            {
                return true;
            }
        }
    }

    return false;
}

int
ArrayManager::_DeleteFaultArray(string arrayName)
{
    ArrayMeta meta;
    meta.arrayName = arrayName;
    int result = abrManager->LoadAbr(meta);
    if (result != 0)
    {
        return result;
    }
    result = abrManager->DeleteAbr(arrayName);

    return result;
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

string
ArrayManager::GetTargetAddress(string name)
{
    pthread_rwlock_rdlock(&arrayListLock);
    ArrayComponents* array = _FindArray(name);
    string addr = array->GetTargetAddress();
    pthread_rwlock_unlock(&arrayListLock);
    return addr;
}

} // namespace pos
