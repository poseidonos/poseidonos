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

#include <set>
#include <string>
#include <utility>

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"
#include "src/allocator/include/allocator_const.h"
#include "src/meta_file_intf/meta_file_intf.h"
#include "src/state/interface/i_state_control.h"

namespace pos
{
class RebuildCtx
{
public:
    RebuildCtx(std::string arrayName, AllocatorCtx* allocCtx, IStateControl* iStateCtrl);
    virtual ~RebuildCtx(void);
    virtual void Init(AllocatorAddressInfo* info);
    virtual void Dispose(void);

    virtual SegmentId GetRebuildTargetSegment(void);
    virtual int ReleaseRebuildSegment(SegmentId segmentId);
    virtual bool NeedRebuildAgain(void);
    void FreeSegmentInRebuildTarget(SegmentId segId);

    virtual bool IsRebuildTargetSegment(SegmentId segId);
    bool IsRebuidTargetSegmentsEmpty(void);
    RTSegmentIter FindRebuildTargetSegment(SegmentId segmentId);
    RTSegmentIter RebuildTargetSegmentsBegin(void);
    RTSegmentIter RebuildTargetSegmentsEnd(void);
    std::pair<RTSegmentIter, bool> EmplaceRebuildTargetSegment(SegmentId segmentId);
    void ClearRebuildTargetSegments(void);
    uint32_t GetTargetSegmentCnt(void);
    void FlushRebuildCtx(void);
    void SetUnderRebuildSegmentId(SegmentId segmentId);

private:
    int _PrepareRebuildCtx(void);
    void _LoadRebuildCtxSync(void);
    void _RebuildCtxLoaded(void);
    void _StoreRebuildCtx(void);
    void _EraseRebuildTargetSegments(RTSegmentIter iter);
    void _FlushRebuildCtxCompleted(AsyncMetaFileIoCtx* ctx);
    SegmentId _GetUnderRebuildSegmentId(void);

    bool needRebuildCont;
    uint32_t targetSegmentCnt;
    std::set<SegmentId> rebuildTargetSegments; // No lock
    SegmentId underRebuildSegmentId;
    MetaFileIntf* rebuildSegmentsFile;
    std::mutex rebuildLock;
    char* bufferInObj;
    std::string arrayName;

    AllocatorCtx* allocatorCtx;
    IStateControl* iStateControl;
};

} // namespace pos
