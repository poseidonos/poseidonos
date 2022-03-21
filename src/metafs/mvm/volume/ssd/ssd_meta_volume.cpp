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

#include "ssd_meta_volume.h"

#include <string>

#include "src/logger/logger.h"
namespace pos
{
SsdMetaVolume::SsdMetaVolume(int arrayId, MetaVolumeType volType, MetaLpnType maxVolumePageNum,
    InodeManager* inodeMgr, CatalogManager* catalogMgr)
: MetaVolume(arrayId, volType, maxVolumePageNum, inodeMgr, catalogMgr)
{
}

SsdMetaVolume::~SsdMetaVolume(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "SSD Meta Vol. destructed...");
}

void
SsdMetaVolume::InitVolumeBaseLpn(void)
{
    volumeBaseLpn_ = SSD_VOLUME_BASE_LPN;
}

bool
SsdMetaVolume::IsFreeSpaceEnough(FileSizeType fileByteSize)
{
    if (GetAvailableSpace() >= fileByteSize)
    {
        return true;
    }

    MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_META_VOLUME_NOT_ENOUGH_SPACE,
        "Not Enough SSD Meta Space, freeSpace={}, reqSize={}", GetAvailableSpace(), fileByteSize);
    return false;
}

bool
SsdMetaVolume::IsOkayToStore(FileSizeType fileByteSize, MetaFilePropertySet& prop)
{
    if (GetUtilizationInPercent() >= META_VOL_CAPACITY_FULL_LIMIT_IN_PERCENT)
    {
        MFS_TRACE_WARN((int)POS_EVENT_ID::MFS_META_VOLUME_ALMOST_FULL,
            "Volume is almost full,");
        return false;
    }

    if (IsFreeSpaceEnough(fileByteSize) == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}
} // namespace pos
