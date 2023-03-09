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
  segmentInfoData(nullptr)
{
}

VersionedSegmentCtx::~VersionedSegmentCtx(void)
{
    Dispose();
}

void
VersionedSegmentCtx::Init(JournalConfiguration* journalConfiguration, uint32_t numSegments_, uint32_t numStripesPerSegment)
{
    _Init(journalConfiguration, numSegments_);

    for (int index = 0; index < config->GetNumLogGroups(); index++)
    {
        std::shared_ptr<VersionedSegmentInfo> segmentInfo(new VersionedSegmentInfo(numStripesPerSegment));
        segmentInfoDiffs.push_back(segmentInfo);
    }
}

void
VersionedSegmentCtx::Init(JournalConfiguration* journalConfiguration, uint32_t numSegments_,
    std::vector<std::shared_ptr<VersionedSegmentInfo>> inputVersionedSegmentInfo)
{
    _Init(journalConfiguration, numSegments_);

    assert((int)inputVersionedSegmentInfo.size() == config->GetNumLogGroups());
    segmentInfoDiffs = inputVersionedSegmentInfo;
}

void
VersionedSegmentCtx::_Init(JournalConfiguration* journalConfiguration, uint32_t numSegments_)
{
    config = journalConfiguration;

    numSegments = numSegments_;
    segmentInfoData = new SegmentInfoData[numSegments];
    for (uint32_t segId = 0; segId < numSegments; segId++)
    {
        segmentInfoData[segId].Set(0, 0, SegmentState::FREE);
    }
}

void
VersionedSegmentCtx::Load(SegmentInfoData* loadedSegmentInfos)
{
    if (nullptr != loadedSegmentInfos)
    {
        for (uint32_t segId = 0; segId < numSegments; segId++)
        {
            segmentInfoData[segId].validBlockCount = loadedSegmentInfos[segId].validBlockCount.load();
            segmentInfoData[segId].occupiedStripeCount = loadedSegmentInfos[segId].occupiedStripeCount.load();
            segmentInfoData[segId].state = loadedSegmentInfos[segId].state;
            POS_TRACE_INFO(EID(JOURNAL_MANAGER_INITIALIZED), "Loaded segment: segId {}, validcnt {}, stripeCnt {}, state {}",
                segId, loadedSegmentInfos[segId].validBlockCount,
                loadedSegmentInfos[segId].occupiedStripeCount,
                loadedSegmentInfos[segId].state);
        }
    }
    else
    {
        // For Unit Test
        // Test should provide loadedSegmentInfo, but some are not, so initialize it here temporally
        POS_TRACE_INFO(EID(JOURNAL_MANAGER_INITIALIZED), "Unable to load Versioned Segment Context because a loaded segment context does not exist.");
    }
}

void
VersionedSegmentCtx::Dispose(void)
{
    if (segmentInfoData != nullptr)
    {
        delete[] segmentInfoData;
        segmentInfoData = nullptr;
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
    _UpdateSegmentContext(segmentInfoDiffs[logGroupId].get());
}

void
VersionedSegmentCtx::_UpdateSegmentContext(VersionedSegmentInfo* targetSegInfo)
{
    tbb::concurrent_unordered_map<SegmentId, tbb::atomic<int>> changedValidBlkCount = targetSegInfo->GetChangedValidBlockCount();
    for (auto it = changedValidBlkCount.begin(); it != changedValidBlkCount.end(); it++)
    {
        auto segmentId = it->first;
        auto validBlockCountDiff = it->second;

        uint32_t getValidCount = segmentInfoData[segmentId].validBlockCount;
        uint32_t result = getValidCount + validBlockCountDiff;

        POS_TRACE_DEBUG(EID(JOURNAL_DEBUG),
            "Before _UpdateSegmentContext, segmentInfos[{}].GetValidBlockCount() = {}, validBlockCountDiff {}, sum {}",
            segmentId, getValidCount, validBlockCountDiff, result);

        segmentInfoData[segmentId].validBlockCount = result;

        if (0 > (int)getValidCount)
        {
            POS_TRACE_ERROR(EID(JOURNAL_INVALID),
                "After update underflow occurred, segmentInfos[{}].GetValidBlockCount() = {}, validBlockCountDiff {}",
                segmentId, getValidCount, validBlockCountDiff);
            assert(false);
        }
    }

    tbb::concurrent_unordered_map<SegmentId, tbb::atomic<uint32_t>> changedOccupiedCount = targetSegInfo->GetChangedOccupiedStripeCount();
    for (auto it = changedOccupiedCount.begin(); it != changedOccupiedCount.end(); it++)
    {
        auto segmentId = it->first;
        auto occupiedStripeCountDiff = it->second;

        segmentInfoData[segmentId].occupiedStripeCount += occupiedStripeCountDiff;
    }
}

SegmentInfoData*
VersionedSegmentCtx::GetUpdatedInfoDataToFlush(int logGroupId)
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

    return segmentInfoData;
}

SegmentInfoData*
VersionedSegmentCtx::GetUpdatedInfoDataToFlush(VersionedSegmentInfo* info)
{
    _UpdateSegmentContext(info);

    POS_TRACE_INFO(EID(JOURNAL_CHECKPOINT_IN_PROGRESS), "Versioned segment info to flush is constructed using provided info");

    return segmentInfoData;
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

void
VersionedSegmentCtx::LogFilled(int logGroupId, const MapList& dirty)
{
    // do nothing
}

void 
VersionedSegmentCtx::LogBufferReseted(int logGroupId)
{    
    _CheckLogGroupIdValidity(logGroupId);
    segmentInfoDiffs[logGroupId]->Reset();

    POS_TRACE_INFO(EID(VERSIONED_SEGMENT_INFO), "Versioned segment info is flushed, logGroupId:{}", logGroupId);
}

void
VersionedSegmentCtx::NotifySegmentFreed(SegmentId segmentId)
{
    for (int groupId = 0; groupId < config->GetNumLogGroups(); groupId++)
    {
        segmentInfoDiffs[groupId]->ResetOccupiedStripeCount(segmentId);
        segmentInfoDiffs[groupId]->ResetValidBlockCount(segmentId);
    }
    segmentInfoData[segmentId].occupiedStripeCount = 0;
    segmentInfoData[segmentId].state = SegmentState::FREE;
}

} // namespace pos
