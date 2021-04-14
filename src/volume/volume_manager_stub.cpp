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

#include "src/include/pos_event_id.h"
#include "src/include/memory.h"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume_manager.h"

namespace pos
{

VolumeManager::VolumeManager(IArrayInfo* i, IStateControl* s)
:arrayInfo(i),
state(s)
{
}

VolumeManager::~VolumeManager(void)
{
}

int
VolumeManager::Init(void)
{
    return 0;
}

void
VolumeManager::Dispose(void)
{
}

uint64_t
VolumeManager::EntireVolumeSize()
{
    return 0;
}

int
VolumeManager::GetVolumeSize(int volId, uint64_t& volSize)
{
    volSize = 0;
    return (int)POS_EVENT_ID::SUCCESS;
}

int
VolumeManager::Create(std::string name, uint64_t size, uint64_t maxiops, uint64_t maxbw)
{
    return (int)POS_EVENT_ID::SUCCESS;
}

int
VolumeManager::UpdateQoS(std::string name, uint64_t maxiops, uint64_t maxbw)
{
    return (int)POS_EVENT_ID::SUCCESS;
}

int
VolumeManager::Rename(std::string oldname, std::string newname)
{
    return (int)POS_EVENT_ID::SUCCESS;
}

int
VolumeManager::Resize(std::string name, uint64_t newsize)
{
    return (int)POS_EVENT_ID::SUCCESS;
}

int
VolumeManager::Delete(std::string name)
{
    return (int)POS_EVENT_ID::SUCCESS;
}

int
VolumeManager::GetVolumeStatus(int volId)
{
    return 0;
}

int
VolumeManager::Mount(std::string name, std::string subnqn)
{
    return (int)POS_EVENT_ID::SUCCESS;
}

int
VolumeManager::Unmount(std::string name)
{
    return (int)POS_EVENT_ID::SUCCESS;
}

void
VolumeManager::DetachVolumes()
{
}

int
VolumeManager::VolumeName(int volId, std::string& volName)
{
    volName = "";
    return (int)POS_EVENT_ID::SUCCESS;
}

int
VolumeManager::VolumeID(std::string volName)
{
    return 0;
}

int
VolumeManager::GetVolumeCount()
{
    return 0;
}

VolumeBase*
VolumeManager::GetVolume(int volId)
{
    return nullptr;
}

void
VolumeManager::StateChanged(StateContext* prev, StateContext* next)
{
}

bool
VolumeManager::CheckVolumeIdle(int volId)
{
    return true;
}

VolumeList*
VolumeManager::GetVolumeList(void)
{
    return nullptr;
}

std::string
VolumeManager::GetStatusStr(VolumeStatus status)
{
    std::string str{""};
    return str;
}

int
VolumeManager::IncreasePendingIOCount(int volId, uint32_t ioCountToSubmit)
{
    return 0;
}

int
VolumeManager::DecreasePendingIOCount(int volId, uint32_t ioCountCompleted)
{
    return 0;
}

} // namespace pos
