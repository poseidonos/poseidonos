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

#include "abr_manager.h"

#include <list>

#include "src/array/device/array_device_list.h"
#include "src/helper/time/time_helper.h"
#include "src/include/array_device_state.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/mbr/mbr_manager.h"
#include "src/mbr/mbr_util.h"

namespace pos
{
AbrManager::AbrManager(void)
: AbrManager(new MbrManager())
{
}

AbrManager::AbrManager(MbrManager* mbrMgr)
: mbrManager(mbrMgr)
{
}

AbrManager::~AbrManager(void)
{
    delete mbrManager;
}

int
AbrManager::LoadAbr(ArrayMeta& meta)
{
    struct ArrayBootRecord* abr = nullptr;
    unsigned int arrayIndex = -1;
    mbrManager->GetAbr(meta.arrayName, &abr, arrayIndex);
    if (abr == nullptr)
    {
        int result = EID(MBR_ABR_NOT_FOUND);
        POS_TRACE_WARN(result, "No array found with arrayName :{}", meta.arrayName);
        return result;
    }
    meta.id = arrayIndex;
    meta.arrayName = abr->arrayName;
    meta.dataRaidType = abr->dataRaidType;
    meta.metaRaidType = abr->metaRaidType;
    int nvmNum = abr->nvmDevNum;
    int dataNum = abr->dataDevNum;
    int spareNum = abr->spareDevNum;

    int devIndex = 0;

    for (int i = 0; i < nvmNum; i++)
    {
        devIndex = i;
        string uid(abr->devInfo[devIndex].deviceUid);
        meta.devs.nvm.push_back(DeviceMeta(uid));
    }

    for (int i = 0; i < dataNum; i++)
    {
        devIndex = nvmNum + i;
        string uid(abr->devInfo[devIndex].deviceUid);
        int state = abr->devInfo[devIndex].deviceState;
        DeviceMeta deviceMeta(uid, static_cast<ArrayDeviceState>(state));
        meta.devs.data.push_back(deviceMeta);
    }

    for (int i = 0; i < spareNum; i++)
    {
        devIndex = nvmNum + dataNum + i;
        string uid(abr->devInfo[devIndex].deviceUid);
        meta.devs.spares.push_back(DeviceMeta(uid));
    }

    meta.createDatetime = abr->createDatetime;
    meta.updateDatetime = abr->updateDatetime;
    meta.unique_id = abr->uniqueId;
    return 0;
}

int
AbrManager::SaveAbr(ArrayMeta& meta)
{
    struct ArrayBootRecord* abr = nullptr;
    unsigned int arrayIndex;
    mbrManager->GetAbr(meta.arrayName, &abr, arrayIndex);
    if (abr == nullptr)
    {
        int ret = EID(MBR_ABR_NOT_FOUND);
        POS_TRACE_ERROR(ret, "Cannot save abr, abr not found");
        return ret;
    }

    int nvmNum = meta.devs.nvm.size();
    int dataNum = meta.devs.data.size();
    int spareNum = meta.devs.spares.size();

    abr->nvmDevNum = nvmNum;
    abr->dataDevNum = dataNum;
    abr->spareDevNum = spareNum;
    abr->totalDevNum = nvmNum + dataNum + spareNum;

    CopyData(abr->arrayName, meta.arrayName, ARRAY_NAME_SIZE);
    CopyData(abr->metaRaidType, meta.metaRaidType, META_RAID_TYPE_SIZE);
    CopyData(abr->dataRaidType, meta.dataRaidType, DATA_RAID_TYPE_SIZE);
    CopyData(abr->updateDatetime, Time::GetCurrentTimeStr("%Y-%m-%d %X %z", DATE_SIZE), DATE_SIZE);

    for (int i = 0; i < nvmNum; i++)
    {
        int deviceIndex = i;
        abr->devInfo[deviceIndex].deviceType =
            (int)ArrayDeviceType::NVM;
        CopyData(abr->devInfo[deviceIndex].deviceUid,
            meta.devs.nvm.at(i).uid, DEVICE_UID_SIZE);
    }

    for (int i = 0; i < dataNum; i++)
    {
        int deviceIndex = nvmNum + i;
        abr->devInfo[deviceIndex].deviceType =
            (int)ArrayDeviceType::DATA;
        CopyData(abr->devInfo[deviceIndex].deviceUid,
            meta.devs.data.at(i).uid, DEVICE_UID_SIZE);
        abr->devInfo[deviceIndex].deviceState =
            (int)meta.devs.data.at(i).state;
    }

    for (int i = 0; i < spareNum; i++)
    {
        int deviceIndex = nvmNum + dataNum + i;
        abr->devInfo[deviceIndex].deviceType =
            (int)ArrayDeviceType::SPARE;
        CopyData(abr->devInfo[deviceIndex].deviceUid,
            meta.devs.spares.at(i).uid, DEVICE_UID_SIZE);
    }

    meta.createDatetime = abr->createDatetime;
    meta.updateDatetime = abr->updateDatetime;
    mbrManager->UpdateDeviceIndexMap(meta.arrayName);

    int result = mbrManager->SaveMbr();
    return result;
}

int
AbrManager::CreateAbr(ArrayMeta& meta)
{
    return mbrManager->CreateAbr(meta);
}

int
AbrManager::DeleteAbr(string arrayName)
{
    return mbrManager->DeleteAbr(arrayName);
}

int
AbrManager::ResetMbr(void)
{
    return mbrManager->ResetMbr();
}

int
AbrManager::GetAbrList(std::vector<ArrayBootRecord>& abrList)
{
    int result = mbrManager->LoadMbr();
    if (result != 0)
    {
        return result;
    }
    
    result = mbrManager->GetAbrList(abrList);
    return result;
}

string
AbrManager::GetLastUpdatedDateTime(string arrayName)
{
    struct ArrayBootRecord* abr = nullptr;
    unsigned int arrayIndex;
    mbrManager->GetAbr(arrayName, &abr, arrayIndex);
    if (abr == nullptr)
    {
        POS_TRACE_ERROR(EID(MBR_ABR_NOT_FOUND), "Cannot get Abr for {}", arrayName);
        return "";
    }
    return abr->updateDatetime;
}

string
AbrManager::GetCreatedDateTime(string arrayName)
{
    struct ArrayBootRecord* abr = nullptr;
    unsigned int arrayIndex;
    mbrManager->GetAbr(arrayName, &abr, arrayIndex);
    if (abr == nullptr)
    {
        POS_TRACE_ERROR(EID(MBR_ABR_NOT_FOUND), "Cannot get Abr for {}", arrayName);
        return "";
    }
    return abr->createDatetime;
}

string
AbrManager::FindArrayWithDeviceSN(string devSN)
{
    string arrayName = mbrManager->FindArrayWithDeviceSN(devSN);

    return arrayName;
}

void
AbrManager::InitDisk(UblockSharedPtr dev)
{
    mbrManager->InitDisk(dev);
}

} // namespace pos
