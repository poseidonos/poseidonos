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

#include "src/master_context/config_manager.h"
#include "src/include/pos_event_id.h"
#include "src/lib/system_timeout_checker.h"
#include "src/gc/flow_control/flow_control.h"
#include "src/gc/flow_control/flow_control_service.h"
#include "src/gc/flow_control/token_distributer.h"
#include "src/gc/flow_control/linear_distributer.h"
#include "src/gc/flow_control/state_distributer.h"
#include "src/allocator/i_context_manager.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/logger/logger.h"
#include "src/array_models/dto/partition_logical_size.h"
#include "assert.h"

namespace pos
{

FlowControl::FlowControl(IArrayInfo* arrayInfo)
: FlowControl(arrayInfo, AllocatorServiceSingleton::Instance()->GetIContextManager(arrayInfo->GetName()), nullptr,
              new SystemTimeoutChecker(), FlowControlServiceSingleton::Instance(), nullptr)
{
}

FlowControl::FlowControl(IArrayInfo* arrayInfo, IContextManager* inputIContextManager, const PartitionLogicalSize* inputSizeInfo,
                         SystemTimeoutChecker* inputSystemTimeoutChecker, FlowControlService* inputFlowControlService, TokenDistributer* inputTokenDistributer)
: arrayInfo(arrayInfo),
  iContextManager(inputIContextManager),
  sizeInfo(inputSizeInfo),
  systemTimeoutChecker(inputSystemTimeoutChecker),
  flowControlService(inputFlowControlService),
  tokenDistributer(inputTokenDistributer)
{
}

FlowControl::~FlowControl(void)
{
    if (nullptr != tokenDistributer)
    {
        delete tokenDistributer;
    }
}

int
FlowControl::Init(void)
{
    int result = 0;

    if (nullptr == iContextManager)
    {
        iContextManager = AllocatorServiceSingleton::Instance()->GetIContextManager(arrayInfo->GetName());
    }

    sizeInfo = arrayInfo->GetSizeInfo(PartitionType::USER_DATA);

    blksPerStripe = sizeInfo->blksPerStripe;
    stripesPerSegment = sizeInfo->stripesPerSegment;
    totalSegments = sizeInfo->totalSegments;
    totalTokenInStripe = stripesPerSegment;
    totalToken = totalTokenInStripe * blksPerStripe;
    gcThreshold = iContextManager->GetGcThreshold(CurrentGcMode::MODE_NORMAL_GC);
    gcUrgentThreshold = iContextManager->GetGcThreshold(CurrentGcMode::MODE_URGENT_GC);

    _ReadConfig();

    if (nullptr == tokenDistributer)
    {
        if (FlowControlStrategy::LINEAR == flowControlStrategy)
        {
            tokenDistributer = new LinearDistributer(totalToken, gcThreshold, gcUrgentThreshold, totalTokenInStripe, blksPerStripe);
        }
        else if (FlowControlStrategy::STATE == flowControlStrategy)
        {
            tokenDistributer = new StateDistributer(totalToken, gcThreshold, targetSegment, targetPercent, urgentSegment, urgentPercent, totalTokenInStripe, blksPerStripe);
        }
    }

    for (uint32_t i = 0; i < FlowControlType::MAX_FLOW_CONTROL_TYPE; i++)
    {
        bucket[i] = 0;
        previousBucket[i] = 0;
    }

    flowControlService->Register(arrayInfo->GetName(), this);

    return result;
}

void
FlowControl::Dispose(void)
{
    flowControlService->UnRegister(arrayInfo->GetName());
}

void
FlowControl::Shutdown(void)
{
    flowControlService->UnRegister(arrayInfo->GetName());
}

int
FlowControl::GetToken(FlowControlType type, int token)
{
    if (token < 0)
    {
        POS_TRACE_DEBUG(EID(FC_NEGATIVE_TOKEN), "FlowControl GetToken should be greater than or equal to zero token:{}", token);
        assert(false);
    }

    if (FlowControlStrategy::DISABLE == flowControlStrategy)
    {
        return token;
    }

    uint32_t freeSegments = iContextManager->GetNumFreeSegment();
    if (freeSegments > gcThreshold)
    {
        return token;
    }

    int oldBucket = bucket[type].load();
    do
    {
        if (oldBucket <= 0)
        {
            if (refillTokenMutex.try_lock())
            {
                bool ret = _RefillToken(type);
                refillTokenMutex.unlock();
                if (false == ret)
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
    } while (!bucket[type].compare_exchange_weak(oldBucket, oldBucket - token));

    return token;
}

void
FlowControl::ReturnToken(FlowControlType type, int token)
{
    if (token < 0)
    {
        POS_TRACE_DEBUG(EID(FC_NEGATIVE_TOKEN), "FlowControl GetToken should be greater than or equal to zero token:{}", token);
        assert(false);
    }

    if (FlowControlStrategy::DISABLE == flowControlStrategy)
    {
        return;
    }

    bucket[type].fetch_add(token);
}

bool
FlowControl::_RefillToken(FlowControlType type)
{
    int counterType = type ^ 0x1;
    if (bucket[counterType] > 0)
    {
        bool ret = _TryForceResetToken(type);
        if (false == ret)
        {
            return false;
        }
    }

    uint32_t userToken;
    uint32_t gcToken;

    std::tie(userToken, gcToken) = _DistributeToken();
    POS_TRACE_INFO(EID(FC_TOKEN_DISTRIBUTED), "_DistributeToken userToken:{}, gcToken:{}, userBucket:{}, gcBucket:{}",
                                        userToken, gcToken, bucket[FlowControlType::USER], bucket[FlowControlType::GC]);
    bucket[FlowControlType::USER].fetch_add(userToken);
    bucket[FlowControlType::GC].fetch_add(gcToken);

    return true;
}

bool
FlowControl::_TryForceResetToken(FlowControlType type)
{
    int counterType = type ^ 0x1;
    if (previousBucket[counterType] != bucket[counterType])
    {
        previousBucket[counterType] = bucket[counterType].load();
        if (true == isForceReset)
        {
            isForceReset = false;
        }
        return false;
    }

    if (false == isForceReset)
    {
        systemTimeoutChecker->SetTimeout(forceResetTimeout);
        isForceReset = true;
        return false;
    }

    if (false == systemTimeoutChecker->CheckTimeout())
    {
        return false;
    }

    bucket[FlowControlType::USER] = 0;
    bucket[FlowControlType::GC] = 0;

    isForceReset = false;

    return true;
}

std::tuple<uint32_t, uint32_t>
FlowControl::_DistributeToken(void)
{
    return tokenDistributer->Distribute(iContextManager->GetNumFreeSegment());
}

void
FlowControl::_ReadConfig(void)
{
    int SUCCESS = (int)POS_EVENT_ID::SUCCESS;

    bool enable = true;
    int ret = ConfigManagerSingleton::Instance()->GetValue("flow_control", "enable",
                &enable, ConfigType::CONFIG_TYPE_BOOL);
    if (SUCCESS == ret)
    {
        if (false == enable)
        {
            flowControlStrategy = FlowControlStrategy::DISABLE;
            return;
        }
    }

    bool use_default = true;
    ret = ConfigManagerSingleton::Instance()->GetValue("flow_control", "use_default",
                &use_default, ConfigType::CONFIG_TYPE_BOOL);
    if (SUCCESS == ret)
    {
        if (true == use_default)
        {
            return;
        }
    }

    uint64_t refill_timeout_in_msec = 1000;
    ret = ConfigManagerSingleton::Instance()->GetValue("flow_control", "refill_timeout_in_msec",
            &refill_timeout_in_msec, ConfigType::CONFIG_TYPE_UINT64);
    if (SUCCESS == ret)
    {
        forceResetTimeout = refill_timeout_in_msec * NANOS_PER_MSEC;
    }


    uint32_t total_token_in_stripe = 1024;
    ret = ConfigManagerSingleton::Instance()->GetValue("flow_control", "total_token_in_stripe",
            &total_token_in_stripe, ConfigType::CONFIG_TYPE_UINT32);
    if (SUCCESS == ret)
    {
        totalTokenInStripe = total_token_in_stripe;
        totalToken = totalTokenInStripe * blksPerStripe;
    }

    std::string strategy = "linear";
    ret = ConfigManagerSingleton::Instance()->GetValue("flow_control", "strategy",
            &strategy, ConfigType::CONFIG_TYPE_STRING);
    if (SUCCESS == ret)
    {
        for (int i = 0; i < (int)FlowControlStrategy::MAX_FLOW_CONTROL_STRATEGY; i++)
        {
            if (FLOW_CONTROL_STRATEGY_NAME[i] == strategy)
            {
                flowControlStrategy = (FlowControlStrategy)i;
            }
        }
    }

    uint32_t flow_control_target_percent = 30;
    uint32_t flow_control_urgent_percent = 10;
    uint32_t flow_control_target_segment = 10;
    uint32_t flow_control_urgent_segment = 5;
    if (FlowControlStrategy::STATE == flowControlStrategy)
    {
        ret = ConfigManagerSingleton::Instance()->GetValue("flow_control", "flow_control_target_percent",
                &flow_control_target_percent, ConfigType::CONFIG_TYPE_UINT32);
        if (SUCCESS != ret)
        {
            return;
        }
        ret = ConfigManagerSingleton::Instance()->GetValue("flow_control", "flow_control_urgent_percent",
                &flow_control_urgent_percent, ConfigType::CONFIG_TYPE_UINT32);
        if (SUCCESS != ret)
        {
            return;
        }
        ret = ConfigManagerSingleton::Instance()->GetValue("flow_control", "flow_control_target_segment",
                &flow_control_target_segment, ConfigType::CONFIG_TYPE_UINT32);
        if (SUCCESS != ret)
        {
            return;
        }
        ret = ConfigManagerSingleton::Instance()->GetValue("flow_control", "flow_control_urgent_segment",
                &flow_control_urgent_segment, ConfigType::CONFIG_TYPE_UINT32);
        if (SUCCESS != ret)
        {
            return;
        }

        if (flow_control_target_percent < flow_control_urgent_percent || flow_control_target_segment < flow_control_urgent_segment)
        {
            return;
        }
        else
        {
            targetPercent = flow_control_target_percent;
            urgentPercent = flow_control_urgent_percent;
            targetSegment = flow_control_target_segment;
            urgentSegment = flow_control_urgent_segment;
        }
    }
}

} // namespace pos
