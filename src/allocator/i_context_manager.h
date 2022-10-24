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

#include <mutex>
#include <vector>

#include "src/allocator/include/allocator_const.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"
#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"
#include "src/journal_manager/log_buffer/versioned_segment_ctx.h"

namespace pos
{
class IContextManager
{
public:
    virtual int FlushContexts(EventSmartPtr callback, bool sync, int logGroupId = ALL_LOG_GROUP) = 0;
    virtual uint64_t GetStoredContextVersion(int owner) = 0;

    virtual SegmentId AllocateFreeSegment(void) = 0;
    virtual SegmentId AllocateGCVictimSegment(void) = 0;
    virtual SegmentId AllocateRebuildTargetSegment(void) = 0;
    virtual int ReleaseRebuildSegment(SegmentId segmentId) = 0;
    virtual int StopRebuilding(void) = 0;
    virtual bool NeedRebuildAgain(void) = 0;
    virtual uint32_t GetRebuildTargetSegmentCount(void) = 0;
    virtual int GetGcThreshold(GcMode mode) = 0;

    virtual SegmentCtx* GetSegmentCtx(void) = 0;
    virtual GcCtx* GetGcCtx(void) = 0;
    virtual void PrepareVersionedSegmentCtx(IVersionedSegmentContext* versionedSegCtx_) = 0;
    virtual void ResetFlushedInfo(int logGroupId) = 0;
    virtual void SetAllocateDuplicatedFlush(bool flag) = 0;
    virtual void SetSegmentContextUpdaterPtr(ISegmentCtx* segmentContextUpdater_) = 0;
    virtual ISegmentCtx* GetSegmentContextUpdaterPtr(void) = 0;

private:
    static const int ALL_LOG_GROUP = -1;
};

} // namespace pos
