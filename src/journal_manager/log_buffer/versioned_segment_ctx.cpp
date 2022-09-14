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

#include "versioned_segment_ctx.h"

#include <unordered_map>

#include "src/allocator/context_manager/segment_ctx/segment_info.h"
#include "src/include/pos_event_id.h"
#include "src/journal_manager/config/journal_configuration.h"
#include "src/logger/logger.h"
#include "versioned_segment_info.h"

namespace pos
{
VersionedSegmentCtx::VersionedSegmentCtx(void)
: config(nullptr),
  numSegments(0),
  segmentInfos(nullptr)
{
}

VersionedSegmentCtx::~VersionedSegmentCtx(void)
{
    Dispose();
}

void
VersionedSegmentCtx::Init(JournalConfiguration* journalConfiguration, SegmentInfo* loadedSegmentInfo, uint32_t numSegments_)
{
    _Init(journalConfiguration, loadedSegmentInfo, numSegments_);

    for (int index = 0; index < config->GetNumLogGroups(); index++)
    {
        std::shared_ptr<VersionedSegmentInfo> segmentInfo(new VersionedSegmentInfo());
        segmentInfoDiffs.push_back(segmentInfo);
    }
}

void
VersionedSegmentCtx::Init(JournalConfiguration* journalConfiguration, SegmentInfo* loadedSegmentInfo, uint32_t numSegments_,
    std::vector<std::shared_ptr<VersionedSegmentInfo>> inputVersionedSegmentInfo)
{
    _Init(journalConfiguration, loadedSegmentInfo, numSegments_);

    assert((int)inputVersionedSegmentInfo.size() == config->GetNumLogGroups());
    segmentInfoDiffs = inputVersionedSegmentInfo;
}

void
VersionedSegmentCtx::_Init(JournalConfiguration* journalConfiguration, SegmentInfo* loadedSegmentInfo, uint32_t numSegments_)
{
    config = journalConfiguration;

    numSegments = numSegments_;
    segmentInfos = new SegmentInfo[numSegments]();
    for (uint32_t segId = 0; segId < numSegments; segId++)
    {
        if (nullptr != loadedSegmentInfo)
        {
            POS_TRACE_INFO(EID(JOURNAL_MANAGER_INITIALIZED), "Loaded segment: segId {}, validcnt {}, stripeCnt {}, state {}",
                           segId, loadedSegmentInfo[segId].GetValidBlockCount(),
                           loadedSegmentInfo[segId].GetOccupiedStripeCount(),
                           loadedSegmentInfo[segId].GetState());

            memcpy(segmentInfos, loadedSegmentInfo, sizeof(SegmentInfo) * numSegments);
        }
    }
}

void
VersionedSegmentCtx::Dispose(void)
{
    if (segmentInfos != nullptr)
    {
        delete[] segmentInfos;
        segmentInfos = nullptr;
    }
}

void
VersionedSegmentCtx::IncreaseValidBlockCount(int logGroupId, SegmentId segId, uint32_t cnt)
{
    _CheckSegIdValidity(segId);
    _CheckLogGroupIdValidity(logGroupId);

    segmentInfoDiffs[logGroupId]->IncreaseValidBlockCount(segId, cnt);
}

void
VersionedSegmentCtx::DecreaseValidBlockCount(int logGroupId, SegmentId segId, uint32_t cnt)
{
    _CheckSegIdValidity(segId);
    _CheckLogGroupIdValidity(logGroupId);

    segmentInfoDiffs[logGroupId]->DecreaseValidBlockCount(segId, cnt);
}

void
VersionedSegmentCtx::IncreaseOccupiedStripeCount(int logGroupId, SegmentId segId)
{
    _CheckSegIdValidity(segId);
    _CheckLogGroupIdValidity(logGroupId);

    segmentInfoDiffs[logGroupId]->IncreaseOccupiedStripeCount(segId);
}

void
VersionedSegmentCtx::_UpdateSegmentContext(int logGroupId)
{
    _CheckLogGroupIdValidity(logGroupId);

    shared_ptr<VersionedSegmentInfo> targetSegInfo = segmentInfoDiffs[logGroupId];
    tbb::concurrent_unordered_map<SegmentId, int> changedValidBlkCount = targetSegInfo->GetChangedValidBlockCount();
    for (auto it = changedValidBlkCount.begin(); it != changedValidBlkCount.end(); it++)
    {
        auto segmentId = it->first;
        auto validBlockCountDiff = it->second;

        uint32_t getValidCount = segmentInfos[segmentId].GetValidBlockCount();
        uint32_t result = getValidCount + validBlockCountDiff;

        POS_TRACE_DEBUG(EID(JOURNAL_DEBUG),
           "Before _UpdateSegmentContext, logGroupId {}, segmentInfos[{}].GetValidBlockCount() = {}, validBlockCountDiff {}, sum {}",
            logGroupId, segmentId, getValidCount, validBlockCountDiff, result);

        segmentInfos[segmentId].SetValidBlockCount(result);

        if (0 > (int)getValidCount)
        {
            POS_TRACE_ERROR(EID(JOURNAL_INVALID),
                "After update underflow occurred, logGroupId {}, segmentInfos[{}].GetValidBlockCount() = {}, validBlockCountDiff {}",
                logGroupId, segmentId, getValidCount, validBlockCountDiff);
            assert(false);
        }
    }

    tbb::concurrent_unordered_map<SegmentId, uint32_t> changedOccupiedCount = targetSegInfo->GetChangedOccupiedStripeCount();
    for (auto it = changedOccupiedCount.begin(); it != changedOccupiedCount.end(); it++)
    {
        auto segmentId = it->first;
        auto occupiedStripeCountDiff = it->second;

        segmentInfos[segmentId].SetOccupiedStripeCount(segmentInfos[segmentId].GetOccupiedStripeCount() + occupiedStripeCountDiff);
    }
}

SegmentInfo*
VersionedSegmentCtx::GetUpdatedInfoToFlush(int logGroupId)
{
    _CheckLogGroupIdValidity(logGroupId);

    if (ALL_LOG_GROUP == logGroupId)
    {
        for (int id = 0; id < GetNumLogGroups(); id++)
        {
            _UpdateSegmentContext(id);
        }
    }
    else
    {
        _UpdateSegmentContext(logGroupId);
    }

    POS_TRACE_INFO(EID(JOURNAL_CHECKPOINT_IN_PROGRESS), "Versioned segment info to flush is constructed, logGroup {}", logGroupId);

    return segmentInfos;
}

void
VersionedSegmentCtx::ResetFlushedInfo(int logGroupId)
{
    _CheckLogGroupIdValidity(logGroupId);
    segmentInfoDiffs[logGroupId]->Reset();

    POS_TRACE_INFO(EID(JOURNAL_CHECKPOINT_COMPLETED), "Versioned segment info is flushed, logGroup {}", logGroupId);
}

int
VersionedSegmentCtx::GetNumSegments(void)
{
    return numSegments;
}

int
VersionedSegmentCtx::GetNumLogGroups(void)
{
    return config->GetNumLogGroups();
}

void
VersionedSegmentCtx::_CheckLogGroupIdValidity(int logGroupId)
{
    if (logGroupId >= config->GetNumLogGroups())
    {
        POS_TRACE_ERROR(EID(JOURNAL_INVALID),
            "Failed to check logGroupId validity, logGroupId {} is invalid", logGroupId);
        assert(false);
    }
}

void
VersionedSegmentCtx::_CheckSegIdValidity(int segId)
{
    if (segId >= (int)numSegments)
    {
        POS_TRACE_ERROR(EID(JOURNAL_INVALID),
            "Failed to check segId validity, segId {} is invalid", segId);
        assert(false);
    }
}
} // namespace pos
