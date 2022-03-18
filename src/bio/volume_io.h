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
#include <string>
#include <vector>

#include "src/bio/ubio.h"
#include "src/include/smart_ptr_type.h"

struct pos_io;

namespace pos
{
using VirtualBlockHandler = void (*)(VirtualBlks&);

struct RbaAndSize
{
    uint64_t sectorRba;
    uint64_t size;

    inline bool
    operator==(RbaAndSize input) const
    {
        return (input.sectorRba == sectorRba && input.size == size);
    }

    inline bool
    operator<(RbaAndSize input) const
    {
        return (sectorRba < input.sectorRba);
    }
};

class VolumeIo : public Ubio
{
public:
    using RbaList = std::list<RbaAndSize>;
    using RbaListIter = RbaList::iterator;

    VolumeIo(void) = delete;
    VolumeIo(void* buffer, uint32_t unitCount, int arrayId);
    VolumeIo(const VolumeIo& volumeIo);
    ~VolumeIo(void) override;

    virtual VolumeIoSmartPtr Split(uint32_t sectors, bool removalFromTail);
    virtual VolumeIoSmartPtr GetOriginVolumeIo(void);
    virtual uint32_t GetVolumeId(void);
    void SetVolumeId(uint32_t inputVolumeId);
    bool IsPollingNecessary(void);
    uint32_t GetOriginCore(void) override;
    virtual void SetLsidEntry(StripeAddr& lsidEntry);
    void SetOldLsidEntry(StripeAddr& lsidEntry);
    virtual const StripeAddr& GetLsidEntry(void);
    virtual const StripeAddr& GetOldLsidEntry(void);
    virtual const VirtualBlkAddr& GetVsa(void);
    void SetVsa(VirtualBlkAddr&);
    void SetSectorRba(uint64_t inputSectorRba);
    virtual uint64_t GetSectorRba(void);
    void SetUserLsid(StripeId stripeId);
    virtual StripeId GetUserLsid(void);

private:
    static const StripeAddr INVALID_LSID_ENTRY;
    static const VirtualBlkAddr INVALID_VSA;
    static const uint64_t INVALID_RBA;

    uint32_t volumeId;
    uint32_t originCore;
    StripeAddr lsidEntry;
    StripeAddr oldLsidEntry;
    VirtualBlkAddr vsa;
    uint64_t sectorRba;
    StripeId stripeId;

    bool _IsInvalidVolumeId(uint32_t inputVolumeId);
    virtual bool _IsInvalidLsidEntry(StripeAddr& inputLsidEntry);
    bool _IsInvalidVsa(VirtualBlkAddr&);
    bool _IsInvalidSectorRba(uint64_t inputSectorRba);
    bool _CheckVolumeIdSet(void);
    bool _CheckOriginCoreSet(void);
    bool _CheckVsaSet(void);
    bool _CheckSectorRbaSet(void);
};

} // namespace pos
