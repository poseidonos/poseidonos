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
#include <vector>

#include "partition.h"

using namespace std;

namespace pos
{

class JournalSsdPartition : public Partition
{
public:
    explicit JournalSsdPartition(vector<ArrayDevice *> devs);
    virtual ~JournalSsdPartition();
    int Create(uint64_t startLba);
    void RegisterService(IPartitionServices* svc) override;
    int Translate(PhysicalBlkAddr& dst, const LogicalBlkAddr& src) override;
    int ByteTranslate(PhysicalByteAddr& dst, const LogicalByteAddr& src) override;
    int Convert(list<PhysicalWriteEntry>& dst, const LogicalWriteEntry& src) override;
    int ByteConvert(list<PhysicalByteWriteEntry> &dst, const LogicalByteWriteEntry &src) override;
    bool IsByteAccessSupported(void) override;

private:
    int _SetPhysicalAddress(uint64_t startLba);
    void _SetLogicalAddress(void);
};

} // namespace pos
