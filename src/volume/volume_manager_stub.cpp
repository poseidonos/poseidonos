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

#include "src/include/ibof_event_id.h"
#include "src/include/memory.h"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume_manager.h"

namespace ibofos
{
void
VolumeManager::Initialize(void)
{
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
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeManager::Create(string name, uint64_t size, string array, uint64_t maxiops, uint64_t maxbw)
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeManager::_CheckVolumeSize(uint64_t volSize)
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeManager::UpdateQoS(string name, string array, uint64_t maxiops, uint64_t maxbw)
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeManager::_SetVolumeQoS(VolumeBase* vol, uint64_t maxiops, uint64_t maxbw)
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeManager::Rename(string oldname, string newname, string array)
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeManager::Resize(string name, string array, uint64_t newsize)
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeManager::Delete(string name, string array)
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeManager::GetVolumeStatus(int volId)
{
    return 0;
}

int
VolumeManager::_LoadVolumes(string array)
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeManager::SaveVolumes()
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeManager::Mount(string name, string array, string subnqn)
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeManager::Unmount(string name, string array)
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

int
VolumeManager::MountAll()
{
    return (int)IBOF_EVENT_ID::SUCCESS;
}

void
VolumeManager::DetachVolumes()
{
}

int
VolumeManager::VolumeName(int volId, std::string& volName)
{
    volName = "";
    return (int)IBOF_EVENT_ID::SUCCESS;
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
VolumeManager::StateChanged(StateContext prev, StateContext next)
{
}

} // namespace ibofos
