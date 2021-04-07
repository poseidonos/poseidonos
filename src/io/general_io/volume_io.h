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

#include <vector>

#include "src/io/general_io/ubio.h"

struct ibof_io;

namespace ibofos
{
using VirtualBlockHandler = void (*)(VirtualBlks&);

struct AlignmentInformation
{
    uint32_t offset;
    uint32_t size;
};

class VolumeIo;

using VolumeIoSmartPtr = std::shared_ptr<VolumeIo>;

class VolumeIo : public Ubio
{
public:
    VolumeIo(void) = delete;
    VolumeIo(void* buffer, uint32_t unitCount);
    VolumeIo(const VolumeIo& volumeIo);
    ~VolumeIo(void) override;

    VolumeIoSmartPtr Split(uint32_t sectors, bool removalFromTail);
    VolumeIoSmartPtr GetOriginVolumeIo(void);
    uint32_t GetVolumeId(void);
    void SetVolumeId(uint32_t inputVolumeId);
    bool IsPollingNecessary(void);
    uint32_t GetOriginCore(void) override;
    void SetLsidEntry(StripeAddr& lsidEntry);
    void SetOldLsidEntry(StripeAddr& lsidEntry);
    const StripeAddr& GetLsidEntry(void);
    const StripeAddr& GetOldLsidEntry(void);
    const VirtualBlkAddr& GetVsa(void);
    void SetVsa(VirtualBlkAddr&);
    void AddAllocatedVirtualBlks(VirtualBlks& virtualBlks);
    VirtualBlkAddr PopHeadVsa(void);
    VirtualBlkAddr PopTailVsa(void);
    uint32_t GetAllocatedBlockCount(void);
    uint32_t GetAllocatedVirtualBlksCount(void);
    VirtualBlks& GetAllocatedVirtualBlks(uint32_t index);
    void SetAlignmentInformation(AlignmentInformation alignmentInformation);
    const AlignmentInformation& GetAlignmentInformation(void);
    void SetGc(VirtualBlkAddr& oldVsa);
    bool IsGc(void);
    const VirtualBlkAddr& GetOldVsa(void);

private:
    static const StripeAddr INVALID_LSID_ENTRY;
    static const VirtualBlkAddr INVALID_VSA;

    uint32_t volumeId;
    uint32_t originCore;
    StripeAddr lsidEntry;
    StripeAddr oldLsidEntry;
    VirtualBlkAddr vsa;
    uint32_t allocatedBlockCount;
    std::vector<VirtualBlks> allocatedVirtualBlks;
    AlignmentInformation alignmentInformation;
    bool isGc;
    VirtualBlkAddr oldVsaForGc;

    bool _IsInvalidVolumeId(uint32_t inputVolumeId);
    bool _IsInvalidLsidEntry(StripeAddr& inputLsidEntry);
    bool _IsInvalidVsa(VirtualBlkAddr&);
    bool _CheckVolumeIdSet(void);
    bool _CheckOriginCoreSet(void);
    bool _CheckVsaSet(void);
};

} // namespace ibofos
