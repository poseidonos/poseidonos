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

#include <tuple>
#include <string>
#include "src/lib/singleton.h"
#include "src/gc/flow_control/flow_control_configuration.h"
namespace pos
{

enum FlowControlStrategy : uint32_t
{
    DISABLE = 0,
    LINEAR,
    STATE,
    MAX_FLOW_CONTROL_STRATEGY
};

class IArrayInfo;
class PartitionLogicalSize;
class TokenDistributer;
class ConfigManager;

class FlowControlConfiguration
{
public:
    explicit FlowControlConfiguration(IArrayInfo* arrayInfo);
    FlowControlConfiguration(IArrayInfo* arrayInfo,
                ConfigManager* inputConfigManager);
    virtual ~FlowControlConfiguration(void);
    virtual void ReadConfig(void);
    virtual bool GetEnable(void);
    virtual uint64_t GetForceResetTimeout(void);
    virtual uint32_t GetTotalToken(void);
    virtual uint32_t GetTotalTokenInStripe(void);
    virtual FlowControlStrategy GetFlowControlStrategy(void);
    virtual uint32_t GetTargetPercent(void);
    virtual uint32_t GetUrgentPercent(void);
    virtual uint32_t GetTargetSegment(void);
    virtual uint32_t GetUrgentSegment(void);

private:
    bool _IsEnabled(void);
    bool _IsDefaultSetting(void);
    uint64_t _ReadTimeoutInMsec(void);
    uint32_t _ReadTotalTokenInStripe(void);
    FlowControlStrategy _ReadFlowControlStrategy(void);
    std::tuple<bool, uint32_t, uint32_t, uint32_t, uint32_t> _ReadStateConfig(void);

    IArrayInfo* arrayInfo = nullptr;

    const uint64_t NANOS_PER_MSEC = 1000000ULL; // 1 sec
    const std::string FLOW_CONTROL_STRATEGY_NAME[(int)FlowControlStrategy::MAX_FLOW_CONTROL_STRATEGY] = {
        "disable", "linear", "state"};

    const uint64_t DEFAULT_FORCE_RESET_TIMEOUT = 1000 * NANOS_PER_MSEC;
    const uint32_t DEFAULT_TOTAL_TOKEN_IN_STRIPE = 1024;
    const FlowControlStrategy DEFAULT_FLOW_CONTROL_STRATEGY = FlowControlStrategy::LINEAR;

    ConfigManager* configManager;
    const PartitionLogicalSize* sizeInfo;

    bool enable;
    uint64_t forceResetTimeout;
    uint32_t totalTokenInStripe;
    uint32_t totalToken;
    FlowControlStrategy flowControlStrategy;
    uint32_t targetPercent;
    uint32_t urgentPercent;
    uint32_t targetSegment;
    uint32_t urgentSegment;
};
} // namespace pos
