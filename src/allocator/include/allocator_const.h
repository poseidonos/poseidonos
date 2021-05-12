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

#include <cstdint>
#include <set>

#include "src/include/address_type.h"
#include "src/volume/volume_list.h"

namespace pos
{
using ASTailArrayIdx = uint32_t;
using RTSegmentIter = std::set<SegmentId>::iterator;

const int ACTIVE_STRIPE_TAIL_ARRAYLEN = MAX_VOLUME_COUNT * 2;

enum FileOwner
{
    SEGMENT_CTX,
    ALLOCATOR_CTX,
    NUM_FILES
};

enum SegmentCtxSection
{
    SC_HEADER = 0,
    SC_SEGMENT_INFO,
    NUM_SEGMENT_CTX_SECTION
};

enum AllocatorCtxSection
{
    AC_HEADER = 0,
    AC_ALLOCATE_SEGMENT_BITMAP,
    AC_CURRENT_SSD_LSID,
    AC_SEGMENT_STATES,
    NUM_ALLOCATION_INFO,
    AC_ALLOCATE_WBLSID_BITMAP = NUM_ALLOCATION_INFO,
    AC_ACTIVE_STRIPE_TAIL,
    NUM_ALLOCATOR_CTX_SECTION
};

enum WBTAllocatorMetaType
{
    WBT_SEGMENT_BITMAP,
    WBT_CURRENT_SSD_LSID,
    WBT_SEGMENT_STATES,
    WBT_WBLSID_BITMAP,
    WBT_ACTIVE_STRIPE_TAIL,

    WBT_SEGMENT_VALID_COUNT,
    WBT_SEGMENT_OCCUPIED_STRIPE,
    WBT_NUM_ALLOCATOR_META
};

struct AllocatorCtxHeader
{
    uint32_t sig;
    uint64_t ctxVersion;
    uint32_t numValidWbLsid;
    uint32_t numValidSegment;
};

struct SegmentCtxHeader
{
    uint32_t sig;
    uint64_t ctxVersion;
};

enum CurrentGcMode
{
    MODE_NO_GC = 0,
    MODE_NORMAL_GC,
    MODE_URGENT_GC,
};

} // namespace pos
