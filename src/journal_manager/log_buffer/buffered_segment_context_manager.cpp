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

#include "buffered_segment_context_manager.h"

#include "buffered_segment_context.h"
#include "src/include/pos_event_id.h"
#include "src/journal_manager/config/journal_configuration.h"
#include "src/logger/logger.h"

namespace pos
{
BufferedSegmentContextManager::BufferedSegmentContextManager(void)
: config(nullptr),
  bufferedSegCtx(nullptr),
  numLogGroups(0)
{
}

BufferedSegmentContextManager::~BufferedSegmentContextManager(void)
{
    if (bufferedSegCtx != nullptr)
    {
        for (uint32_t index = 0; index < numLogGroups; index++)
        {
            if (bufferedSegCtx[index] != nullptr)
            {
                delete bufferedSegCtx[index];
            }
        }
        delete[] bufferedSegCtx;
    }
}

void
BufferedSegmentContextManager::Init(JournalConfiguration* journalConfiguration)
{
    config = journalConfiguration;
    numLogGroups = config->GetNumLogGroups();
    bufferedSegCtx = new BufferedSegmentContext*[numLogGroups];
    for (uint32_t index = 0; index < numLogGroups; index++)
    {
        bufferedSegCtx[index] = new BufferedSegmentContext;
    }
}

void
BufferedSegmentContextManager::Init(JournalConfiguration* journalConfiguration, BufferedSegmentContext** inputBufferedSegmentContext)
{
    config = journalConfiguration;
    numLogGroups = config->GetNumLogGroups();
    bufferedSegCtx = inputBufferedSegmentContext;
}

void
BufferedSegmentContextManager::IncreaseValidBlockCount(uint32_t logGroupId, SegmentId segId, uint32_t cnt)
{
    BufferedSegmentContext* targetSegCtx = bufferedSegCtx[logGroupId];
    targetSegCtx->IncreaseValidBlockCount(segId, cnt);
    // TODO (cheolho.kang): Add ISegmentContext method after introduced
}

void
BufferedSegmentContextManager::DecreaseValidBlockCount(uint32_t logGroupId, SegmentId segId, uint32_t cnt)
{
    BufferedSegmentContext* targetSegCtx = bufferedSegCtx[logGroupId];
    targetSegCtx->DecreaseValidBlockCount(segId, cnt);
    // TODO (cheolho.kang): Add ISegmentContext method after introduced
}

void
BufferedSegmentContextManager::IncreaseOccupiedStripeCount(uint32_t logGroupId, SegmentId segId)
{
    BufferedSegmentContext* targetSegCtx = bufferedSegCtx[logGroupId];
    targetSegCtx->IncreaseOccupiedStripeCount(segId);
    // TODO (cheolho.kang): Add ISegmentContext method after introduced
}

void
BufferedSegmentContextManager::UpdateSegmentContext(uint32_t logGroupId)
{
    BufferedSegmentContext* targetSegCtx = bufferedSegCtx[logGroupId];
    std::unordered_map<uint32_t, int> changedValidBlkCount = targetSegCtx->GetChangedValidBlockCount();
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

    std::unordered_map<uint32_t, uint32_t> changedOccupiedCount = targetSegCtx->GetChangedOccupiedStripeCount();
    for (auto it = changedOccupiedCount.begin(); it != changedOccupiedCount.end(); it++)
    {
        if (it->second > 0)
        {
            // TODO (cheolho.kang): Add ISegmentContext method after introduced
        }
    }
    targetSegCtx->Reset();
}

// TODO (cheolho.kang): Change return type to ISegmentContext
BufferedSegmentContext*
BufferedSegmentContextManager::GetSegmentContext(uint32_t logGroupId)
{
    if (logGroupId >= numLogGroups)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::JOURNAL_INVALID,
            "Failed to get buffered segment context, Invalid log gorup ID: {}, Maximun log group ID: {}", logGroupId, numLogGroups);
        return nullptr;
    }
    return bufferedSegCtx[logGroupId];
}

} // namespace pos
