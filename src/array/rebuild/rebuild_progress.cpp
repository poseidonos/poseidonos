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

#include "rebuild_progress.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

#include <utility>

namespace pos
{
RebuildProgress::RebuildProgress(string name)
{
    arrayName = name;
    POS_REPORT_TRACE(EID(REBUILD_PROGRESS), "[0], {}", arrayName);
}

RebuildProgress::~RebuildProgress(void)
{
    for (auto it : progress)
    {
        delete it.second;
    }
}

void
RebuildProgress::Update(string _id, uint32_t _done, uint32_t _total)
{
    POS_TRACE_DEBUG(EID(REBUILD_PROGRESS_DETAIL),
        "array:{}, id:{}, done:{}, total:{}", arrayName, _id, _done, _total);

    auto it = progress.find(_id);
    if (it == progress.end())
    {
        PartitionProgress* partProg = new PartitionProgress();
        partProg->total = _total;
        partProg->done = 0;
        progress.insert(make_pair(_id, partProg));
    }
    else
    {
        uint32_t increment = it->second->total - _total;

        if (increment > 0 && _done == 0)
        {
            // cases where the progress is increasing as the total decreases
            it->second->done = increment;
        }
        else
        {
            it->second->total = _total;
            it->second->done = _done;
        }
    }

    uint32_t now = Current();

    if (percent != now)
    {
        percent = now;
        POS_REPORT_TRACE(EID(REBUILD_PROGRESS), "[{}], {}", percent, arrayName);
    }
}

uint32_t RebuildProgress::Current(void)
{
    uint32_t total = 0;
    uint32_t done = 0;

    for (auto it : progress)
    {
        PartitionProgress* partProg = it.second;
        total += partProg->total;
        done += partProg->done;
    }

    POS_TRACE_DEBUG(EID(REBUILD_PROGRESS_DETAIL),
        "rebuilding in progress, num_of_tasks:{}, array:{}, done:{}, total:{}", progress.size(), arrayName, done, total);

    if (total == 0)
    {
        return 100;
    }
    return done * 100 / total;
}

} // namespace pos
