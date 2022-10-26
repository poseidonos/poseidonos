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

#include "src/allocator/i_segment_ctx.h"
#include "src/journal_manager/log_buffer/i_versioned_segment_context.h"

namespace pos
{
class PartitionLogicalSize;

class SegmentContextUpdater : public ISegmentCtx
{
public:
    SegmentContextUpdater(ISegmentCtx* segmentCtx_, IVersionedSegmentContext* versionedContext_,
        const PartitionLogicalSize* addrInfo_);
    virtual ~SegmentContextUpdater(void);

    virtual void ValidateBlks(VirtualBlks blks);
    virtual bool InvalidateBlks(VirtualBlks blks, bool isForced);
    virtual bool UpdateOccupiedStripeCount(StripeId lsid);
    virtual void ValidateBlocksWithGroupId(VirtualBlks blks, int logGroupId);
    virtual bool InvalidateBlocksWithGroupId(VirtualBlks blks, bool isForced, int logGroupId);
    virtual bool UpdateStripeCount(StripeId lsid, int logGroupId);

private:
    const PartitionLogicalSize* addrInfo;
    ISegmentCtx* activeSegmentCtx;
    IVersionedSegmentContext* versionedContext;
    pthread_rwlock_t lock;
};

} // namespace pos
