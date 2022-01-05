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

#include "raid10.h"
#include "src/helper/enumerable/query.h"
#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/array_models/dto/partition_physical_size.h"

namespace pos
{
Raid10::Raid10(const PartitionPhysicalSize* pSize)
: Method(RaidTypeEnum::RAID10)
{
    mirrorDevCnt = pSize->chunksPerStripe / 2;
    ftSize_ = {
        .minWriteBlkCnt = ArrayConfig::MIN_WRITE_BLOCK_COUNT,
        .backupBlkCnt = mirrorDevCnt * pSize->blksPerChunk,
        .blksPerChunk = pSize->blksPerChunk,
        .blksPerStripe = pSize->chunksPerStripe * pSize->blksPerChunk,
        .chunksPerStripe = pSize->chunksPerStripe};
    _BindRecoverFunc();
}

int
Raid10::Translate(FtBlkAddr& dst, const LogicalBlkAddr& src)
{
    dst = {.stripeId = src.stripeId,
        .offset = src.offset};
    return 0;
}

int
Raid10::Convert(list<FtWriteEntry>& dst, const LogicalWriteEntry& src)
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
Raid10::GetRebuildGroup(FtBlkAddr fba)
{
    uint32_t idx = fba.offset / ftSize_.blksPerChunk;
    uint32_t offset = fba.offset % ftSize_.blksPerChunk;
    uint32_t mirror = _GetMirrorIndex(idx);

    list<FtBlkAddr> recoveryGroup;
    fba.offset = mirror * ftSize_.blksPerChunk + offset;
    recoveryGroup.push_back(fba);
    return recoveryGroup;
}

RaidState
Raid10::GetRaidState(vector<ArrayDeviceState> devs)
{
    RaidState rs = RaidState::NORMAL;
    for (size_t i = 0; i < devs.size(); i++)
    {
        ArrayDeviceState state = devs[i];
        if (state != ArrayDeviceState::NORMAL)
        {
            rs = RaidState::DEGRADED;
            ArrayDeviceState mirrorState = devs[_GetMirrorIndex(i)];
            if (mirrorState != ArrayDeviceState::NORMAL)
            {
                return RaidState::FAILURE;
            }
        }
    }
    POS_TRACE_INFO(EID(RAID_DEBUG_MSG), "GetRaidState from raid10:{} ", rs);
    return rs;
}

vector<uint32_t>
Raid10::GetParityOffset(StripeId lsid)
{
    vector<uint32_t> parityIdx;
    for (uint32_t i = mirrorDevCnt; i < mirrorDevCnt * 2 ; i++)
    {
        parityIdx.push_back(i);
    }
    return parityIdx;
}

void
Raid10::_BindRecoverFunc(void)
{
    using namespace std::placeholders;
    recoverFunc = bind(&Raid10::_RebuildData, this, _1, _2, _3);
}

void
Raid10::_RebuildData(void* dst, void* src, uint32_t size)
{
    memcpy(dst, src, size);
}

uint32_t
Raid10::_GetMirrorIndex(uint32_t idx)
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

Raid10::~Raid10()
{
}

} // namespace pos
