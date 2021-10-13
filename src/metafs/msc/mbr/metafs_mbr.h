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

#include "mfs_geometry.h"
#include "meta_region.h"

#define MBR_RESETSIGNATURE 0xCECE0001BEEFCAFE

namespace pos
{
class MetaFsMBRContent : public MetaRegionContent
{
public:
    bool isNPOR;
    static const uint64_t MBR_SIGNATURE = 0x1B0F0001CEC0FFEE;
    uint64_t mbrSignature;
    uint64_t mfsEpochSignature;
    MetaFsGeometryInfo geometry;
};

enum class MetaFsAnchorRegionType
{
    First = 0,
    MasterBootRecord = First,

    Max,
};

class MetaFsMBR : public MetaRegion<MetaFsAnchorRegionType, MetaFsMBRContent>
{
public:
    MetaFsMBR(void) = default;  // Ctor for UT code
    MetaFsMBR(MetaFsAnchorRegionType regionType, MetaLpnType baseLpn);
    virtual ~MetaFsMBR(void);

    virtual void CreateMBR(void);
    void BuildMBR(void);
    virtual uint64_t GetEpochSignature(void);
    virtual bool IsValidMBRExist(void);
    virtual bool GetPORStatus(void);
    virtual void SetPORStatus(bool isShutdownOff);
    virtual void InvalidMBRSignature(void);

protected:
    void _MarkValid(void);
};
} // namespace pos
