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

#include "mbr_manager.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <string>

#include "src/array/device/array_device.h"
#include "src/array/device/array_device_list.h"
#include "src/device/device_manager.h"
#include "src/device/ublock_device.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/io_dispatcher.h"
#include "version_provider.h"

namespace ibofos
{
MbrManager::MbrManager(void)
{
    dataProtect = new DataProtect();
    systemUuid = _GetSystemUuid();
    using namespace std::placeholders;
    diskIoFunc = bind(&MbrManager::_DiskIo, this, _1, _2);
    iterateReadFunc = bind(&MbrManager::_IterateReadFromDevices, this, _1, _2);

    memset(&systeminfo, '\0', sizeof(systeminfo));
}

MbrManager::~MbrManager(void)
{
    if (mbrBuffer != nullptr)
    {
        ibofos::Memory<CHUNK_SIZE>::Free(mbrBuffer);
    }
    delete dataProtect;
}

bool
MbrManager::_AllocMem(void)
{
    bool ret = false;

    if (mbrBuffer == nullptr)
    {
        IBOF_TRACE_INFO((int)IBOF_EVENT_ID::MBR_ALLOCATE_MEMORY,
            "MBR allocate memory done");
        mbrBuffer = ibofos::Memory<CHUNK_SIZE>::Alloc(MBR_CHUNKS);
    }

    if (mbrBuffer != nullptr)
    {
        ret = true;
    }
    return ret;
}

int
MbrManager::Write(int arrayNum, ArrayMeta& meta)
{
    string posVersion = VersionProviderSingleton::Instance()->GetVersion();

    _CopyData(systeminfo.pos_version, posVersion, POS_VERSION_SIZE);
    systeminfo.mbr_version = version;
    _CopyData(systeminfo.system_uuid, systemUuid, SYSTEM_UUID_SIZE);

    int nvmNum = meta.devs.nvm.size();
    int dataNum = meta.devs.data.size();
    int spareNum = meta.devs.spares.size();

    systeminfo.array_info[arrayNum].nvm_dev_num = nvmNum;
    systeminfo.array_info[arrayNum].data_dev_num = dataNum;
    systeminfo.array_info[arrayNum].spare_dev_num = spareNum;
    systeminfo.array_info[arrayNum].total_dev_num = nvmNum + dataNum + spareNum;

    _CopyData(systeminfo.array_info[arrayNum].array_name, meta.arrayName, ARRAY_NAME_SIZE);
    _CopyData(systeminfo.array_info[arrayNum].meta_raid_type, meta.metaRaidType, META_RAID_TYPE_SIZE);
    _CopyData(systeminfo.array_info[arrayNum].data_raid_type, meta.dataRaidType, DATA_RAID_TYPE_SIZE);

    for (int i = 0; i < nvmNum; i++)
    {
        int deviceIndex = i;
        systeminfo.array_info[arrayNum].dev_info[deviceIndex].device_type =
            (int)ArrayDeviceType::NVM;
        _CopyData(systeminfo.array_info[arrayNum].dev_info[deviceIndex].device_uid,
            meta.devs.nvm.at(i).uid, DEVICE_UID_SIZE);
    }

    for (int i = 0; i < dataNum; i++)
    {
        int deviceIndex = nvmNum + i;
        systeminfo.array_info[arrayNum].dev_info[deviceIndex].device_type =
            (int)ArrayDeviceType::DATA;
        _CopyData(systeminfo.array_info[arrayNum].dev_info[deviceIndex].device_uid,
            meta.devs.data.at(i).uid, DEVICE_UID_SIZE);
        systeminfo.array_info[arrayNum].dev_info[deviceIndex].device_state =
            (int)meta.devs.data.at(i).state;
    }

    for (int i = 0; i < spareNum; i++)
    {
        int deviceIndex = nvmNum + dataNum + i;
        systeminfo.array_info[arrayNum].dev_info[deviceIndex].device_type =
            (int)ArrayDeviceType::SPARE;
        _CopyData(systeminfo.array_info[arrayNum].dev_info[deviceIndex].device_uid,
            meta.devs.spares.at(i).uid, DEVICE_UID_SIZE);
    }

    systeminfo.array_info[arrayNum].mfsInit = GetMfsInit(arrayNum);
    int result = _WriteToDevices();

    return result;
}

int
MbrManager::Read(void)
{
    _ReadFromDevices();

    string mbrData = _ParseMbrData();

    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::MBR_READ_DONE,
        "read mbr data, mbrdata:{}", mbrData);

    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
MbrManager::Reset(void)
{
    version = 0;

    _AllocMem();

    memset(mbrBuffer, 0, MBR_SIZE);
    memset(&systeminfo, 0, sizeof(systeminfo));

    DeviceManager* devMgr = DeviceManagerSingleton::Instance();
    struct DiskIoContext diskIoCtxt = {UbioDir::Write, mbrBuffer};
    int result = devMgr->IterateDevicesAndDoFunc(diskIoFunc, &diskIoCtxt);
    if (result != 0)
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::MBR_DEVICE_NOT_FOUND,
            "device not found");
        return (int)IBOF_EVENT_ID::MBR_DEVICE_NOT_FOUND;
    }
    return result;
}

int
MbrManager::RebuildMbr(void)
{
    char* buf = new char[BLOCK_SIZE];
    _ReadFromDevices();
    _WriteToDevices();

    delete[] buf;
    buf = nullptr;
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
MbrManager::_WriteToDevices(void)
{
    _AllocMem();
    string mbrData = _ParseMbrData();

    memcpy(mbrBuffer, &systeminfo, MBR_SIZE);
    _SetParity(mbrBuffer);

    DeviceManager* devMgr = DeviceManagerSingleton::Instance();
    struct DiskIoContext diskIoCtxt = {UbioDir::Write, mbrBuffer};
    int result = devMgr->IterateDevicesAndDoFunc(diskIoFunc, &diskIoCtxt);
    if (result != 0)
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::MBR_DEVICE_NOT_FOUND,
            "device not found");
        return (int)IBOF_EVENT_ID::MBR_DEVICE_NOT_FOUND;
    }
    version++;
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::MBR_WRITE_DONE,
        "write mbr data, mbrdata:{}", mbrData);

    return result;
}

void
MbrManager::_IterateReadFromDevices(UBlockDevice* dev, void* ctx)
{
    std::list<void*>* mems = static_cast<std::list<void*>*>(ctx);
    void* mem = nullptr;

    mem = ibofos::Memory<CHUNK_SIZE>::Alloc(MBR_CHUNKS);

    struct DiskIoContext diskIoCtxt = {UbioDir::Read, mem};
    _DiskIo(dev, &diskIoCtxt);

    if ((0 == _VerifyParity(mem)) && (0 == _VerifySystemUuid(mem)))
    {
        mems->push_back(mem);
    }
    else
    {
        ibofos::Memory<CHUNK_SIZE>::Free(mem);
    }
}

int
MbrManager::_ReadFromDevices(void)
{
    std::list<void*> mems;

    DeviceManager* devMgr = DeviceManagerSingleton::Instance();
    int result = devMgr->IterateDevicesAndDoFunc(iterateReadFunc, &mems);

    if (result != 0)
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::MBR_DEVICE_NOT_FOUND,
            "device not found");
        return (int)IBOF_EVENT_ID::MBR_DEVICE_NOT_FOUND;
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
            ibofos::Memory<CHUNK_SIZE>::Free(*memIter);
        }
    }
    else
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::MBR_DATA_NOT_FOUND,
            "mbr data not found");
        result = (int)IBOF_EVENT_ID::MBR_DATA_NOT_FOUND; // no mbr data
    }

    return result;
}

void
MbrManager::_DiskIo(UBlockDevice* dev, void* ctx)
{
    struct DiskIoContext* ctxt = static_cast<struct DiskIoContext*>(ctx);
    UbioSmartPtr bio(new Ubio(ctxt->mem, MBR_BLOCKS * Ubio::UNITS_PER_BLOCK));

    bio->dir = ctxt->ubioDir;
    ArrayDevice* arrayDev = new ArrayDevice(dev, ArrayDeviceState::NORMAL);
    PhysicalBlkAddr pba = {.dev = arrayDev, .lba = MBR_ADDRESS};
    bio->SetPba(pba);

    IODispatcher& ioDispatcher = *EventArgument::GetIODispatcher();
    ioDispatcher.Submit(bio, true);

    delete arrayDev;
}

int
MbrManager::LoadMeta(int arrayNum, ArrayMeta& meta, string targetArray)
{
    int result = _ReadFromDevices();
    if (result != 0)
    {
        return result;
    }
    string arrayName(systeminfo.array_info[arrayNum].array_name);

    if (targetArray != arrayName)
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array requested does not exist");
        return (int)IBOF_EVENT_ID::ARRAY_WRONG_NAME;
    }

    string mbrData = _ParseMbrData();

    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::MBR_READ_DONE,
        "read mbr data, mbrdata:{}", mbrData);

    int nvmNum = systeminfo.array_info[arrayNum].nvm_dev_num;
    int dataNum = systeminfo.array_info[arrayNum].data_dev_num;
    int spareNum = systeminfo.array_info[arrayNum].spare_dev_num;

    int devIndex = 0;

    for (int i = 0; i < nvmNum; i++)
    {
        devIndex = i;
        string uid(systeminfo.array_info[arrayNum].dev_info[devIndex].device_uid);
        meta.devs.nvm.push_back(DeviceMeta(uid));
    }

    for (int i = 0; i < dataNum; i++)
    {
        devIndex = nvmNum + i;
        string uid(systeminfo.array_info[arrayNum].dev_info[devIndex].device_uid);
        int state = systeminfo.array_info[arrayNum].dev_info[devIndex].device_state;
        DeviceMeta deviceMeta(uid, static_cast<ArrayDeviceState>(state));
        meta.devs.data.push_back(deviceMeta);
    }

    for (int i = 0; i < spareNum; i++)
    {
        devIndex = nvmNum + dataNum + i;
        string uid(systeminfo.array_info[arrayNum].dev_info[devIndex].device_uid);
        meta.devs.spares.push_back(DeviceMeta(uid));
    }

    return 0;
}

bool
MbrManager::_IsValidDevice(UBlockDevice* ublockDev)
{
    bool ret = false;

    if (ublockDev->GetType() == DeviceType::SSD)
    {
        ret = true;
    }
    else if (ublockDev->GetType() == DeviceType::NVRAM)
    {
        ret = true;
    }

    return ret;
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
        return (int)IBOF_EVENT_ID::SUCCESS;
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
            IBOF_TRACE_WARN((int)IBOF_EVENT_ID::MBR_PARITY_CHECK_FAILED,
                "mbr parity check fail, mbr is reset");
        }
        else
        {
            IBOF_TRACE_WARN((int)IBOF_EVENT_ID::MBR_PARITY_CHECK_FAILED,
                "mbr parity check fail");
        }
        return (int)IBOF_EVENT_ID::MBR_PARITY_CHECK_FAILED; // e2e data protect error
    }
}

int
MbrManager::_VerifySystemUuid(void* mem)
{
    int ret;
    struct systemBootRecord* temp = reinterpret_cast<struct systemBootRecord*>(mem);

    if (systemUuid == temp->system_uuid)
    {
        IBOF_TRACE_INFO((int)IBOF_EVENT_ID::MBR_SYSTEM_UUID_CHECK,
            "mbr system uuid check ");
        ret = (int)IBOF_EVENT_ID::SUCCESS;
    }
    else
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::MBR_SYSTEM_UUID_CHECK_FAILED,
            "mbr system uuid check fail");
        ret = (int)IBOF_EVENT_ID::MBR_SYSTEM_UUID_CHECK_FAILED;
    }
    return ret;
}

int
MbrManager::_GetVersion(void* mem)
{
    struct systemBootRecord* temp = reinterpret_cast<struct systemBootRecord*>(mem);
    int mbrVersion = temp->mbr_version;

    return mbrVersion;
}

int
MbrManager::_GetLatestDataList(list<void*> mems, list<void*>* latestMems)
{
    if (mems.size() == 0)
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::MBR_DATA_NOT_FOUND,
            "mbr data not found");
        return (int)IBOF_EVENT_ID::MBR_DATA_NOT_FOUND;
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

    return (int)IBOF_EVENT_ID::SUCCESS;
}

string
MbrManager::_GetSystemUuid(void)
{
    const string uuidPath = "/sys/class/dmi/id/product_uuid";
    ifstream inputFile(uuidPath, ifstream::in);

    if (false == inputFile.is_open())
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::MBR_GET_SYSTEM_UUID_FAILED,
            "mbr get system uuid failed");
        return string("");
    }

    string uuid;
    inputFile >> uuid;
    inputFile.close();
    return uuid;
}

bool
MbrManager::GetMfsInit(int arrayNum)
{
    int value = systeminfo.array_info[arrayNum].mfsInit;
    return value ? true : false;
}

int
MbrManager::SetMfsInit(int arrayNum, bool value)
{
    int mfsInit = value == true ? 1 : 0;
    systeminfo.array_info[arrayNum].mfsInit = mfsInit;

    _WriteToDevices();
    return 0;
}

int
MbrManager::_CopyData(char* dest, string src, size_t len)
{
    if (src.length() + 1 >= len)
    {
        int ret = (int)IBOF_EVENT_ID::MBR_DATA_SIZE_ERROR;
        IBOF_TRACE_DEBUG(ret,
            "MBR Data {} with null character is longer than buffer size {}",
            src, len);
        return ret;
    }
    else
    {
        strncpy(dest, src.c_str(), src.length());
        dest[src.length()] = '\0';
        return 0;
    }
}

string
MbrManager::_ParseMbrData(void)
{
    string mbrData = "";
    string posVersion(systeminfo.pos_version);
    string uuid(systeminfo.system_uuid);

    mbrData += posVersion + " ";
    mbrData += to_string(systeminfo.mbr_version) + " ";
    mbrData += uuid + " ";

    for (unsigned int i = 0; i < systeminfo.arrayNum; i++)
    {
        string arrayName(systeminfo.array_info[i].array_name);
        string metaRaidType(systeminfo.array_info[i].meta_raid_type);
        string dataRaidType(systeminfo.array_info[i].data_raid_type);

        int totalDevNum = systeminfo.array_info[i].total_dev_num;
        int nvmNum = 0;
        int dataNum = 0;
        int spareNum = 0;

        mbrData += arrayName + " ";
        mbrData += metaRaidType + " ";
        mbrData += dataRaidType + " ";

        mbrData += to_string(nvmNum) + " ";
        mbrData += to_string(dataNum) + " ";
        mbrData += to_string(spareNum) + " ";

        for (int j = 0; j < totalDevNum; j++)
        {
            string uid(systeminfo.array_info[i].dev_info[j].device_uid);

            mbrData += to_string(systeminfo.array_info[i].dev_info[j].device_type) + " ";
            mbrData += uid + " ";
            mbrData += to_string(systeminfo.array_info[i].dev_info[j].device_state) + " ";
        }
    }

    return mbrData;
}

} // namespace ibofos
