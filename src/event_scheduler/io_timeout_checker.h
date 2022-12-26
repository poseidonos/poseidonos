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

#include "src/lib/singleton.h"
#include "src/event_scheduler/callback_type.h"
#include "src/event_scheduler/publish_pending_io.h"

#include "src/debug_lib/debug_info_maker.h"
#include "src/debug_lib/debug_info_maker.hpp"
#include "src/debug_lib/debug_info_queue.h"
#include "src/debug_lib/debug_info_queue.hpp"

#include <vector>

namespace pos
{

const uint32_t CHECK_RESOLUTION_RANGE = 36000; // 1 hour
const uint32_t TIMER_RESOLUTION_MS = 100;       // 100 ms
const uint32_t ONE_SEC_IN_MS = 1000;       // 1 s
const uint32_t CHECK_TIMEOUT_THRESHOLD = 30;    // 3s
class TelemetryPublisher;

struct PendingIo
{
    std::atomic<std::int64_t> pendingIoCnt[CHECK_RESOLUTION_RANGE];
    std::atomic<std::uint64_t> oldestIdx;
};

class DebugIoTimeoutChecker : public DebugInfoInstance
{
public:
    class TimeoutInfo
    {
    public:
        CallbackType callbackType;
        float ioDelaySec;
    } timeoutInfo[Total_CallbackType_Cnt];
    uint64_t currentIdx;
};

class IoTimeoutChecker : public DebugIoTimeoutChecker, public DebugInfoMaker<DebugIoTimeoutChecker>
{
public:
    IoTimeoutChecker(void);
    ~IoTimeoutChecker(void);

    void Initialize(void);

    void IncreasePendingCnt(CallbackType callbackType, uint64_t pendingTime);
    void DecreasePendingCnt(CallbackType callbackType, uint64_t pendingTime);

    bool FindPendingIo(CallbackType callbackType);
    void GetPendingIoCount(CallbackType callbackType, std::vector<int> &pendingIoCnt);

    void MoveOldestIdx(CallbackType callbackType);
    void MoveCurrentIdx(uint64_t pendingTime);

    uint64_t GetCurrentRoughTime(void);
    virtual void MakeDebugInfo(DebugIoTimeoutChecker& obj) final;

private:

    bool _CheckPeningOverTime(CallbackType callbackType);    

    bool initialize;
    PublishPendingIo* publisher;
    std::atomic<std::uint64_t> currentIdx;

    PendingIo pendingIoCnt[CallbackType::Total_CallbackType_Cnt];
    TelemetryPublisher* telemetryPublisher;
    DebugIoTimeoutChecker debugIoTimeoutChecker;
    DebugInfoQueue<DebugIoTimeoutChecker> debugIoTimeoutCheckerQueue;
};

using IoTimeoutCheckerSingleton = Singleton<IoTimeoutChecker>;
}
