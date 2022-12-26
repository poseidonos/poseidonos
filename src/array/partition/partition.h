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

#include <string>
#include <vector>
#include <array>

#include "src/include/partition_type.h"
#include "src/include/address_type.h"
#include "src/include/raid_type.h"
#include "src/include/raid_state.h"
#include "src/array/device/array_device.h"
#include "src/array_models/dto/partition_physical_size.h"
#include "src/array_models/dto/partition_logical_size.h"
#include "src/array/service/io_translator/i_translator.h"
#include "src/array/partition/i_partition_services.h"

#include "src/debug_lib/debug_info_maker.h"
#include "src/debug_lib/debug_info_maker.hpp"
#include "src/debug_lib/debug_info_queue.h"
#include "src/debug_lib/debug_info_queue.hpp"

using namespace std;

namespace pos
{
class DebugPartition : public DebugInfoInstance
{
public:
    PartitionLogicalSize logicalSize;
    PartitionPhysicalSize physicalSize;
    PartitionType type;
};
class Partition : public ITranslator, public DebugInfoMaker<DebugPartition>
{
public:
    Partition(vector<ArrayDevice*> d, PartitionType type);
    virtual ~Partition(void);
    virtual bool IsByteAccessSupported(void) = 0;
    const PartitionLogicalSize* GetLogicalSize();
    const PartitionPhysicalSize* GetPhysicalSize();
    bool IsValidLba(uint64_t lba);
    int FindDevice(IArrayDevice* dev);
    virtual RaidState GetRaidState(void) { return RaidState::NORMAL; }
    virtual void RegisterService(IPartitionServices* svc) {}
    PartitionType GetType(void) { return type; }
    uint64_t GetLastLba() { return physicalSize.lastLba; }
    const vector<ArrayDevice*> GetDevs(void) { return devs; }
    virtual RaidTypeEnum GetRaidType(void) { return RaidTypeEnum::NONE; }
    virtual void MakeDebugInfo(DebugPartition& obj) final;

protected:
    bool _IsValidEntry(StripeId stripeId, BlkOffset offset, uint32_t blkCnt);
    void _UpdateLastLba(void);
    PartitionLogicalSize logicalSize;
    PartitionPhysicalSize physicalSize;
    vector<ArrayDevice*> devs;
    PartitionType type;
    DebugPartition debugPartition;
    DebugInfoQueue<DebugPartition> partitionQueue;;
};

using Partitions = array<Partition*, PartitionType::TYPE_COUNT>;

} // namespace pos
