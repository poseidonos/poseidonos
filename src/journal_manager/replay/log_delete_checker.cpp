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

#include "src/journal_manager/log/log_event.h"
#include "src/journal_manager/replay/log_delete_checker.h"

namespace pos
{
// Constructor for injecting deletedVolumes for unit test
LogDeleteChecker::LogDeleteChecker(std::vector<DeletedVolume> input)
: deletedVolumes(input)
{
}

void
LogDeleteChecker::Update(std::vector<ReplayLog>& replayLogs)
{
    for (auto replayLog : replayLogs)
    {
        VolumeDeletedLog* log = reinterpret_cast<VolumeDeletedLog*>((replayLog.log)->GetData());
        DeletedVolume deletion = {
            .volumeId = log->volId,
            .time = replayLog.time,
            .prevSegInfoVersion = log->allocatorContextVersion};
        deletedVolumes.push_back(deletion);
    }
}

void
LogDeleteChecker::ReplayedUntil(uint64_t time, int volumeId)
{
    for (auto it = deletedVolumes.begin(); it != deletedVolumes.end();)
    {
        if ((it->time < time) && (volumeId == it->volumeId))
        {
            it = deletedVolumes.erase(it);
        }
        else
        {
            it++;
        }
    }
}

bool
LogDeleteChecker::IsDeleted(int volumeId)
{
    for (auto volume : deletedVolumes)
    {
        if (volume.volumeId == volumeId)
        {
            return true;
        }
    }
    return false;
}

std::vector<DeletedVolume>
LogDeleteChecker::GetDeletedVolumes(void)
{
    return deletedVolumes;
}
} // namespace pos
