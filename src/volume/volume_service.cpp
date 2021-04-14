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

#include "i_volume_manager.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/volume/volume_service.h"

namespace pos
{
VolumeService::VolumeService(void)
{
}

VolumeService::~VolumeService(void)
{
}

IVolumeManager*
VolumeService::_Find(std::string arrayName)
{
    // TODO (huijeong.kim) This's temporal workaround for current single array
    // Current Write handler in single array uses "" for arrayName
    if (arrayName == "" && volumeManager.size() == 1)
    {
        return volumeManager.begin()->second;
    }

    auto found = volumeManager.find(arrayName);
    if (found == volumeManager.end())
    {
        return nullptr;
    }
    else
    {
        return found->second;
    }
}

void
VolumeService::Register(std::string arrayName, IVolumeManager* writer)
{
    if (volumeManager.find(arrayName) == volumeManager.end())
    {
        volumeManager.emplace(arrayName, writer);
        POS_TRACE_DEBUG(9999,
            "Volume manager for array {} is registered", arrayName);
    }
    else
    {
        POS_TRACE_ERROR(9999,
            "Volume manager for array {} already exists", arrayName);
    }
}

void
VolumeService::Unregister(std::string arrayName)
{
    if (volumeManager.find(arrayName) != volumeManager.end())
    {
        volumeManager.erase(arrayName);
        POS_TRACE_DEBUG(9999,
            "Volume manager for array {} is unregistered", arrayName);
    }
    else
    {
        POS_TRACE_ERROR(9999,
            "Volume manager for array {} does not exist", arrayName);
    }
}

IVolumeManager*
VolumeService::GetVolumeManager(std::string arrayName)
{
    return _Find(arrayName);
}

} // namespace pos
