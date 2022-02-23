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

#include "src/include/partition_type.h"
#include "src/include/raid_type.h"
#include "src/array/partition/partition.h"
#include "src/logger/logger.h"

#include <vector>
using namespace std;

namespace pos
{
class ArrayDevice;

class SsdPartitionOptions
{
public:
    SsdPartitionOptions(PartitionType type, RaidTypeEnum raid, vector<ArrayDevice*> devs, ArrayDevice* dev)
    {
        partitionType = type;
        raidType = raid;
        devices = devs;
        nvm = dev;
    }

    SsdPartitionOptions(const SsdPartitionOptions& opt)
    {
        partitionType = opt.partitionType;
        raidType = opt.raidType;
        devices = opt.devices;
        nvm = opt.nvm;
    }

    PartitionType partitionType;
    RaidTypeEnum raidType = RaidTypeEnum::NONE;
    vector<ArrayDevice*> devices;
    ArrayDevice* nvm = nullptr;
};

class SsdPartitionBuilder
{
public:
    explicit SsdPartitionBuilder(SsdPartitionOptions opt)
    : option(opt)
    {
    }
    int Build(uint64_t startLba, Partitions& out);
    void SetNext(SsdPartitionBuilder* builder) { next = builder; }

private:
    uint32_t _GetSegmentCount(void);
    SsdPartitionBuilder* next = nullptr;
    SsdPartitionOptions option;
    uint64_t _GetMinCapacity(const vector<ArrayDevice*>& devs);
    uint64_t _GetMaxCapacity(const vector<ArrayDevice*>& devs);
};
} // namespace pos
