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

#include "space_info.h"

#include <string>

#include "src/array_mgmt/array_manager.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/include/array_config.h"
#include "src/logger/logger.h"
#include "src/volume/volume_service.h"

namespace pos
{
bool
SpaceInfo::IsEnough(uint32_t arrayId, uint64_t size)
{
    return Remaining(arrayId) >= size;
}

uint64_t
SpaceInfo::OPSize(uint32_t arrayId)
{
    uint64_t capa = RawCapacity(arrayId);
    return (uint64_t)(capa * ArrayConfig::OVER_PROVISIONING_RATIO / 100);
}

uint64_t
SpaceInfo::RawCapacity(uint32_t arrayId)
{
    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayId);
    if (info != nullptr)
    {
        const PartitionLogicalSize* ptnSize =
            info->arrayInfo->GetSizeInfo(PartitionType::USER_DATA);

        if (ptnSize != nullptr)
        {
            uint64_t dataBlks = ptnSize->blksPerStripe;
            return ptnSize->totalStripes * dataBlks * ArrayConfig::BLOCK_SIZE_BYTE;
        }
    }
    return 0;
}

uint64_t
SpaceInfo::TotalCapacity(uint32_t arrayId)
{
    uint64_t total = RawCapacity(arrayId);
    uint64_t op = OPSize(arrayId);
    POS_TRACE_INFO(9000, "TotalCapacity: total:{} - op:{} = sys:{} ",
        total, op, total - op);
    return total - op;
}

uint64_t
SpaceInfo::Used(uint32_t arrayId)
{
    uint64_t usedSize = 0;
    IVolumeInfoManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayId);

    if (volMgr != nullptr)
    {
        usedSize = volMgr->EntireVolumeSize();
    }
    return usedSize;
}

uint64_t
SpaceInfo::Remaining(uint32_t arrayId)
{
    return TotalCapacity(arrayId) - Used(arrayId);
}
} // namespace pos
