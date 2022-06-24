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

#include "src/master_context/config_manager.h"
#include "src/include/pos_event_id.h"
#include "src/lib/system_timeout_checker.h"
#include "src/gc/flow_control/flow_control_configuration.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/logger/logger.h"
#include "src/array_models/dto/partition_logical_size.h"
#include "assert.h"

namespace pos
{

FlowControlConfiguration::FlowControlConfiguration(IArrayInfo* arrayInfo)
: FlowControlConfiguration(arrayInfo, ConfigManagerSingleton::Instance())
{
}

FlowControlConfiguration::FlowControlConfiguration(IArrayInfo* arrayInfo,
                        ConfigManager* inputConfigManager)
: arrayInfo(arrayInfo),
  configManager(inputConfigManager),
  enable(false),
  forceResetTimeout(0),
  totalTokenInStripe(0),
  totalToken(0),
  flowControlStrategy(FlowControlStrategy::LINEAR),
  targetPercent(0),
  urgentPercent(0),
  targetSegment(0),
  urgentSegment(0)
{
    sizeInfo = arrayInfo->GetSizeInfo(PartitionType::USER_DATA);
}

FlowControlConfiguration::~FlowControlConfiguration(void)
{
}

void
FlowControlConfiguration::ReadConfig(void)
{
    enable = _IsEnabled();
    if (false == enable)
    {
        flowControlStrategy = FlowControlStrategy::DISABLE;
        return;
    }

    bool isDefault = _IsDefaultSetting();
    if (isDefault)
    {
        forceResetTimeout = DEFAULT_FORCE_RESET_TIMEOUT;
        totalTokenInStripe = sizeInfo->blksPerStripe;
        totalToken = DEFAULT_TOTAL_TOKEN_IN_STRIPE * sizeInfo->blksPerStripe;
        flowControlStrategy = DEFAULT_FLOW_CONTROL_STRATEGY;
        return;
    }

    uint64_t timeoutInMsec = _ReadTimeoutInMsec();
    forceResetTimeout = timeoutInMsec * NANOS_PER_MSEC;

    totalTokenInStripe = _ReadTotalTokenInStripe();
    totalToken = totalTokenInStripe * sizeInfo->blksPerStripe;

    flowControlStrategy = _ReadFlowControlStrategy();
    if (FlowControlStrategy::LINEAR == flowControlStrategy)
    {
        return;
    }

    bool ret;
    std::tie(ret, targetPercent, urgentPercent, targetSegment, urgentSegment) = _ReadStateConfig();
    if (false == ret)
    {
        flowControlStrategy = FlowControlStrategy::LINEAR;
    }
    return;
}

bool
FlowControlConfiguration::GetEnable(void)
{
    return enable;
}

uint64_t
FlowControlConfiguration::GetForceResetTimeout(void)
{
    return forceResetTimeout;
}

uint32_t
FlowControlConfiguration::GetTotalToken(void)
{
    return totalToken;
}

uint32_t
FlowControlConfiguration::GetTotalTokenInStripe(void)
{
    return totalTokenInStripe;
}

FlowControlStrategy
FlowControlConfiguration::GetFlowControlStrategy(void)
{
    return flowControlStrategy;
}

uint32_t
FlowControlConfiguration::GetTargetPercent(void)
{
    return targetPercent;
}

uint32_t
FlowControlConfiguration::GetUrgentPercent(void)
{
    return urgentPercent;
}

uint32_t
FlowControlConfiguration::GetTargetSegment(void)
{
    return targetSegment;
}

uint32_t
FlowControlConfiguration::GetUrgentSegment(void)
{
    return urgentSegment;
}

bool
FlowControlConfiguration::_IsEnabled(void)
{
    bool enabled = false;
    int ret = configManager->GetValue("flow_control", "enable",
                &enabled, ConfigType::CONFIG_TYPE_BOOL);
    if (0 != ret)
    {
        POS_TRACE_INFO(EID(FLOW_CONTROL_CONFIGURATION),
            "Failed to read FlowControl enablement from config file, default_enable: {}",
            enabled);
    }
    return enabled;
}

bool
FlowControlConfiguration::_IsDefaultSetting(void)
{
    bool use_default = true;
    int ret = configManager->GetValue("flow_control", "use_default",
                &use_default, ConfigType::CONFIG_TYPE_BOOL);
    if (0 != ret)
    {
        POS_TRACE_INFO(EID(FLOW_CONTROL_CONFIGURATION),
            "Failed to read FlowControl use default setting from config file, default_use_default: {}",
            use_default);
    }
    return use_default;
}

uint64_t
FlowControlConfiguration::_ReadTimeoutInMsec(void)
{
    uint64_t timeoutInMsec = 1000;
    int ret = configManager->GetValue("flow_control", "refill_timeout_in_msec",
            &timeoutInMsec, ConfigType::CONFIG_TYPE_UINT64);
    if (0 != ret)
    {
        POS_TRACE_INFO(EID(FLOW_CONTROL_CONFIGURATION),
            "Failed to read FlowControl timeout in msec from config file, default_refill_timeout_in_msed: {}",
            timeoutInMsec);
    }
    return timeoutInMsec;
}

uint32_t
FlowControlConfiguration::_ReadTotalTokenInStripe(void)
{
    uint32_t total_token_in_stripe = 1024;
    int ret = configManager->GetValue("flow_control", "total_token_in_stripe",
            &total_token_in_stripe, ConfigType::CONFIG_TYPE_UINT32);
    if (0 != ret)
    {
        POS_TRACE_INFO(EID(FLOW_CONTROL_CONFIGURATION),
            "Failed to read FlowControl total token in stripe from config file, defualt_total_token_in_stripe: {}",
            total_token_in_stripe);
    }
    return total_token_in_stripe;
}

FlowControlStrategy
FlowControlConfiguration::_ReadFlowControlStrategy(void)
{
    std::string strategy = "linear";
    FlowControlStrategy flowControlStrategy = FlowControlStrategy::LINEAR;
    int ret = configManager->GetValue("flow_control", "strategy",
            &strategy, ConfigType::CONFIG_TYPE_STRING);
    if (0 == ret)
    {
        for (int i = 0; i < (int)FlowControlStrategy::MAX_FLOW_CONTROL_STRATEGY; i++)
        {
            if (FLOW_CONTROL_STRATEGY_NAME[i] == strategy)
            {
                flowControlStrategy = (FlowControlStrategy)i;
            }
        }
    }
    else
    {
        flowControlStrategy = FlowControlStrategy::LINEAR;
        POS_TRACE_INFO(EID(FLOW_CONTROL_CONFIGURATION),
            "Failed to read FlowControl strategy from config file, defualt_strategy: linear");
    }
    return flowControlStrategy;
}

std::tuple<bool, uint32_t, uint32_t, uint32_t, uint32_t>
FlowControlConfiguration::_ReadStateConfig(void)
{
    uint32_t flow_control_target_percent = 30;
    uint32_t flow_control_urgent_percent = 10;
    uint32_t flow_control_target_segment = 10;
    uint32_t flow_control_urgent_segment = 5;

    int ret;
    bool success = true;
    ret = configManager->GetValue("flow_control", "flow_control_target_percent",
            &flow_control_target_percent, ConfigType::CONFIG_TYPE_UINT32);
    if (0 != ret)
    {
        success = false;
    }
    ret = configManager->GetValue("flow_control", "flow_control_urgent_percent",
            &flow_control_urgent_percent, ConfigType::CONFIG_TYPE_UINT32);
    if (0 != ret)
    {
        success = false;
    }
    ret = configManager->GetValue("flow_control", "flow_control_target_segment",
            &flow_control_target_segment, ConfigType::CONFIG_TYPE_UINT32);
    if (0 != ret)
    {
        success = false;
    }
    ret = configManager->GetValue("flow_control", "flow_control_urgent_segment",
            &flow_control_urgent_segment, ConfigType::CONFIG_TYPE_UINT32);
    if (0 != ret)
    {
        success = false;
    }

    if (flow_control_target_percent < flow_control_urgent_percent || flow_control_target_segment < flow_control_urgent_segment)
    {
        success = false;
    }

    if (false == success)
    {
        POS_TRACE_INFO(EID(FLOW_CONTROL_CONFIGURATION),
            "Failed to read FlowControl state strategy from config file, default_strategy: linear");
        return std::make_tuple(false, 0, 0, 0, 0);
    }
    return std::make_tuple(true, flow_control_target_percent,
                        flow_control_urgent_percent,
                        flow_control_target_segment, flow_control_urgent_segment);
}
} // namespace pos
