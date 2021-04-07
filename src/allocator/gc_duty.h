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

#include <atomic>

#include "allocator_address_info.h"
#include "allocator_meta_archive.h"

namespace ibofos
{
class GcDuty
{
public:
    GcDuty(AllocatorAddressInfo& addrInfoI, AllocatorMetaArchive* metaI);
    virtual ~GcDuty(void);

    uint32_t GetNumOfFreeUserDataSegment(void);
    SegmentId GetMostInvalidatedSegment(void);
    void SetGcThreshold(uint32_t inputThreshold) { thresholdSegments = inputThreshold; }
    void SetUrgentThreshold(uint32_t inputThreshold) { urgentSegments = inputThreshold; }
    uint32_t GetGcThreshold(void) { return thresholdSegments; }
    uint32_t GetUrgentThreshold(void) { return urgentSegments; }
    std::atomic<bool>& IsBlockedForUserSegmentAlloc(void) { return blockSegmentAllocForUser; }
    void SetUpBlockSegmentAllocForUser(bool isBlock) { blockSegmentAllocForUser = isBlock; }
    void FreeUserDataSegmentId(SegmentId segId);

private:
    void _FreeRebuildTargetSegment(SegmentId segId);

    uint32_t thresholdSegments;
    uint32_t urgentSegments;
    std::atomic<bool> blockSegmentAllocForUser;

    AllocatorAddressInfo& addrInfo;
    AllocatorMetaArchive* meta;
};

} // namespace ibofos
