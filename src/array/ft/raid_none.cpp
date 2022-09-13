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

#include "raid_none.h"
#include "src/helper/enumerable/query.h"
#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/array_models/dto/partition_physical_size.h"

namespace pos
{
RaidNone::RaidNone(const PartitionPhysicalSize* pSize)
: Method(RaidTypeEnum::NONE)
{
    ftSize_ = {
        .minWriteBlkCnt = ArrayConfig::MIN_WRITE_BLOCK_COUNT,
        .backupBlkCnt = 0,
        .blksPerChunk = pSize->blksPerChunk,
        .blksPerStripe = pSize->blksPerChunk,
        .chunksPerStripe = 1};
}

list<FtEntry>
RaidNone::Translate(const LogicalEntry& le)
{
    FtEntry fe;
    fe.addr.stripeId = le.addr.stripeId;
    fe.addr.offset = le.addr.offset;
    fe.blkCnt = le.blkCnt;

    return list<FtEntry> { fe };
}

int
RaidNone::MakeParity(list<FtWriteEntry>& ftl, const LogicalWriteEntry& src)
{
    ftl.clear();
    return 0;
}

RaidState
RaidNone::GetRaidState(const vector<ArrayDeviceState>& devs)
{
    auto&& abnormalDevs = Enumerable::Where(devs,
        [](auto d) { return d != ArrayDeviceState::NORMAL; });

    if (abnormalDevs.size() == 0)
    {
        return RaidState::NORMAL;
    }
    return RaidState::FAILURE;
}

bool
RaidNone::CheckNumofDevsToConfigure(uint32_t numofDevs)
{
    uint32_t requiredNumofDevsforRAIDNone = 1;
    return numofDevs == requiredNumofDevsforRAIDNone;
}

RaidNone::~RaidNone()
{
}

} // namespace pos
