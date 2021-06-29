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

#include "raid1.h"

#include <cassert>
#include <cstring>
#include <list>

#include "src/include/array_config.h"
#include "src/array_models/dto/partition_physical_size.h"

namespace pos
{
Raid1::Raid1(const PartitionPhysicalSize* physicalSize)
{
    raidType = RaidTypeEnum::RAID1;
    mirrorDevCnt = physicalSize->chunksPerStripe / 2;

    ftSize_ = {
        .minWriteBlkCnt = ArrayConfig::RAID1_MIN_WRITE_BLOCK_COUNT,
        .backupBlkCnt = mirrorDevCnt * physicalSize->blksPerChunk,
        .blksPerChunk = physicalSize->blksPerChunk,
        .blksPerStripe =
            physicalSize->chunksPerStripe * physicalSize->blksPerChunk,
        .chunksPerStripe = physicalSize->chunksPerStripe};
    _BindRecoverFunc();
}

int
Raid1::Translate(FtBlkAddr& dst, const LogicalBlkAddr& src)
{
    dst = {.stripeId = src.stripeId,
        .offset = src.offset};
    return 0;
}

int
Raid1::Convert(list<FtWriteEntry>& dst, const LogicalWriteEntry& src)
{
    FtWriteEntry ftEntry;
    Translate(ftEntry.addr, src.addr);
    ftEntry.buffers = *(src.buffers);
    ftEntry.blkCnt = src.blkCnt;

    FtWriteEntry mirror(ftEntry);
    mirror.addr.offset += ftSize_.backupBlkCnt;

    dst.clear();
    dst.push_back(ftEntry);
    dst.push_back(mirror);

    return 0;
}

list<FtBlkAddr>
Raid1::GetRebuildGroup(FtBlkAddr fba)
{
    uint32_t idx = fba.offset / ftSize_.blksPerChunk;
    uint32_t offset = fba.offset % ftSize_.blksPerChunk;
    uint32_t mirror = _GetMirrorIndex(idx);

    list<FtBlkAddr> recoveryGroup;
    fba.offset = mirror * ftSize_.blksPerChunk + offset;
    recoveryGroup.push_back(fba);
    return recoveryGroup;
}

void
Raid1::_BindRecoverFunc(void)
{
    using namespace std::placeholders;
    recoverFunc_ = bind(&Raid1::_RebuildData, this, _1, _2, _3);
}

void
Raid1::_RebuildData(void* dst, void* src, uint32_t size)
{
    memcpy(dst, src, size);
}

uint32_t
Raid1::_GetMirrorIndex(uint32_t idx)
{
    if (idx >= mirrorDevCnt)
    {
        return idx - mirrorDevCnt;
    }
    else
    {
        return idx + mirrorDevCnt;
    }
}

Raid1::~Raid1()
{
}

} // namespace pos
