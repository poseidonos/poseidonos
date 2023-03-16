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

#include "segment_freed_update_request.h"

#include <memory>

#include "src/logger/logger.h"
#include "src/include/pos_event_id.hpp"
#include "src/meta_service/meta_service.h"
#include "src/meta_service/i_meta_updater.h"

namespace pos
{
SegmentFreedUpdateRequest::SegmentFreedUpdateRequest(SegmentCtx* segmentCtx, SegmentId targetSegmentId, int arrayIdInput)
: SegmentFreedUpdateRequest(segmentCtx, targetSegmentId,
      MetaServiceSingleton::Instance()->GetMetaUpdater(arrayIdInput))
{
}

SegmentFreedUpdateRequest::SegmentFreedUpdateRequest(SegmentCtx* segmentCtx, SegmentId targetSegmentId,
    IMetaUpdater* metaUpdater)
: Callback(false, CallbackType_BackendLogWriteDone),
  segmentCtx(segmentCtx),
  targetSegmentId(targetSegmentId),
  metaUpdater(metaUpdater)
{
}

SegmentFreedUpdateRequest::~SegmentFreedUpdateRequest(void)
{
}

// For IT
void
SegmentFreedUpdateRequest::SetMetaUpdater(IMetaUpdater* metaUpdater_)
{
    metaUpdater = metaUpdater_;
}

SegmentId
SegmentFreedUpdateRequest::GetTargetSegmentId(void)
{
    return targetSegmentId;
}

bool
SegmentFreedUpdateRequest::_DoSpecificJob(void)
{
    int result = metaUpdater->UpdateFreedSegmentContext(segmentCtx, targetSegmentId);
    if (result != 0)
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_SEGMENT_FREE_REQUEST_FAILED), "Failed request to update for freed segment, segmendId:{}", targetSegmentId);
        return false;
    }
    return true;
}

} // namespace pos
