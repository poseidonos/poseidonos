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

#include "src/metadata/segment_context_updater.h"

#include "src/array_models/dto/partition_logical_size.h"

namespace pos
{
SegmentContextUpdater::SegmentContextUpdater(ISegmentCtx* context_, IVersionedSegmentContext* versionedContext_, const PartitionLogicalSize* sizeInfo)
: addrInfo(sizeInfo),
  logGroupId(0),
  activeContext(context_),
  versionedContext(versionedContext_)
{
}

void
SegmentContextUpdater::ValidateBlks(VirtualBlks blks, int groupId)
{
    activeContext->ValidateBlks(blks);

    SegmentId segmentId = blks.startVsa.stripeId / addrInfo->stripesPerSegment;
    versionedContext->IncreaseValidBlockCount(groupId, segmentId, blks.numBlks);
}

bool
SegmentContextUpdater::InvalidateBlks(VirtualBlks blks, bool isForced, int groupId)
{
    bool ret = activeContext->InvalidateBlks(blks, isForced);

    SegmentId segmentId = blks.startVsa.stripeId / addrInfo->stripesPerSegment;
    versionedContext->DecreaseValidBlockCount(groupId, segmentId, blks.numBlks);

    return ret;
}

bool
SegmentContextUpdater::UpdateOccupiedStripeCount(StripeId lsid, int groupId)
{
    bool ret = activeContext->UpdateOccupiedStripeCount(lsid);

    SegmentId segmentId = lsid / addrInfo->stripesPerSegment;
    versionedContext->IncreaseOccupiedStripeCount(groupId, segmentId);

    return ret;
}
} // namespace pos
