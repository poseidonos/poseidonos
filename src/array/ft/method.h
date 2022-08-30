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

#pragma once

#include <list>
#include <vector>
#include <utility>

#include "src/include/address_type.h"
#include "src/include/recover_func.h"
#include "src/include/raid_type.h"
#include "src/include/raid_state.h"
#include "src/include/array_device_state.h"
#include "src/array_models/dto/partition_physical_size.h"

using namespace std;

namespace pos
{
struct FtSizeInfo
{
    uint32_t minWriteBlkCnt;
    uint32_t backupBlkCnt;
    uint32_t blksPerChunk;
    uint32_t blksPerStripe;
    uint32_t chunksPerStripe;
};

class Method
{
public:
    explicit Method(RaidTypeEnum rt)
    : raidType(rt)
    {
    }
// LCOV_EXCL_START
    virtual ~Method()
    {
    }
// LCOV_EXCL_STOP

    const FtSizeInfo* GetSizeInfo(void) { return &ftSize_; }
    virtual list<FtEntry> Translate(const LogicalEntry& le) = 0;
    virtual int MakeParity(list<FtWriteEntry>& ftl, const LogicalWriteEntry& src) = 0;
    virtual RaidState GetRaidState(const vector<ArrayDeviceState>& devs) = 0;
    virtual bool CheckNumofDevsToConfigure(uint32_t numofDevs) = 0;
    RaidTypeEnum GetRaidType(void) { return raidType; }
    virtual RecoverFunc GetRecoverFunc(vector<uint32_t> targets, vector<uint32_t> abnormals) { return nullptr; }
    virtual list<FtBlkAddr> GetRebuildGroup(FtBlkAddr fba, const vector<uint32_t>& abnormals) { return list<FtBlkAddr>(); }
    virtual vector<uint32_t> GetParityOffset(StripeId lsid) { return vector<uint32_t>(); }
    virtual bool IsRecoverable(void) { return true; }
    virtual vector<pair<vector<uint32_t>, vector<uint32_t>>> GetRebuildGroupPairs(vector<uint32_t>& targetIndexs)
    {
        return vector<pair<vector<uint32_t>, vector<uint32_t>>>();
    }

protected:
    FtSizeInfo ftSize_ = {
        0,
    };
    RaidTypeEnum raidType;
};

} // namespace pos
