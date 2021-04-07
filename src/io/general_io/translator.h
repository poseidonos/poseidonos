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

#pragma once

#include <mutex>

#include "src/array/partition/partition.h"
#include "src/include/address_type.h"
#include "src/mapper/mapper.h"

namespace ibofos
{
class Array;

class Translator
{
public:
    Translator(uint32_t volumeId, BlkAddr startRba, uint32_t blockCount,
        bool isRead = false);
    Translator(uint32_t volumeId, BlkAddr rba, bool isRead);
    Translator(const VirtualBlkAddr& vsa);
    PhysicalBlkAddr GetPba(uint32_t blockIndex);
    PhysicalBlkAddr GetPba(void);
    PhysicalEntries GetPhysicalEntries(void* mem, uint32_t blockCount);
    StripeAddr GetLsidEntry(uint32_t blockIndex);
    LsidRefResult GetLsidRefResult(uint32_t blockIndex);
    bool IsUnmapped(void);
    bool IsMapped(void);
    VirtualBlkAddr GetVsa(void);
    VirtualBlkAddr GetVsa(uint32_t blockIndex);

private:
    static const uint32_t ONLY_ONE = 1;

    Mapper* mapper;
    BlkAddr startRba;
    uint32_t blockCount;
    VsaArray vsaArray;
    VirtualBlkAddr lastVsa;
    StripeAddr lastLsidEntry;
    std::array<LsidRefResult, MAX_PROCESSABLE_BLOCK_COUNT> lsidRefResults;
    Array* arrayManager;
    bool isRead;
    uint32_t volumeId;
    static thread_local StripeId recentVsid;
    static thread_local StripeId recentLsid;

    LogicalBlkAddr _GetLsa(uint32_t blockIndex);
    LsidRefResult _GetLsidRefResult(BlkAddr rba, VirtualBlkAddr& vsa);
    void _CheckSingleBlock(void);
    PartitionType _GetPartitionType(uint32_t blockIndex);
};

} // namespace ibofos
