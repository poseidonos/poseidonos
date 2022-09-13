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
#include "volume_service.h"

#include <string>

#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/sys_event/volume_event_publisher.h"

namespace pos
{
VolumeService::VolumeService(void)
{
    volumeManagerCnt = 0;

    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        items[i] = nullptr;
    }
}

VolumeService::~VolumeService(void)
{
    Clear();
}

void
VolumeService::Clear(void)
{
    std::unique_lock<std::mutex> lock(listMutex);
    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        if (items[i] != nullptr)
        {
            items[i] = nullptr;
        }
    }
    volumeManagerCnt = 0;
}

int
VolumeService::Register(int arrayId, IVolumeManager* volumeManager)
{
    if (arrayId < 0)
    {
        POS_TRACE_ERROR(9999, "Fail to Register Volume Manager for Array {}", volumeManager->GetArrayName());
        return -1;
    }

    std::unique_lock<std::mutex> lock(listMutex);

    if (items[arrayId] != nullptr)
    {
        POS_TRACE_ERROR(9999, "Volume manager for array {} already exists", volumeManager->GetArrayName());
        return -1;
    }

    items[arrayId] = volumeManager;
    volumeManagerCnt++;
    POS_TRACE_DEBUG(9999, "Volume manager for array {} is registered", volumeManager->GetArrayName());

    return 0;
}

void
VolumeService::Unregister(int arrayId)
{
    if (arrayId < 0 || arrayId >= ArrayMgmtPolicy::MAX_ARRAY_CNT)
    {
        POS_TRACE_ERROR(9999, "Volume manager for array {} does not exist", arrayId);
        return;
    }

    std::unique_lock<std::mutex> lock(listMutex);
    IVolumeManager* target = items[arrayId];
    if (target == nullptr)
    {
        POS_TRACE_ERROR(9999, "Volume manager for array {} does not exist", arrayId);
        return;
    }

    items[arrayId] = nullptr;
    volumeManagerCnt--;

    POS_TRACE_DEBUG(9999, "Volume manager for array {} is unregistered", arrayId);
}

IVolumeManager*
VolumeService::GetVolumeManager(int arrayId)
{
    IVolumeManager* volumeManager = nullptr;

    if (likely((0 <= arrayId) && (arrayId < ArrayMgmtPolicy::MAX_ARRAY_CNT)))
    {
        volumeManager = items[arrayId];
    }

    return volumeManager;
}


IVolumeManager*
VolumeService::GetVolumeManager(std::string arrayName)
{
    if (0 == volumeManagerCnt)
    {
        return nullptr;
    }

    for (int arrayId = 0 ; arrayId < ArrayMgmtPolicy::MAX_ARRAY_CNT; arrayId++)
    {
        if (items[arrayId] != nullptr)
        {
            if (items[arrayId]->GetArrayName() == arrayName)
            {
                return items[arrayId];
            }
        }
    }

    return nullptr;
}

} // namespace pos
