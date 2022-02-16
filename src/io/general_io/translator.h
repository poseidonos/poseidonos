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

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/array/service/io_translator/io_translator.h"
#include "src/include/address_type.h"
#include "src/include/partition_type.h"
#include "src/mapper/i_stripemap.h"
#include "src/mapper/i_vsamap.h"

namespace pos
{
class Translator
{
public:
    Translator(uint32_t volumeId, BlkAddr startRba, uint32_t blockCount,
        int arrayId, bool isRead = false, IVSAMap* iVSAMap = nullptr,
        IStripeMap* iStripeMap = nullptr, IWBStripeAllocator* iWBStripeAllocator = nullptr,
        IIOTranslator* iTranslator = nullptr);
    Translator(uint32_t volumeId, BlkAddr rba, int arrayId, bool isRead);
    Translator(const VirtualBlkAddr& vsa, int arrayId);
    virtual ~Translator(void)
    {
    }
    virtual PhysicalBlkAddr GetPba(uint32_t blockIndex);
    virtual PhysicalBlkAddr GetPba(void);
    virtual list<PhysicalEntry> GetPhysicalEntries(void* mem, uint32_t blockCount);
    virtual StripeAddr GetLsidEntry(uint32_t blockIndex);
    virtual LsidRefResult GetLsidRefResult(uint32_t blockIndex);
    virtual bool IsUnmapped(void);
    virtual bool IsMapped(void);
    virtual VirtualBlkAddr GetVsa(uint32_t blockIndex);

private:
    static const uint32_t ONLY_ONE = 1;

    IVSAMap* iVSAMap{nullptr};
    IStripeMap* iStripeMap{nullptr};
    IWBStripeAllocator* iWBStripeAllocator{nullptr};
    IIOTranslator* iTranslator{nullptr};
    BlkAddr startRba;
    uint32_t blockCount;
    VsaArray vsaArray;
    VirtualBlkAddr lastVsa;
    StripeAddr lastLsidEntry;
    std::array<LsidRefResult, MAX_PROCESSABLE_BLOCK_COUNT> lsidRefResults;
    bool isRead;
    uint32_t volumeId;
    static thread_local StripeId recentVsid;
    static thread_local StripeId recentLsid;
    static thread_local int recentArrayId;

    LogicalBlkAddr _GetLsa(uint32_t blockIndex);
    LsidRefResult _GetLsidRefResult(BlkAddr rba, VirtualBlkAddr& vsa);
    void _CheckSingleBlock(void);
    PartitionType _GetPartitionType(uint32_t blockIndex);
    int arrayId;
};

} // namespace pos
