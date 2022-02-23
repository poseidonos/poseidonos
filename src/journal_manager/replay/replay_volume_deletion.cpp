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

#include "src/journal_manager/replay/replay_volume_deletion.h"
#include "src/journal_manager/replay/log_delete_checker.h"

#include "src/allocator/i_context_manager.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

#include <string>
#include <vector>

namespace pos
{
ReplayVolumeDeletion::ReplayVolumeDeletion(LogDeleteChecker* logDeleteChecker,
    IContextManager* contextManager, IVolumeManager* volumeManager, ReplayProgressReporter* reporter)
: ReplayTask(reporter),
  deleteChecker(logDeleteChecker),
  contextManager(contextManager),
  volumeManager(volumeManager)
{
}

int
ReplayVolumeDeletion::Start(void)
{
    int result = 0;
    uint64_t storedContextVersion = contextManager->GetStoredContextVersion(SEGMENT_CTX);
    std::vector<DeletedVolume> deletedVolumes = deleteChecker->GetDeletedVolumes();
    for (auto vol : deletedVolumes)
    {
        // Reused volume is not in the list
        // If context version in the log is same with stored version, do not delete the volume
        // If context version is increased after log is written, volume should be deleted
        if (vol.prevSegInfoVersion < storedContextVersion)
        {
            int result = volumeManager->CheckVolumeValidity(vol.volumeId);
            if (result == static_cast<int>(POS_EVENT_ID::SUCCESS))
            {
                std::string volname;
                volumeManager->VolumeName(vol.volumeId, volname);
                result = volumeManager->Delete(volname);

                if (result == 0)
                {
                    POS_TRACE_INFO(POS_EVENT_ID::JOURNAL_REPLAY_VOLUME_EVENT,
                        "[Replay] volume {} is deleted", vol.volumeId);
                }
                else
                {
                    POS_TRACE_INFO(POS_EVENT_ID::JOURNAL_REPLAY_VOLUME_EVENT,
                        "[Replay] volume {} delete failed", vol.volumeId);
                }

                POS_TRACE_INFO(POS_EVENT_ID::JOURNAL_REPLAY_VOLUME_EVENT,
                    "[Replay] Volume delete replayed, prev ver {}, stored ver {}",
                    vol.prevSegInfoVersion, storedContextVersion);
            }
        }
    }

    reporter->SubTaskCompleted(GetId(), 1);
    return result;
}

ReplayTaskId
ReplayVolumeDeletion::GetId(void)
{
    return ReplayTaskId::REPLAY_VOLUME_DELETION;
}

int
ReplayVolumeDeletion::GetWeight(void)
{
    return 10;
}

int
ReplayVolumeDeletion::GetNumSubTasks(void)
{
    return 1;
}
} // namespace pos
