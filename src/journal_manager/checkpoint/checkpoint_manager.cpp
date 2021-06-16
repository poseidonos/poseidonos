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

#include "src/journal_manager/checkpoint/checkpoint_manager.h"

#include "src/journal_manager/checkpoint/dirty_map_manager.h"
#include "src/journal_manager/log_buffer/callback_sequence_controller.h"

namespace pos
{
CheckpointManager::CheckpointManager(void)
: CheckpointManager(new CheckpointHandler())
{
}

CheckpointManager::CheckpointManager(CheckpointHandler* cpHandler)
: sequenceController(nullptr),
  checkpointHandler(cpHandler)
{
}

CheckpointManager::~CheckpointManager(void)
{
    if (checkpointHandler != nullptr)
    {
        delete checkpointHandler;
    }
}

void
CheckpointManager::Init(IMapFlush* mapFlush, IContextManager* ctxManager,
    EventScheduler* scheduler, CallbackSequenceController* seqController, DirtyMapManager* dMapManager)
{
    sequenceController = seqController;
    dirtyMapManager = dMapManager;

    checkpointHandler->Init(mapFlush, ctxManager, scheduler);
}

int
CheckpointManager::RequestCheckpoint(int logGroupId, EventSmartPtr callback)
{
    // TODO (huijeong.kim) Add checkpoint sequence controller here

    MapPageList dirtyPages;
    if (logGroupId == -1)
    {
        dirtyPages = dirtyMapManager->GetTotalDirtyList();
    }
    else
    {
        dirtyPages = dirtyMapManager->GetDirtyList(logGroupId);
    }

    sequenceController->GetCheckpointExecutionApproval();
    int ret = checkpointHandler->Start(dirtyPages, callback);
    sequenceController->AllowCallbackExecution();
    if (ret != 0)
    {
        // TODO(huijeong.kim): Go to the fail mode - not to journal any more
    }
        return ret;
}

CheckpointStatus
CheckpointManager::GetStatus(void)
{
    return checkpointHandler->GetStatus();
}

} // namespace pos
