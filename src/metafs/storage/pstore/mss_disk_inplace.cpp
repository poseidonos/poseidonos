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

#include "mss_disk_inplace.h"
#include "src/array_mgmt/array_manager.h"

#include <string>

namespace pos
{
MssDiskInplace::MssDiskInplace(void)
{
    mediaType = MetaStorageType::Max;
    partitionType = PartitionType::TYPE_COUNT;
    array = nullptr;
}

MssDiskInplace::MssDiskInplace(int arrayId, MetaStorageType mediaType, uint64_t capacity)
{
    this->mediaType = mediaType;
    switch (mediaType)
    {
        case MetaStorageType::NVRAM:
            partitionType = PartitionType::META_NVM;
            break;
        case MetaStorageType::JOURNAL_SSD:
            partitionType = PartitionType::JOURNAL_SSD;
            break;
        default:
            partitionType = PartitionType::META_SSD;
            break;
    }

    array = pos::ArrayMgr()->GetInfo(arrayId)->arrayInfo;
}

MssDiskInplace::~MssDiskInplace(void)
{
}

uint32_t
MssDiskInplace::GetMaxLpnCntPerIOSubmit(void)
{
    return array->GetSizeInfo(partitionType)->blksPerStripe;
}
/**
 * Calculate on disk address for given LPN.
 * This is for inplace update of data. So static mapping.
 *
 * @metaLpn page number to mapped to device.
 *
 * @return physical LBA and device
 */
pos::LogicalBlkAddr
MssDiskInplace::CalculateOnDiskAddress(uint64_t pageNumber)
{
    pos::LogicalBlkAddr logicalAddr;
    uint32_t blksPerStripe = GetMaxLpnCntPerIOSubmit();
    logicalAddr.stripeId = pageNumber / blksPerStripe; // calculate stripe id
    logicalAddr.offset = pageNumber % blksPerStripe;   // calculate offset in stripe

    return logicalAddr;
}
} // namespace pos
