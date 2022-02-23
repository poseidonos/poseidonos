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

#pragma once

#include <string>

#include "src/volume/volume_list.h"
#include "src/volume/volume_base.h"
#include "src/qos/qos_common.h"

namespace pos
{
class VolumeBase;

class IVolumeManager
{
public:
    virtual int Create(std::string name, uint64_t size, uint64_t maxiops, uint64_t maxbw) = 0;
    virtual int Delete(std::string name) = 0;
    virtual int Mount(std::string name, std::string subnqn) = 0;
    virtual int Unmount(std::string name) = 0;
    virtual int UpdateQoS(std::string name, uint64_t maxiops, uint64_t maxbw, uint64_t miniops, uint64_t minbw) = 0;
    virtual int Rename(std::string oldname, std::string newname) = 0;
    virtual void DetachVolumes(void) = 0;

    virtual int VolumeName(int volId, std::string& volName) = 0;
    virtual int VolumeID(std::string volName) = 0;
    virtual int GetVolumeCount(void) = 0;
    virtual int GetVolumeStatus(int volId) = 0;
    virtual int CheckVolumeValidity(int volId) = 0;
    virtual uint64_t EntireVolumeSize(void) = 0;
    virtual int GetVolumeSize(int volId, uint64_t& volSize) = 0;
    virtual VolumeList* GetVolumeList(void) = 0;
    virtual std::string GetStatusStr(VolumeStatus status) = 0;

    virtual int IncreasePendingIOCountIfNotZero(int volId, VolumeStatus mounted = VolumeStatus::Mounted, uint32_t ioCountToSubmit = 1) = 0;
    virtual int DecreasePendingIOCount(int volId, VolumeStatus mounted = VolumeStatus::Mounted, uint32_t ioCountCompleted = 1) = 0;
    virtual VolumeBase* GetVolume(int volId) = 0;

    virtual std::string GetArrayName(void) = 0;
};

} // namespace pos
