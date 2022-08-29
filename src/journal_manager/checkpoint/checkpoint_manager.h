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

#pragma once

#include <atomic>
#include <mutex>
#include <queue>

#include "src/include/smart_ptr_type.h"
#include "src/journal_manager/checkpoint/checkpoint_handler.h"
#include "src/mapper/include/mpage_info.h"

namespace pos
{
class IMapFlush;
class IContextManager;
class EventScheduler;

class DirtyMapManager;
class CheckpointHandler;
class TelemetryPublisher;

class CheckpointManager
{
public:
    CheckpointManager(void);
    CheckpointManager(const int arrayId);
    explicit CheckpointManager(CheckpointHandler* cpHandler, const int arrayId = 0);
    virtual ~CheckpointManager(void);

    virtual void Init(IMapFlush* mapFlush, IContextManager* ctxManager,
        EventScheduler* scheduler, DirtyMapManager* dMapManager, TelemetryPublisher* tp);
    virtual int RequestCheckpoint(int logGroupId, EventSmartPtr callback);
    virtual int StartCheckpoint(EventSmartPtr callback);

    virtual CheckpointStatus GetStatus(void);

    virtual void CheckpointCompleted(void);

    virtual void BlockCheckpointAndWaitToBeIdle(void);
    virtual void UnblockCheckpoint(void);

    bool IsCheckpointInProgress(void);
    bool IsCheckpointBlocked(void);
    int GetNumPendingCheckpointRequests(void);

private:
    struct CheckpointRequest
    {
        int groupId;
        EventSmartPtr callback;
    };
    CheckpointRequest invalid{INT32_MAX, nullptr};

    int _TryToStartCheckpoint(CheckpointRequest request);
    int _StartCheckpoint(CheckpointRequest request);
    void _AddToPendingRequestList(CheckpointRequest request);
    void _CompleteCheckpoint(void);

    bool _HasPendingRequests(void);
    void _StartPendingRequests(void);

    bool _GetNextRequest(CheckpointRequest& request);

    EventScheduler* eventScheduler;
    DirtyMapManager* dirtyMapManager;
    CheckpointHandler* checkpointHandler;

    std::mutex checkpointTriggerLock;
    std::atomic<bool> checkpointInProgress;
    std::atomic<bool> checkpointBlocked;
    EventSmartPtr clientCallback;
    std::queue<CheckpointRequest> checkpointRequests;
    TelemetryPublisher* telemetryPublisher;
    int arrayId;

    const int ALL_LOG_GROUP = -1;
};

} // namespace pos
