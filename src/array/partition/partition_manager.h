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

#include "partition.h"
#include "partition_formatter.h"
#include "src/include/raid_type.h"

#include <array>
#include <vector>

using namespace std;

namespace pos
{
class IODispatcher;
class PartitionManager
{
    friend class ParityLocationWbtCommand;

public:
    PartitionManager(void);
    virtual ~PartitionManager(void);
    virtual const PartitionLogicalSize* GetSizeInfo(PartitionType type);
    virtual const PartitionPhysicalSize* GetPhysicalSize(PartitionType type);
    virtual int CreatePartitions(ArrayDevice* nvm, vector<ArrayDevice*> data,
        RaidTypeEnum metaRaid, RaidTypeEnum dataRaid, IPartitionServices* svc);
    virtual void DeletePartitions(void);
    virtual void FormatPartition(PartitionType type, uint32_t arrayId, IODispatcher* io);
    virtual RaidState GetRaidState(void);
    virtual RaidTypeEnum GetRaidType(PartitionType type);

private:
    Partitions partitions;
};

} // namespace pos
