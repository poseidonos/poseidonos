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
#include <memory>
#include <string>
#include <vector>

#include "partition.h"
#include "src/array/rebuild/rebuild_target.h"
#include "src/array/service/io_recover/i_recover.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/array/ft/method.h"

using namespace std;

namespace pos
{

class StripePartition : public Partition, public IRecover, public RebuildTarget
{
    friend class ParityLocationWbtCommand;

public:
    StripePartition(PartitionType type,
                    vector<ArrayDevice *> devs,
                    RaidTypeEnum raid);
    virtual ~StripePartition(void);
    virtual int Create(uint64_t startLba, uint32_t segCnt, uint64_t totalNvmBlks);
    void RegisterService(IPartitionServices* svc) override;
    int Translate(list<PhysicalEntry>& pel, const LogicalEntry& le) override;
    int GetParityList(list<PhysicalWriteEntry>& parity, const LogicalWriteEntry& src) override;
    int ByteTranslate(PhysicalByteAddr& dst, const LogicalByteAddr& src) override;
    int ByteConvert(list<PhysicalByteWriteEntry> &dst, const LogicalByteWriteEntry &src) override;
    bool IsByteAccessSupported(void) override;
    RaidState GetRaidState(void) override;
    int GetRecoverMethod(UbioSmartPtr ubio, RecoverMethod& out) override;
    unique_ptr<RebuildContext> GetRebuildCtx(ArrayDevice* fault) override;
    Method* GetMethod(void) { return method; }
    RaidTypeEnum GetRaidType(void) override { return raidType; }

private:
    list<FtEntry> _L2FTranslate(const LogicalEntry& le);
    list<PhysicalEntry> _F2PTranslate(const list<FtEntry>& fel);
    list<PhysicalWriteEntry> _F2PTranslate(const list<FtWriteEntry>& fwel);
    FtBlkAddr _Pba2Fba(const PhysicalBlkAddr& pba);
    PhysicalBlkAddr _Fba2Pba(const FtBlkAddr& fsa);

    list<BufferEntry> _SpliceBuffer(
        list<BufferEntry>& src, uint32_t start, uint32_t remain);
    int _SetPhysicalAddress(uint64_t startLba, uint32_t segCnt);
    void _SetLogicalAddress(void);
    int _SetMethod(uint64_t totalNvmBlks);
    list<PhysicalBlkAddr> _GetRebuildGroup(FtBlkAddr fba);
    RaidTypeEnum raidType;
    Method* method = nullptr;
};

} // namespace pos
