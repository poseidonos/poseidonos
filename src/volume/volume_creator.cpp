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

#include <string>
#include "src/volume/volume_creator.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/network/nvmf_target.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume.h"
#include "src/volume/volume_list.h"
#include "src/qos/qos_manager.h"

namespace pos
{

VolumeCreator::VolumeCreator(VolumeList& volumeList, std::string arrayName, int arrayID, VolumeEventPublisher* volumeEventPublisher)
: VolumeInterface(volumeList, arrayName, arrayID, volumeEventPublisher),
  VolumeNamePolicy(EID(CREATE_VOL_NAME_INCLUDES_SPECIAL_CHAR), EID(CREATE_VOL_NAME_START_OR_END_WITH_SPACE),
    EID(CREATE_VOL_NAME_TOO_LONG), EID(CREATE_VOL_NAME_TOO_SHORT))
{
    vol = nullptr;
}

VolumeCreator::~VolumeCreator(void)
{
}

void
VolumeCreator::_CheckQosValidity(uint64_t maxIops,
        uint64_t maxBw, uint64_t minIops, uint64_t minBw, std::string volName, uint32_t volId)
{
    std::string errorMsg = "";
    std::vector<std::pair<string, uint32_t>> volumeInput;
    volumeInput.push_back(std::make_pair(volName, volId));
    int ret = QosManagerSingleton::Instance()->UpdateVolumePolicy(minBw, maxBw,
        minIops, maxIops, errorMsg, volumeInput, arrayName);
    if (ret != 0)
    {
        throw ret;
    }
}

void
VolumeCreator::_CheckRequestValidity(string name, uint64_t size)
{
    CheckVolumeName(name);

    if (volumeList.GetID(name) >= 0)
    {
        POS_TRACE_WARN(EID(CREATE_VOL_SAME_VOL_NAME_EXISTS), "vol_name: {}", name);
        throw EID(CREATE_VOL_SAME_VOL_NAME_EXISTS);
    }
    _CheckVolumeSize(size);
}

void
VolumeCreator::_CreateVolume(string name, uint64_t size, uint64_t maxIops,
        uint64_t maxBw, uint64_t minIops, uint64_t minBw, bool checkWalVolume, std::string uuid)
{
    VolumeAttribute volumeAttribute = (checkWalVolume ? VolumeAttribute::HAJournalData : VolumeAttribute::UserData);

    vol = new Volume(arrayName, arrayID, name, uuid, size, maxIops, minIops, maxBw, minBw, volumeAttribute);
    if (vol == nullptr)
    {
        POS_TRACE_ERROR(EID(CREATE_VOL_MEM_ALLOC_FAIL), "Fail to allocate memory");
        throw EID(CREATE_VOL_MEM_ALLOC_FAIL);
    }

    int ret = volumeList.Add(vol);
    if (ret != EID(SUCCESS))
    {
        throw ret;
    }
}

void
VolumeCreator::_NotificationVolumeEvent()
{
    _SetVolumeEventBase(vol);
    _SetVolumeEventPerf(vol);
    _SetVolumeArrayInfo();

    bool res = eventPublisher->NotifyVolumeCreated(&volumeEventBase, &volumeEventPerf, &volumeArrayInfo);

    if (res == false)
    {
        POS_TRACE_WARN(EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED),
            "array_name: {}", arrayName);
        throw EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED);
    }
}

void
VolumeCreator::_SetUuid()
{
    NvmfTarget nvmfTarget;

    std::string uuid = nvmfTarget.GetPosBdevUuid(vol->ID, vol->GetArrayName());
    vol->SetUuid(uuid);
}

void
VolumeCreator::_RollbackCreatedVolume(int exceptionEvent)
{
    // [To do] cancel notificaiton
    try
    {
    if (vol != nullptr)
    {
        volumeList.Remove(vol->ID);
    }
    }
    catch(int& exceptionEvent)
    {
        POS_TRACE_ERROR(EID(VOL_EVENT_ROLLBACK_FAIL), "event_id: {}", exceptionEvent);
        if (vol != nullptr)
        {
            delete vol;
        }
    }
}

int
VolumeCreator::Do(string name, uint64_t size, uint64_t maxIops, uint64_t maxBw,
        uint64_t minIops, uint64_t minBw, std::string uuid, bool checkWalVolume)
{
    try
    {
        _CheckRequestValidity(name, size);
        _CreateVolume(name, size, maxIops, maxBw, minIops, minBw, checkWalVolume, uuid);
        _CheckQosValidity(maxIops, maxBw, minIops, minBw, name, vol->ID);
        _NotificationVolumeEvent();

        _SetUuid();

        int ret = _SaveVolumes();
        if (ret != EID(SUCCESS))
        {
            throw ret;
        }

        _PrintLogVolumeQos(vol, 0, 0, 0, 0);
    }
    catch (int& exceptionEvent)
    {
        _RollbackCreatedVolume(exceptionEvent);

        return exceptionEvent;
    }

    return EID(SUCCESS);
}

} // namespace pos
