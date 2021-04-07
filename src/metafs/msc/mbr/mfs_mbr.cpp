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

#include "mfs_mbr.h"

#include <ctime>

MetaFsMBR::MetaFsMBR(MetaFsAnchorRegionType regionType, MetaLpnType baseLpn)
: RawMetaRegion(MetaStorageType::SSD, regionType, baseLpn)
{
}

MetaFsMBR::~MetaFsMBR(void)
{
    delete content;
}

void
MetaFsMBR::CreateMBR(void)
{
    // CreateMBR, Need to add more context
    std::time_t t = std::time(0); // get time now
    std::tm* now = std::localtime(&t);
    // 0000_YYYY_MMDD_HHMM_
    content->mfsEpochSignature = (uint64_t)(now->tm_year + 1900) * 10000000000 +
        (uint64_t)(now->tm_mon + 1) * 100000000 +
        (uint64_t)now->tm_mday * 1000000 +
        (uint64_t)now->tm_hour * 10000 +
        (uint64_t)now->tm_min * 100 +
        (uint64_t)now->tm_sec;

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta filesystem MBR has been created. Epoch timestamp={}",
        content->mfsEpochSignature);

    MarkValid();
    SetPORStatus(false /* SPO */);
}

uint64_t
MetaFsMBR::GetEpochSignature(void)
{
    return content->mfsEpochSignature;
}

void
MetaFsMBR::BuildMBR(void)
{
    // BuildMBR
}

void
MetaFsMBR::MarkValid(void)
{
    content->mbrSignature = MetaFsMBRContent::MBR_SIGNATURE;
}

bool
MetaFsMBR::IsValidMBRExist(void)
{
    return (MetaFsMBRContent::MBR_SIGNATURE == content->mbrSignature) ? true : false;
}

void
MetaFsMBR::SetPORStatus(bool isShutdownOff)
{
    content->isNPOR = isShutdownOff;
}
bool
MetaFsMBR::GetPORStatus(void)
{
    return content->isNPOR;
}

void
MetaFsMBR::InvalidMBRSignature(void)
{
    content->mbrSignature = MBR_RESETSIGNATURE;
}
