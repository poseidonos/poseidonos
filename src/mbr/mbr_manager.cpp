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

#include "mbr_manager.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <utility>

#include "src/array/array_name_policy.h"
#include "src/array/device/array_device_list.h"
#include "src/device/base/ublock_device.h"
#include "src/device/device_manager.h"
#include "src/helper/time/time_helper.h"
#include "src/include/array_device_state.h"
#include "src/include/pos_event_id.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/logger/logger.h"
#include "version_provider.h"

using namespace std::placeholders;
namespace pos
{
MbrManager::MbrManager(void)
: MbrManager(
      new DataProtect(),
      _GetSystemUuid(),
      bind(&MbrManager::_DiskIo, this, _1, _2),
      bind(&MbrManager::_IterateReadFromDevices, this, _1, _2),
      DeviceManagerSingleton::Instance(),
      new MbrMapManager())
{
    // delegate to other constructor with default injections of DP, UUID, iterators, and MBR
}

MbrManager::MbrManager(
    DataProtect* dataProtect,
    string systemUuid,
    DeviceIterFunc diskIoFunc,
    DeviceIterFunc iterateReadFunc,
    DeviceManager* devMgr,
    MbrMapManager* mapMgr)
: systemUuid(systemUuid),
  dataProtect(dataProtect),
  diskIoFunc(diskIoFunc),
  iterateReadFunc(iterateReadFunc),
  devMgr(devMgr),
  mapMgr(mapMgr)
{
    pthread_rwlock_init(&mbrLock, nullptr);

    memset(&systeminfo, '\0', sizeof(systeminfo));
    for (unsigned int i = 0; i < MAX_ARRAY_CNT; i++)
    {
        systeminfo.arrayValidFlag[i] = 0;
    }
}

MbrManager::~MbrManager(void)
{
    if (mbrBuffer != nullptr)
    {
        pos::Memory<CHUNK_SIZE>::Free(mbrBuffer);
    }
    delete dataProtect;
    delete mapMgr;
}

struct masterBootRecord&
MbrManager::GetMbr(void)
{
    return systeminfo;
}

// All actions are assumed to be mutually exclusive as array state lock.
int
MbrManager::LoadMbr(void)
{
    int ret = EID(SUCCESS);
    pthread_rwlock_wrlock(&mbrLock);
    ret = _ReadFromDevices();
    if (ret != 0)
    {
        pthread_rwlock_unlock(&mbrLock);
        return ret;
    }

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MBR_READ_DONE,
        "read mbr data");

    _LoadIndexMap();

    if (systeminfo.arrayNum != arrayIndexMap.size())
    {
        ret = (int)POS_EVENT_ID::MBR_WRONG_ARRAY_INDEX_MAP;
    }

    pthread_rwlock_unlock(&mbrLock);
    return ret;
}

int
MbrManager::SaveMbr(void)
{
    pthread_rwlock_wrlock(&mbrLock);
    string posVersion = VersionProviderSingleton::Instance()->GetVersion();
    CopyData(systeminfo.posVersion, posVersion, POS_VERSION_SIZE);
    systeminfo.mbrVersion = version;
    CopyData(systeminfo.systemUuid, systemUuid, SYSTEM_UUID_SIZE);

    int result = _WriteToDevices();
    if (result != 0)
    {
        int eventid = (int)POS_EVENT_ID::MBR_WRITE_ERROR;
        POS_TRACE_ERROR(eventid, "MBR Write Error");
        result = eventid;
    }
    pthread_rwlock_unlock(&mbrLock);
    return result;
}

int
MbrManager::ResetMbr(void)
{
    pthread_rwlock_wrlock(&mbrLock);
    version = 0;
    _AllocMem();
    if (mbrBuffer != nullptr)
    {
        memset(mbrBuffer, 0, MBR_SIZE);
    }

    memset(&systeminfo, 0, sizeof(systeminfo));
    for (unsigned int i = 0; i < MAX_ARRAY_CNT; i++)
    {
        systeminfo.arrayValidFlag[i] = 0;
    }

    arrayIndexMap.clear();
    mapMgr->ResetMap();

    struct DiskIoContext diskIoCtxt = {UbioDir::Write, mbrBuffer};
    int result = devMgr->IterateDevicesAndDoFunc(diskIoFunc, &diskIoCtxt);
    if (result != 0)
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::MBR_DEVICE_NOT_FOUND,
            "device not found");
        pthread_rwlock_unlock(&mbrLock);
        return (int)POS_EVENT_ID::MBR_DEVICE_NOT_FOUND;
    }
    pthread_rwlock_unlock(&mbrLock);
    return result;
}

bool
MbrManager::_AllocMem(void)
{
    bool ret = false;

    if (mbrBuffer == nullptr)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::MBR_ALLOCATE_MEMORY,
            "MBR allocate memory done");
        mbrBuffer = pos::Memory<CHUNK_SIZE>::Alloc(MBR_CHUNKS);
    }

    if (mbrBuffer != nullptr)
    {
        ret = true;
    }
    return ret;
}

int
MbrManager::_WriteToDevices(void)
{
    version++;
    systeminfo.mbrVersion = version;
    _AllocMem();
    memcpy(mbrBuffer, &systeminfo, MBR_SIZE);
    _SetParity(mbrBuffer);

    struct DiskIoContext diskIoCtxt = {UbioDir::Write, mbrBuffer};
    int result = devMgr->IterateDevicesAndDoFunc(diskIoFunc, &diskIoCtxt);
    if (result != 0)
    {
        version--;
        systeminfo.mbrVersion = version;
        POS_TRACE_WARN((int)POS_EVENT_ID::MBR_DEVICE_NOT_FOUND,
            "device not found");
        return (int)POS_EVENT_ID::MBR_DEVICE_NOT_FOUND;
    }
    POS_TRACE_DEBUG((int)POS_EVENT_ID::MBR_WRITE_DONE,
        "write mbr data");

    return result;
}

void
MbrManager::_IterateReadFromDevices(UblockSharedPtr dev, void* ctx)
{
    if (dev->GetType() == DeviceType::NVRAM)
    {
        return;
    }

    std::list<void*>* mems = static_cast<std::list<void*>*>(ctx);
    void* mem = nullptr;

    mem = pos::Memory<CHUNK_SIZE>::Alloc(MBR_CHUNKS);

    struct DiskIoContext diskIoCtxt = {UbioDir::Read, mem};
    _DiskIo(dev, &diskIoCtxt);

    if ((0 == _VerifyParity(mem)) && (0 == _VerifySystemUuid(mem)))
    {
        mems->push_back(mem);
    }
    else
    {
        pos::Memory<CHUNK_SIZE>::Free(mem);
    }
}

int
MbrManager::_ReadFromDevices(void)
{
    std::list<void*> mems;

    int result = devMgr->IterateDevicesAndDoFunc(iterateReadFunc, &mems);

    if (result != 0)
    {
        return result;
    }

    if (0 < mems.size())
    {
        std::list<void*> latestMems;
        _GetLatestDataList(mems, &latestMems);

        std::list<void*>::iterator memIter = latestMems.begin();

        uint32_t maxValidCnt = 0;
        uint32_t maxValidDataIndex = 0;
        uint32_t baseLoop = 0;
        for (; baseLoop < latestMems.size(); baseLoop++, ++memIter)
        {
            uint32_t validCnt = 0;
            uint32_t cmpLoop = 0;
            list<void*>::iterator cmpMemIter = latestMems.begin();

            for (; cmpLoop < latestMems.size(); cmpLoop++, ++cmpMemIter)
            {
                if (0 == memcmp(*cmpMemIter, *memIter, MBR_SIZE * MBR_CHUNKS))
                {
                    validCnt++;
                }
            }

            if (maxValidCnt < validCnt)
            {
                maxValidCnt = validCnt;
                maxValidDataIndex = baseLoop;
            }
        }

        memIter = latestMems.begin();
        std::advance(memIter, maxValidDataIndex);
        version = _GetVersion(*memIter);

        memcpy(&systeminfo, *memIter, MBR_SIZE);

        for (memIter = mems.begin(); memIter != mems.end(); ++memIter)
        {
            pos::Memory<CHUNK_SIZE>::Free(*memIter);
        }
    }
    else
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::MBR_DATA_NOT_FOUND,
            "mbr data not found");
        result = (int)POS_EVENT_ID::MBR_DATA_NOT_FOUND; // no mbr data
    }

    return result;
}

void
MbrManager::_DiskIo(UblockSharedPtr dev, void* ctx)
{
    struct DiskIoContext* ctxt = static_cast<struct DiskIoContext*>(ctx);
    UbioSmartPtr bio(new Ubio(ctxt->mem, MBR_BLOCKS * Ubio::UNITS_PER_BLOCK, 0));

    bio->dir = ctxt->ubioDir;
    bio->SetLba(MBR_ADDRESS);
    bio->SetUblock(dev);

    IODispatcher& ioDispatcher = *IODispatcherSingleton::Instance();
    ioDispatcher.Submit(bio, true);
}

void
MbrManager::_SetParity(void* mem)
{
    uint32_t mbrParity = dataProtect->MakeParity((unsigned char*)mem,
        MBR_SIZE - MBR_PARITY_SIZE);
    memcpy(((char*)mem + MBR_SIZE - MBR_PARITY_SIZE), &mbrParity, MBR_PARITY_SIZE);
}

int
MbrManager::_VerifyParity(void* mem)
{
    uint32_t mbrParity = dataProtect->MakeParity((unsigned char*)mem,
        MBR_SIZE - MBR_PARITY_SIZE);

    int ret = memcmp((char*)mem + MBR_SIZE - MBR_PARITY_SIZE,
        &mbrParity, MBR_PARITY_SIZE);
    if (ret == 0)
    {
        return EID(SUCCESS);
    }
    else
    {
        char* buf = new char[MBR_SIZE];
        memset(buf, 0, MBR_SIZE);
        uint32_t bufParity = dataProtect->MakeParity(
            (unsigned char*)buf, MBR_SIZE - MBR_PARITY_SIZE);
        delete[] buf;

        if (bufParity == mbrParity)
        {
            POS_TRACE_WARN((int)POS_EVENT_ID::MBR_PARITY_CHECK_FAILED,
                "mbr parity check fail, mbr is reset");
        }
        else
        {
            POS_TRACE_WARN((int)POS_EVENT_ID::MBR_PARITY_CHECK_FAILED,
                "mbr parity check fail");
        }
        return (int)POS_EVENT_ID::MBR_PARITY_CHECK_FAILED; // e2e data protect error
    }
}

int
MbrManager::_VerifySystemUuid(void* mem)
{
    int ret;
    struct masterBootRecord* temp = reinterpret_cast<struct masterBootRecord*>(mem);

    if (systemUuid == temp->systemUuid)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::MBR_SYSTEM_UUID_CHECK,
            "mbr system uuid check ");
        ret = EID(SUCCESS);
    }
    else
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::MBR_SYSTEM_UUID_CHECK_FAILED,
            "mbr system uuid check fail");
        ret = (int)POS_EVENT_ID::MBR_SYSTEM_UUID_CHECK_FAILED;
    }
    return ret;
}

int
MbrManager::_GetVersion(void* mem)
{
    struct masterBootRecord* temp = reinterpret_cast<struct masterBootRecord*>(mem);
    int mbrVersion = temp->mbrVersion;

    return mbrVersion;
}

int
MbrManager::_GetLatestDataList(list<void*> mems, list<void*>* latestMems)
{
    if (mems.size() == 0)
    {
        POS_TRACE_WARN((int)POS_EVENT_ID::MBR_DATA_NOT_FOUND,
            "mbr data not found");
        return (int)POS_EVENT_ID::MBR_DATA_NOT_FOUND;
    }
    std::list<void*>::iterator it = mems.begin();

    int latestVer = 0;
    std::vector<int> verArray;
    for (uint32_t index = 0; index < mems.size(); ++it, index++)
    {
        verArray.push_back(_GetVersion(*it));
        if (verArray[index] > latestVer)
        {
            latestVer = verArray[index];
        }
    }

    it = mems.begin();
    for (uint32_t index = 0; index < mems.size(); ++it, index++)
    {
        if (verArray[index] == latestVer)
        {
            latestMems->push_back(*it);
        }
    }

    return EID(SUCCESS);
}

string
MbrManager::_GetSystemUuid(void)
{
    const string uuidPath = "/sys/class/dmi/id/product_uuid";
    ifstream inputFile(uuidPath, ifstream::in);

    if (false == inputFile.is_open())
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MBR_GET_SYSTEM_UUID_FAILED,
            "mbr get system uuid failed");
        return string("");
    }

    string uuid;
    inputFile >> uuid;
    inputFile.close();
    return uuid;
}

int
MbrManager::CreateAbr(ArrayMeta& meta)
{
    int ret = 0;
    ArrayNamePolicy arrayNamePolicy;
    ret = arrayNamePolicy.CheckArrayName(meta.arrayName);
    if (ret != EID(SUCCESS))
    {
        POS_TRACE_ERROR(ret, "Array name double check failed");
        return ret;
    }

    if (systeminfo.arrayNum > MAX_ARRAY_CNT)
    {
        return (int)POS_EVENT_ID::MBR_MAX_ARRAY_CNT_EXCEED;
    }

    pthread_rwlock_wrlock(&mbrLock);
    if (arrayIndexMap.find(meta.arrayName) != arrayIndexMap.end())
    {
        pthread_rwlock_unlock(&mbrLock);
        return (int)POS_EVENT_ID::MBR_ABR_ALREADY_EXIST;
    }

    ret = mapMgr->CheckAllDevices(meta);
    if (ret != 0)
    {
        POS_TRACE_ERROR(ret, "Device is already in other array");
        pthread_rwlock_unlock(&mbrLock);
        return ret;
    }

    unsigned int tempArrayIndex;
    for (tempArrayIndex = 0; tempArrayIndex < MAX_ARRAY_CNT; tempArrayIndex++)
    {
        if (systeminfo.arrayValidFlag[tempArrayIndex] == 0)
        {
            pair<map<string, unsigned int>::iterator, bool> ret;
            ret = arrayIndexMap.insert(pair<string, unsigned int>(meta.arrayName, tempArrayIndex));
            if (ret.second == false)
            {
                POS_TRACE_ERROR((int)POS_EVENT_ID::MBR_WRONG_ARRAY_INDEX_MAP,
                    "Array index map doesn't match with previous condition");
            }

            mapMgr->InsertDevices(meta, tempArrayIndex);

            systeminfo.arrayValidFlag[tempArrayIndex] = 1;
            systeminfo.arrayNum++;
            memset(&systeminfo.arrayInfo[tempArrayIndex], '\0', sizeof(ArrayBootRecord));
            CopyData(systeminfo.arrayInfo[tempArrayIndex].arrayName,
                meta.arrayName, ARRAY_NAME_SIZE);
            CopyData(systeminfo.arrayInfo[tempArrayIndex].createDatetime,
                Time::GetCurrentTimeStr("%Y-%m-%d %X %z", DATE_SIZE), DATE_SIZE);
            CopyData(systeminfo.arrayInfo[tempArrayIndex].updateDatetime,
                Time::GetCurrentTimeStr("%Y-%m-%d %X %z", DATE_SIZE), DATE_SIZE);
            systeminfo.arrayInfo[tempArrayIndex].uniqueId = meta.unique_id;
            pthread_rwlock_unlock(&mbrLock);
            meta.id = tempArrayIndex;
            return 0;
        }
    }

    pthread_rwlock_unlock(&mbrLock);
    return (int)POS_EVENT_ID::MBR_WRONG_ARRAY_VALID_FLAG;
}

int
MbrManager::DeleteAbr(string arrayName)
{
    pthread_rwlock_wrlock(&mbrLock);
    int result = (int)POS_EVENT_ID::MBR_ABR_NOT_FOUND;
    unsigned int arrayIndex;
    arrayIndexMapIter iter;
    iter = arrayIndexMap.find(arrayName);
    if (iter == arrayIndexMap.end())
    {
        pthread_rwlock_unlock(&mbrLock);
        return (int)POS_EVENT_ID::MBR_ABR_NOT_FOUND;
    }

    arrayIndex = iter->second;
    string targetArrayName(systeminfo.arrayInfo[arrayIndex].arrayName);
    if (arrayName == targetArrayName)
    {
        ArrayBootRecord* backup = new ArrayBootRecord;
        memcpy(backup, &(systeminfo.arrayInfo[arrayIndex]), sizeof(ArrayBootRecord));
        systeminfo.arrayValidFlag[arrayIndex] = 0;
        systeminfo.arrayNum--;
        memset(&systeminfo.arrayInfo[arrayIndex], '\0', sizeof(ArrayBootRecord));

        result = _WriteToDevices();
        if (result != 0)
        {
            memcpy(&(systeminfo.arrayInfo[arrayIndex]), backup, sizeof(ArrayBootRecord));
            delete backup;
            systeminfo.arrayValidFlag[arrayIndex] = 1;
            systeminfo.arrayNum++;
            int eventid = (int)POS_EVENT_ID::MBR_WRITE_ERROR;
            POS_TRACE_ERROR(eventid, "MBR Write Error");
            result = eventid;
            pthread_rwlock_unlock(&mbrLock);
            return result;
        }
        delete backup;
    }
    else
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MBR_WRONG_ARRAY_INDEX_MAP,
            "Array index map doesn't match with MBR");
    }

    arrayIndexMap.erase(iter);
    mapMgr->DeleteDevices(arrayIndex);

    pthread_rwlock_unlock(&mbrLock);
    return result;
}

void
MbrManager::GetAbr(string targetArrayName, struct ArrayBootRecord** abr, unsigned int& arrayIndex)
{
    pthread_rwlock_wrlock(&mbrLock);
    unsigned int tempArrayIndex = -1;
    *abr = nullptr;
    arrayIndexMapIter iter;
    iter = arrayIndexMap.find(targetArrayName);

    if (iter != arrayIndexMap.end())
    {
        tempArrayIndex = iter->second;
        string arrayName(systeminfo.arrayInfo[tempArrayIndex].arrayName);
        if (targetArrayName == arrayName)
        {
            *abr = &(systeminfo.arrayInfo)[tempArrayIndex];
            arrayIndex = tempArrayIndex;
        }
        else
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::MBR_WRONG_ARRAY_VALID_FLAG,
                "Array Valid Flag doesn't match with Index Map");
        }
    }
    pthread_rwlock_unlock(&mbrLock);
}

int
MbrManager::GetAbrList(std::vector<ArrayBootRecord>& abrList)
{
    pthread_rwlock_wrlock(&mbrLock);
    unsigned int arrayNum = systeminfo.arrayNum;
    unsigned int arrayIndex = 0;

    int result = 0;
    result = _ReadFromDevices();
    if (result != 0)
    {
        pthread_rwlock_unlock(&mbrLock);
        return result;
    }

    arrayIndexMapIter iter;
    for (iter = arrayIndexMap.begin(); iter != arrayIndexMap.end(); iter++)
    {
        arrayIndex = iter->second;
        abrList.push_back(systeminfo.arrayInfo[arrayIndex]);
    }

    if (arrayNum != arrayIndexMap.size())
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MBR_WRONG_ARRAY_INDEX_MAP,
            "Array index map doesn't match with MBR : arrayNum = {}, searched = {}",
            arrayNum, arrayIndexMap.size());
    }
    pthread_rwlock_unlock(&mbrLock);
    return result;
}

int
MbrManager::GetMbrVersionInMemory(void)
{
    return version;
}

int
MbrManager::UpdateDeviceIndexMap(string arrayName)
{
    pthread_rwlock_wrlock(&mbrLock);
    arrayIndexMapIter it = arrayIndexMap.find(arrayName);
    unsigned int arrayIndex = it->second;
    mapMgr->DeleteDevices(arrayIndex);
    for (unsigned int j = 0; j < systeminfo.arrayInfo[arrayIndex].totalDevNum; j++)
    {
        string deviceUid = systeminfo.arrayInfo[arrayIndex].devInfo[j].deviceUid;
        mapMgr->InsertDevice(deviceUid, arrayIndex);
    }

    pthread_rwlock_unlock(&mbrLock);
    return 0;
}

int
MbrManager::_LoadIndexMap(void)
{
    arrayIndexMap.clear();
    mapMgr->ResetMap();

    for (unsigned int i = 0; i < MAX_ARRAY_CNT; i++)
    {
        string arrayName(systeminfo.arrayInfo[i].arrayName);
        if (systeminfo.arrayValidFlag[i] == 1 &&
            arrayIndexMap.find(arrayName) == arrayIndexMap.end())
        {
            arrayIndexMap.insert(pair<string, unsigned int>(arrayName, i));

            for (unsigned int j = 0; j < systeminfo.arrayInfo[i].totalDevNum; j++)
            {
                string deviceUid = systeminfo.arrayInfo[i].devInfo[j].deviceUid;
                mapMgr->InsertDevice(deviceUid, i);
            }
        }
    }
    return 0;
}

string
MbrManager::FindArrayWithDeviceSN(string devSN)
{
    pthread_rwlock_rdlock(&mbrLock);
    int arrayIndex = mapMgr->FindArrayIndex(devSN);
    pthread_rwlock_unlock(&mbrLock);
    if (0 <= arrayIndex)
    {
        return systeminfo.arrayInfo[arrayIndex].arrayName;
    }
    else
    {
        return "";
    }
}

} // namespace pos
