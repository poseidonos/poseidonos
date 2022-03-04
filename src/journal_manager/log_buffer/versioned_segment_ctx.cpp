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

#include "versioned_segment_info.h"
#include "src/include/pos_event_id.h"
#include "src/journal_manager/config/journal_configuration.h"
#include "src/logger/logger.h"

namespace pos
{
VersionedSegmentCtx::VersionedSegmentCtx(void)
: config(nullptr),
  versionedSegInfo(nullptr),
  numLogGroups(0)
{
}

VersionedSegmentCtx::~VersionedSegmentCtx(void)
{
    Dispose();
}

void
VersionedSegmentCtx::Init(JournalConfiguration* journalConfiguration)
{
    config = journalConfiguration;
    numLogGroups = config->GetNumLogGroups();
    versionedSegInfo = new VersionedSegmentInfo*[numLogGroups];
    for (uint32_t index = 0; index < numLogGroups; index++)
    {
        versionedSegInfo[index] = new VersionedSegmentInfo;
    }
}

void
VersionedSegmentCtx::Init(JournalConfiguration* journalConfiguration, VersionedSegmentInfo** inputVersionedSegmentInfo)
{
    config = journalConfiguration;
    numLogGroups = config->GetNumLogGroups();
    versionedSegInfo = inputVersionedSegmentInfo;
}

void
VersionedSegmentCtx::Dispose(void)
{
    if (versionedSegInfo != nullptr)
    {
        for (uint32_t index = 0; index < numLogGroups; index++)
        {
            if (versionedSegInfo[index] != nullptr)
            {
                delete versionedSegInfo[index];
            }
        }
        delete[] versionedSegInfo;
        versionedSegInfo = nullptr;
    }
}

void
VersionedSegmentCtx::IncreaseValidBlockCount(uint32_t logGroupId, SegmentId segId, uint32_t cnt)
{
    VersionedSegmentInfo* targetSegInfo = versionedSegInfo[logGroupId];
    targetSegInfo->IncreaseValidBlockCount(segId, cnt);
    // TODO (cheolho.kang): Add ISegmentContext method after introduced
}

void
VersionedSegmentCtx::DecreaseValidBlockCount(uint32_t logGroupId, SegmentId segId, uint32_t cnt)
{
    VersionedSegmentInfo* targetSegInfo = versionedSegInfo[logGroupId];
    targetSegInfo->DecreaseValidBlockCount(segId, cnt);
    // TODO (cheolho.kang): Add ISegmentContext method after introduced
}

void
VersionedSegmentCtx::IncreaseOccupiedStripeCount(uint32_t logGroupId, SegmentId segId)
{
    VersionedSegmentInfo* targetSegInfo = versionedSegInfo[logGroupId];
    targetSegInfo->IncreaseOccupiedStripeCount(segId);
    // TODO (cheolho.kang): Add ISegmentContext method after introduced
}

void
VersionedSegmentCtx::UpdateSegmentContext(uint32_t logGroupId)
{
    VersionedSegmentInfo* targetSegInfo = versionedSegInfo[logGroupId];
    std::unordered_map<uint32_t, int> changedValidBlkCount = targetSegInfo->GetChangedValidBlockCount();
    for (auto it = changedValidBlkCount.begin(); it != changedValidBlkCount.end(); it++)
    {
        if (it->second > 0)
        {
            // TODO (cheolho.kang): Add ISegmentContext method after introduced
        }
        if (it->second < 0)
        {
            // TODO (cheolho.kang): Add ISegmentContext method after introduced
        }
    }

    std::unordered_map<uint32_t, uint32_t> changedOccupiedCount = targetSegInfo->GetChangedOccupiedStripeCount();
    for (auto it = changedOccupiedCount.begin(); it != changedOccupiedCount.end(); it++)
    {
        if (it->second > 0)
        {
            // TODO (cheolho.kang): Add ISegmentContext method after introduced
        }
    }
    targetSegInfo->Reset();
}

// TODO (cheolho.kang): Change return type to ISegmentContext
VersionedSegmentInfo*
VersionedSegmentCtx::GetSegmentInfo(uint32_t logGroupId)
{
    if (logGroupId >= numLogGroups)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_INVALID,
            "Failed to get buffered segment context, Invalid log gorup ID: {}, Maximun log group ID: {}", logGroupId, numLogGroups);
        return nullptr;
    }
    return versionedSegInfo[logGroupId];
}

} // namespace pos
