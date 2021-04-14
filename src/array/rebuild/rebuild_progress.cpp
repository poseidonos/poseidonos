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

#include "rebuild_progress.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
RebuildProgress::RebuildProgress(void)
{
    POS_REPORT_TRACE((int)POS_EVENT_ID::REBUILD_PROGRESS, "[0]");
}

void
RebuildProgress::Update(string _id, uint64_t _done)
{
    POS_TRACE_DEBUG((int)POS_EVENT_ID::REBUILD_PROGRESS_DETAIL,
        "id:{}, done:{}", _id, _done);

    uint64_t delta = 0;
    auto it = progress.find(_id);
    if (it == progress.end())
    {
        progress.insert(make_pair(_id, _done));
        delta = _done;
    }
    else
    {
        delta = _done - progress[_id]; // assume incremental progress
        progress[_id] = _done;
    }

    done += delta;
    uint64_t now = done * 100 / total;

    if (percent != now)
    {
        percent = now;
        POS_REPORT_TRACE((int)POS_EVENT_ID::REBUILD_PROGRESS, "[{}]", percent);
    }
}

void RebuildProgress::SetTotal(uint64_t _total)
{
    total = _total;
}

uint64_t RebuildProgress::Current(void)
{
    return percent;
}

} // namespace pos
