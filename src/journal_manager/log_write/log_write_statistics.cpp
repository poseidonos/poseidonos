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

#include "src/journal_manager/log_write/log_write_statistics.h"

#include "src/journal_manager/log/log_handler.h"
#include "src/journal_manager/statistics/stripe_info.h"
#include "src/journal_manager/statistics/stripe_log_write_status.h"

namespace pos
{
LogWriteStatistics::LogWriteStatistics(void)
: enabled(false)
{
}

void
LogWriteStatistics::Init(int numLogGroups)
{
    contextAdded.resize(numLogGroups);
    stripeLogsAdded.resize(numLogGroups);

    enabled = true;
}

void
LogWriteStatistics::Dispose(void)
{
    for (int groupId = 0; groupId < (int)stripeLogsAdded.size(); groupId++)
    {
        _Reset(groupId);
    }
}

LogWriteStatistics::~LogWriteStatistics(void)
{
    for (int groupId = 0; groupId < (int)stripeLogsAdded.size(); groupId++)
    {
        _Reset(groupId);
    }
}

void
LogWriteStatistics::_Reset(int groupId)
{
    _ResetContextList(groupId);
    _ResetStripeStatus(groupId);
}

void
LogWriteStatistics::_ResetContextList(int groupId)
{
    std::lock_guard<std::mutex> lock(contextLock);
    for (auto context : contextAdded[groupId])
    {
        delete context;
    }
    contextAdded[groupId].clear();
}

void
LogWriteStatistics::_ResetStripeStatus(int groupId)
{
    std::lock_guard<std::mutex> lock(stripeStatusLock);
    for (auto stripeStats : stripeLogsAdded[groupId])
    {
        delete stripeStats;
    }
    stripeLogsAdded[groupId].clear();
}

void
LogWriteStatistics::AddToList(LogWriteContext* context)
{
    if (enabled == true)
    {
        std::lock_guard<std::mutex> lock(contextLock);
        contextAdded[context->GetLogGroupId()].push_back(context);
    }
}

bool
LogWriteStatistics::UpdateStatus(LogWriteContext* context)
{
    if (enabled == true)
    {
        int groupId = context->GetLogGroupId();
        LogHandlerInterface* log = context->GetLog();

        if (log->GetType() == LogType::BLOCK_WRITE_DONE)
        {
            StripeLogWriteStatus* stripeStats = _FindStripeLogs(groupId, log->GetVsid());
            BlockWriteDoneLog dat = *(reinterpret_cast<BlockWriteDoneLog*>(log->GetData()));
            stripeStats->BlockLogFound(dat);

            return true;
        }
        else if (log->GetType() == LogType::STRIPE_MAP_UPDATED)
        {
            StripeLogWriteStatus* stripeStats = _FindStripeLogs(groupId, log->GetVsid());
            StripeMapUpdatedLog dat = *(reinterpret_cast<StripeMapUpdatedLog*>(log->GetData()));
            stripeStats->StripeLogFound(dat);

            return true;
        }
    }
    return false;
}

void
LogWriteStatistics::PrintStats(int groupId)
{
    if (enabled == false)
    {
        return;
    }

    for (auto stripeStats : stripeLogsAdded[groupId])
    {
        stripeStats->Print();
    }
    _Reset(groupId);
}

StripeLogWriteStatus*
LogWriteStatistics::_FindStripeLogs(int groupId, StripeId vsid)
{
    std::lock_guard<std::mutex> lock(stripeStatusLock);

    for (auto stripeStats : stripeLogsAdded[groupId])
    {
        if (stripeStats->IsFlushed() == false && stripeStats->GetVsid() == vsid)
        {
            return stripeStats;
        }
    }

    StripeLogWriteStatus* stripeStats = new StripeLogWriteStatus(vsid);
    stripeLogsAdded[groupId].push_back(stripeStats);

    return stripeStats;
}

bool
LogWriteStatistics::IsEnabled(void)
{
    return enabled;
}

} // namespace pos
