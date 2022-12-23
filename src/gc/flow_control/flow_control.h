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
#include <tuple>
#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <string>

#include "src/lib/singleton.h"
#include "src/array_models/interface/i_mount_sequence.h"
#include "src/gc/flow_control/flow_control_configuration.h"

#include "src/debug_lib/debug_info_maker.h"
#include "src/debug_lib/debug_info_maker.hpp"
#include "src/debug_lib/debug_info_queue.h"
#include "src/debug_lib/debug_info_queue.hpp"

namespace pos
{

enum FlowControlType : uint32_t
{
    USER = 0,
    GC,
    MAX_FLOW_CONTROL_TYPE
};

enum FlowControlState : uint32_t
{
    USER_ONLY = 0,
    FLOW_CONTROL_TARGET,
    FLOW_CONTROL_URGENT,
    GC_ONLY,
    MAX_FLOW_CONTROL_STATE
};

class IContextManager;
class SystemTimeoutChecker;
class FlowControlService;
class IArrayInfo;
class PartitionLogicalSize;
class TokenDistributer;

class DebugFlowControl : public DebugInfoInstance
{
public:
    int bucket[FlowControlType::MAX_FLOW_CONTROL_TYPE];
    int previousBucket[FlowControlType::MAX_FLOW_CONTROL_TYPE];

    uint32_t totalToken;
    uint32_t totalTokenInStripe;
    uint32_t totalSegments;
    uint32_t gcThreshold;
    uint32_t gcUrgentThreshold;
    uint32_t freeSegments;

    uint32_t targetPercent;
    uint32_t urgentPercent;
    uint32_t targetSegment;
    uint32_t urgentSegment;

    uint64_t forceResetTimeout;
    uint32_t arrayId = 0;
};

class FlowControl : public IMountSequence, public DebugInfoMaker<DebugFlowControl>
{
public:
    explicit FlowControl(IArrayInfo* arrayInfo);
    FlowControl(IArrayInfo* arrayInfo,
                IContextManager* inputIContextManager,
                SystemTimeoutChecker* inputSystemTimeoutChecker,
                FlowControlService* inputFlowControlService,
                TokenDistributer* inputTokenDistributer,
                FlowControlConfiguration* inputFlowControlConfiguration);
    virtual ~FlowControl(void);

    virtual int Init(void) override;
    virtual void Dispose(void) override;
    virtual void Shutdown(void) override;
    virtual void Flush(void) override;

    virtual int GetToken(FlowControlType type, int token);
    virtual void ReturnToken(FlowControlType type, int token);
    virtual void InitDistributer(void);
    virtual void Reset(void);
    virtual void MakeDebugInfo(DebugFlowControl& obj) final;

private:
    bool _RefillToken(FlowControlType type);
    bool _TryForceResetToken(FlowControlType type);
    std::tuple<uint32_t, uint32_t> _DistributeToken(void);
    void _ReadConfig(void);

    IArrayInfo* arrayInfo = nullptr;

    const uint64_t NANOS_PER_MSEC = 1000000ULL; // 1 sec

    const std::string FLOW_CONTROL_STRATEGY_NAME[(int)FlowControlStrategy::MAX_FLOW_CONTROL_STRATEGY] = {
        "disable", "linear", "state"};

    FlowControlStrategy flowControlStrategy = FlowControlStrategy::LINEAR;

    bool isForceReset = false;

    std::atomic<int> bucket[FlowControlType::MAX_FLOW_CONTROL_TYPE];
    std::atomic<int> previousBucket[FlowControlType::MAX_FLOW_CONTROL_TYPE];

    uint32_t blksPerStripe = 0;
    uint32_t stripesPerSegment = 0;
    uint32_t totalToken = 0;
    uint32_t totalTokenInStripe = 0;
    uint32_t totalSegments = 0;
    uint32_t gcThreshold = 0;
    uint32_t gcUrgentThreshold = 0;
    uint32_t freeSegments = 0;

    uint32_t targetPercent = 30;
    uint32_t urgentPercent = 10;
    uint32_t targetSegment = 10;
    uint32_t urgentSegment = 5;

    uint64_t forceResetTimeout = NANOS_PER_MSEC * 1000;

    std::mutex refillTokenMutex;

    IContextManager* iContextManager = nullptr;
    const PartitionLogicalSize* sizeInfo = nullptr;
    SystemTimeoutChecker* systemTimeoutChecker = nullptr;
    FlowControlService* flowControlService = nullptr;
    TokenDistributer* tokenDistributer = nullptr;
    FlowControlConfiguration* flowControlConfiguration = nullptr;

    // Debug Information
    DebugFlowControl debugFlowControl;
    DebugInfoQueue<DebugFlowControl> flowControlQueue;
};
} // namespace pos
