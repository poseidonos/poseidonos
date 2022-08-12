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

#include "mbr_map_manager.h"

#include <utility>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
MbrMapManager::MbrMapManager(void)
{
}

MbrMapManager::~MbrMapManager(void)
{
}

int
MbrMapManager::InsertDevices(ArrayMeta& meta, unsigned int arrayIndex)
{
    auto insertNum = 0;
    for (auto dev : meta.devs.nvm)
    {
        arrayDeviceIndexMap.insert(pair<string, unsigned int>(dev.uid, arrayIndex));
        POS_TRACE_DEBUG(EID(MBR_DEBUG_MSG),
            "Inserted {} to array {}", dev.uid, arrayIndex);
        insertNum++;
    }

    for (auto dev : meta.devs.data)
    {
        arrayDeviceIndexMap.insert(pair<string, unsigned int>(dev.uid, arrayIndex));
        POS_TRACE_DEBUG(EID(MBR_DEBUG_MSG),
            "Inserted {} to array {}", dev.uid, arrayIndex);
        insertNum++;
    }

    for (auto dev : meta.devs.spares)
    {
        arrayDeviceIndexMap.insert(pair<string, unsigned int>(dev.uid, arrayIndex));
        POS_TRACE_DEBUG(EID(MBR_DEBUG_MSG),
            "Inserted {} to array {}", dev.uid, arrayIndex);
        insertNum++;
    }

    POS_TRACE_DEBUG(EID(MBR_DEBUG_MSG),
        "Inserted {} devices to arrayDeviceMap", insertNum);

    return 0;
}

int
MbrMapManager::InsertDevice(string deviceUid, unsigned int arrayIndex)
{
    arrayDeviceIndexMap.insert(pair<string, unsigned int>(deviceUid, arrayIndex));
    return 0;
}

int
MbrMapManager::DeleteDevices(unsigned int arrayIndex)
{
    arrayDeviceIndexMapIter devIter;
    auto deleteNum = 0;
    for (devIter = arrayDeviceIndexMap.begin(); devIter !=
         arrayDeviceIndexMap.end();)
    {
        if (devIter->second == arrayIndex)
        {
            POS_TRACE_DEBUG(EID(MBR_DEBUG_MSG),
                "Deleted {} from array {}", devIter->first, devIter->second);
            arrayDeviceIndexMap.erase(devIter++);
            deleteNum++;
        }
        else
        {
            ++devIter;
        }
    }

    POS_TRACE_DEBUG(EID(MBR_DEBUG_MSG),
        "Deleted {} devices from arrayDeviceMap", deleteNum);

    return 0;
}

int
MbrMapManager::CheckAllDevices(ArrayMeta& meta)
{
    int result;
    result = _CheckDevices(meta.devs.nvm);
    if (result != 0)
    {
        return result;
    }

    result = _CheckDevices(meta.devs.data);
    if (result != 0)
    {
        return result;
    }

    result = _CheckDevices(meta.devs.spares);
    if (result != 0)
    {
        return result;
    }

    return 0;
}

int
MbrMapManager::_CheckDevices(vector<DeviceMeta>& devs)
{
    for (auto dev : devs)
    {
        auto iter = arrayDeviceIndexMap.find(dev.uid);
        if (iter != arrayDeviceIndexMap.end())
        {
            int event_id = EID(MBR_DEVICE_ALREADY_IN_ARRAY);
            POS_TRACE_WARN(event_id, "device_uid: {}, array: {}", dev.uid, iter->second);
            return event_id;
        }
    }
    return 0;
}

int
MbrMapManager::ResetMap(void)
{
    arrayDeviceIndexMap.clear();
    return 0;
}

int
MbrMapManager::FindArrayIndex(string devSN)
{
    arrayDeviceIndexMapIter devIter;
    for (devIter = arrayDeviceIndexMap.begin();
            devIter != arrayDeviceIndexMap.end();)
    {
        if (devIter->first == devSN)
        {
            return devIter->second;
        }
        else
        {
            ++devIter;
        }
    }

    return -1;
}

} // namespace pos
