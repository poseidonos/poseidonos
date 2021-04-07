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

#include "nvram_meta_vol.h"

#include "mfs_log.h"

NvRamMetaVolumeClass::NvRamMetaVolumeClass(MetaLpnType maxVolumePageNum)
: MetaVolumeClass(MetaVolumeType::NvRamVolume, maxVolumePageNum)
{
}

NvRamMetaVolumeClass::~NvRamMetaVolumeClass(void)
{
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NvRAM Meta Vol. destructed...");
}

bool
NvRamMetaVolumeClass::IsFreeSpaceEnough(FileSizeType fileByteSize)
{
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "NVRAM Space. free space={}, request={}", fileByteSize, GetTheBiggestExtentSize());

    if (fileByteSize == 0) // MetaFsMoMReqType::GetFreeFileRegionSize
    {
        return true;
    }
    if (GetTheBiggestExtentSize() > fileByteSize)
    {
        return true;
    }

    return false;
}

void
NvRamMetaVolumeClass::InitVolumeBaseLpn(void)
{
    volumeBaseLpn = NVRAM_VOLUME_BASE_LPN;
}

bool
NvRamMetaVolumeClass::IsOkayToStore(FileSizeType fileByteSize, MetaFilePropertySet& prop)
{
    if (GetUtilizationInPercent() >= META_VOL_CAPACITY_FULL_LIMIT_IN_PERCENT)
    {
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_META_VOLUME_ALMOST_FULL,
            "NVRAM Volume is almost full");
        return false;
    }

    if (true == IsFreeSpaceEnough(fileByteSize))
    {
        if (_IsNVRAMStoreStronglyPreferred(prop))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

bool
NvRamMetaVolumeClass::_IsNVRAMStoreStronglyPreferred(MetaFilePropertySet& prop)
{
    if (prop.ioAccPattern == MDFilePropIoAccessPattern::ByteIntensive ||
        prop.ioAccPattern == MDFilePropIoAccessPattern::SmallSizeBlockIO)
    {
        return true;
    }

    if (prop.ioOpType == MDFilePropIoOpType::WriteDominant)
    {
        return true;
    }

    return false;
}

// bool
// NvRamMetaVolumeClass::_IsByteStoreUnderUtilized(void)
// {
//     uint32_t utilizationInPercent = GetUtilizationInPercent();

//     // if volume capacity is utilized in 90%, then we say it is underutilized
//     return (utilizationInPercent < 90) ? true : false;
// }
