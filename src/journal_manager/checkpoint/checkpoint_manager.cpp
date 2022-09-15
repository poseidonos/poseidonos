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

#include "src/journal_manager/checkpoint/checkpoint_manager.h"

#include <cassert>

#include "src/event_scheduler/event_scheduler.h"
#include "src/include/pos_event_id.h"
#include "src/journal_manager/checkpoint/checkpoint_completion.h"
#include "src/journal_manager/checkpoint/dirty_map_manager.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
CheckpointManager::CheckpointManager(void)
: CheckpointManager(new CheckpointHandler(INT_MAX), INT_MAX)
{
}

CheckpointManager::CheckpointManager(const int arrayId)
: CheckpointManager(new CheckpointHandler(arrayId), arrayId)
{
}

CheckpointManager::CheckpointManager(CheckpointHandler* cpHandler, const int arrayId)
: eventScheduler(nullptr),
  dirtyMapManager(nullptr),
  checkpointHandler(cpHandler),
  checkpointInProgress(false),
  checkpointBlocked(false),
  clientCallback(nullptr),
  telemetryPublisher(nullptr),
  arrayId(arrayId)
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
    EventScheduler* scheduler, DirtyMapManager* dMapManager, TelemetryPublisher* tp)
{
    eventScheduler = scheduler;
    dirtyMapManager = dMapManager;
    telemetryPublisher = tp;

    checkpointHandler->Init(mapFlush, ctxManager, scheduler);
}

int
CheckpointManager::RequestCheckpoint(int logGroupId, EventSmartPtr cb)
{
    CheckpointRequest request = {
        .groupId = logGroupId,
        .callback = cb};

    std::lock_guard<std::mutex> lock(checkpointTriggerLock);
    int ret = 0;
    if ((_HasPendingRequests() == false) &&
        (checkpointBlocked == false))
    {
        ret = _TryToStartCheckpoint(request);
    }
    else
    {
        _AddToPendingRequestList(request);
    }
    return ret;
}

int
CheckpointManager::StartCheckpoint(EventSmartPtr cb)
{
    if (checkpointInProgress == true || checkpointBlocked != true)
    {
        POS_TRACE_DEBUG(EID(JOURNAL_CHECKPOINT_IN_PROGRESS),
            "Checkpoint cannot start right away, inProgress {} blocked {}, arrayId:{}",
            checkpointInProgress, checkpointBlocked, arrayId);
        return -1 * static_cast<int>(EID(JOURNAL_CHECKPOINT_IN_PROGRESS));
    }

    checkpointInProgress = true;
    CheckpointRequest request = {.groupId = ALL_LOG_GROUP, .callback = cb};
    return _StartCheckpoint(request);
}

bool
CheckpointManager::_HasPendingRequests(void)
{
    return (checkpointRequests.size() != 0);
}

void
CheckpointManager::_AddToPendingRequestList(CheckpointRequest request)
{
    checkpointRequests.push(request);
}

void
CheckpointManager::CheckpointCompleted(void)
{
    _CompleteCheckpoint();
    _StartPendingRequests();

    if (telemetryPublisher)
    {
        POSMetric metric(TEL36001_JRN_CHECKPOINT, POSMetricTypes::MT_GAUGE);
        metric.SetGaugeValue(0);
        telemetryPublisher->PublishMetric(metric);
    }
}

void
CheckpointManager::BlockCheckpointAndWaitToBeIdle(void)
{
    checkpointBlocked = true;

    while (checkpointInProgress == true)
    {
        std::this_thread::sleep_for(10ms);
    }
}

void
CheckpointManager::UnblockCheckpoint(void)
{
    checkpointBlocked = false;

    _StartPendingRequests();
}

bool
CheckpointManager::IsCheckpointInProgress(void)
{
    return checkpointInProgress;
}

bool
CheckpointManager::IsCheckpointBlocked(void)
{
    return checkpointBlocked;
}

int
CheckpointManager::GetNumPendingCheckpointRequests(void)
{
    std::lock_guard<std::mutex> lock(checkpointTriggerLock);
    return checkpointRequests.size();
}

void
CheckpointManager::_CompleteCheckpoint(void)
{
    eventScheduler->EnqueueEvent(clientCallback);
    clientCallback = nullptr;

    checkpointInProgress = false;
}

void
CheckpointManager::_StartPendingRequests(void)
{
    std::lock_guard<std::mutex> lock(checkpointTriggerLock);
    CheckpointRequest request;
    bool found = _GetNextRequest(request);
    if (found == true)
    {
        _TryToStartCheckpoint(request);
    }
}

bool
CheckpointManager::_GetNextRequest(CheckpointRequest& request)
{
    if (checkpointRequests.size() != 0)
    {
        auto nextRequest = checkpointRequests.front();
        checkpointRequests.pop();

        request = nextRequest;
        return true;
    }
    return false;
}

int
CheckpointManager::_TryToStartCheckpoint(CheckpointRequest request)
{
    int ret = 0;
    if (checkpointInProgress.exchange(true) == false)
    {
        ret = _StartCheckpoint(request);
    }
    else
    {
        _AddToPendingRequestList(request);
    }
    return ret;
}

int
CheckpointManager::_StartCheckpoint(CheckpointRequest request)
{
    assert(clientCallback == nullptr);

    if (telemetryPublisher)
    {
        POSMetric metric(TEL36001_JRN_CHECKPOINT, POSMetricTypes::MT_GAUGE);
        metric.SetGaugeValue(1);
        telemetryPublisher->PublishMetric(metric);
    }

    clientCallback = request.callback;
    EventSmartPtr completionEvent(new CheckpointCompletion(this));

    MapList dirtyMaps;
    if (ALL_LOG_GROUP == request.groupId)
    {
        dirtyMaps = dirtyMapManager->GetTotalDirtyList();
    }
    else
    {
        dirtyMaps = dirtyMapManager->GetDirtyList(request.groupId);
    }

    checkpointHandler->UpdateLogGroupInProgress(request.groupId);
    int ret = checkpointHandler->Start(dirtyMaps, completionEvent);
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
