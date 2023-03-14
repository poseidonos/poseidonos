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

#include <set>
#include <string>
#include <utility>

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"
#include "src/allocator/context_manager/context/context.h"
#include "src/allocator/context_manager/context/context_section.h"
#include "src/allocator/context_manager/i_allocator_file_io_client.h"
#include "src/state/interface/i_state_control.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx_extended.h"


namespace pos
{
class TelemetryPublisher;
class AllocatorFileIo;

class RebuildCtx : public IAllocatorFileIoClient
{
public:
    RebuildCtx(void) = default;
    RebuildCtx(TelemetryPublisher* tp_, RebuildCtxHeader* header, AllocatorAddressInfo* info); // for UT
    RebuildCtx(TelemetryPublisher* tp_, AllocatorAddressInfo* info);
    virtual ~RebuildCtx(void);

    virtual void SetAllocatorFileIo(AllocatorFileIo* fileIo_);
    virtual void Init(void);
    virtual void Dispose(void);

    virtual void AfterLoad(char* buf) override;
    virtual void BeforeFlush(char* buf, ContextSectionBuffer externalBuf = INVALID_CONTEXT_SECTION_BUFFER) override;
    virtual void AfterFlush(char* buf) override;
    virtual ContextSectionAddr GetSectionInfo(int section) override;
    virtual uint64_t GetStoredVersion(void) override;
    virtual void ResetDirtyVersion(void) override;
    virtual int GetNumSections(void) override;
    virtual uint64_t GetTotalDataSize(void) override;


    virtual int FlushRebuildSegmentList(std::set<SegmentId> segIdSet);
    virtual std::set<SegmentId> GetList(void);

private:
    int _FlushContext(void);
    void _UpdateRebuildList(std::set<SegmentId> list); // for test
    void _UpdateSectionInfo(void);

    // Data to be stored 1 ~ 2
    ContextSection<RebuildCtxHeader> ctxHeader;
    ContextSection<SegmentId*> segmentList;

    // Data to be stored with Protobuf: Section 3
    ContextSection<RebuildCtxExtended*> ctxExtended;

    uint64_t totalDataSize;

    // In-memory data structures
    uint32_t listSize;

    std::atomic<uint64_t> ctxStoredVersion;
    std::atomic<uint64_t> ctxDirtyVersion;

    bool initialized;

    // Dependencies
    AllocatorAddressInfo* addrInfo;
    TelemetryPublisher* tp;
    AllocatorFileIo* fileIo;
};

} // namespace pos
