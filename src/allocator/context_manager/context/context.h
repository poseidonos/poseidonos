/*
*   BSD LICENSE
*   Copyright (c) 2022 Samsung Electronics Corporation
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

#include <cstdint>
#include <string>

#include "src/allocator/include/allocator_const.h"
#include "src/include/address_type.h"

namespace pos
{
enum FileOwner
{
    SEGMENT_CTX,
    ALLOCATOR_CTX,
    NUM_ALLOCATOR_FILES,
    REBUILD_CTX = NUM_ALLOCATOR_FILES,
    NUM_FILES
};

constexpr auto
ToFilename(FileOwner owner)
{
    const char* names[] = {
        "SegmentContext",
        "AllocatorContexts",
        "RebuildContext"};
    return names[owner];
}

enum SegmentCtxSection
{
    SC_HEADER = 0,
    SC_SEGMENT_INFO,
    SC_EXTENDED,
    NUM_SEGMENT_CTX_SECTION,
};

enum AllocatorCtxSection
{
    AC_HEADER = 0,
    AC_CURRENT_SSD_LSID,
    AC_ALLOCATE_WBLSID_BITMAP,
    AC_ACTIVE_STRIPE_TAIL,
    AC_EXTENDED,
    NUM_ALLOCATOR_CTX_SECTION,
};

enum RebuildCtxSection
{
    RC_HEADER = 0,
    RC_REBUILD_SEGMENT_LIST,
    RC_EXTENDED,
    NUM_REBUILD_CTX_SECTION
};

constexpr int
WbtTypeToAllocatorSectionId(WBTAllocatorMetaType type)
{
    if (type == WBT_CURRENT_SSD_LSID)
        return AC_CURRENT_SSD_LSID;
    else if (type == WBT_WBLSID_BITMAP)
        return AC_ALLOCATE_WBLSID_BITMAP;
    else if (type == WBT_ACTIVE_STRIPE_TAIL)
        return AC_ACTIVE_STRIPE_TAIL;
    else
        return -1; // should not reach here
}

static const uint32_t SIG_SEGMENT_CTX = 0xAFAFAFAF;
static const uint32_t SIG_ALLOCATOR_CTX = 0xBFBFBFBF;
static const uint32_t SIG_REBUILD_CTX = 0xCFCFCFCF;

struct CtxHeader
{
    uint32_t sig;
    uint64_t ctxVersion;

    CtxHeader(uint32_t signature): sig(signature), ctxVersion(0) {}
};

struct AllocatorCtxHeader : CtxHeader
{
    uint32_t numValidWbLsid = 0;

    AllocatorCtxHeader(void): CtxHeader(SIG_ALLOCATOR_CTX) {}
};

struct SegmentCtxHeader : CtxHeader
{
    uint32_t numValidSegment = 0;

    SegmentCtxHeader(void): CtxHeader(SIG_SEGMENT_CTX) {}
};

struct RebuildCtxHeader : CtxHeader
{
    uint32_t numTargetSegments = 0;

    RebuildCtxHeader(void): CtxHeader(SIG_REBUILD_CTX) {}
};

struct ContextSectionAddr
{
    uint64_t offset = 0;
    uint64_t size = 0;
};

const int INVALID_CONTEXT_ID = -1;
const int INVALID_SECTION_ID = -1;

struct ContextSectionBuffer
{
    int owner;
    int sectionId;
    char* buffer;
};

static const ContextSectionBuffer INVALID_CONTEXT_SECTION_BUFFER = {
    .owner = INVALID_CONTEXT_ID,
    .sectionId = INVALID_SECTION_ID,
    .buffer = nullptr};
} // namespace pos