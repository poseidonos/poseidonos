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

#include "src/journal_manager/log_buffer/callback_sequence_controller.h"

namespace pos
{
// Constructor for product code
CallbackSequenceController::CallbackSequenceController(void)
: numCallbacksInExecution(0),
  checkpointTriggerInProgress(false)
{
}

// Constructor for injecting member variables in unit test
CallbackSequenceController::CallbackSequenceController(int numCallbacks, bool checkpointTriggered)
: CallbackSequenceController()
{
    numCallbacksInExecution = numCallbacks;
    checkpointTriggerInProgress = checkpointTriggered;
}

void
CallbackSequenceController::GetCallbackExecutionApproval(void)
{
    std::lock_guard<std::mutex> lock(sequenceLock);

    // Assume checkpoint trigger is completed in short time
    while (checkpointTriggerInProgress == true)
    {
    }

    numCallbacksInExecution++;
}

void
CallbackSequenceController::NotifyCallbackCompleted(void)
{
    numCallbacksInExecution--;
}

void
CallbackSequenceController::GetCheckpointExecutionApproval(void)
{
    std::lock_guard<std::mutex> lock(sequenceLock);

    // Assume callback execution is completed in short time
    while (numCallbacksInExecution != 0)
    {
    }

    checkpointTriggerInProgress = true;
}

void
CallbackSequenceController::AllowCallbackExecution(void)
{
    checkpointTriggerInProgress = false;
}

int
CallbackSequenceController::GetNumPendingCallbacks(void)
{
    return numCallbacksInExecution;
}

bool
CallbackSequenceController::IsCheckpointInProgress(void)
{
    return checkpointTriggerInProgress;
}

} // namespace pos
