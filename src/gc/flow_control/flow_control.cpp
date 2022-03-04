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

#include "src/include/pos_event_id.h"
#include "src/lib/system_timeout_checker.h"
#include "src/gc/flow_control/flow_control.h"
#include "src/gc/flow_control/flow_control_service.h"
#include "src/gc/flow_control/flow_control_configuration.h"
#include "src/gc/flow_control/token_distributer.h"
#include "src/gc/flow_control/linear_distributer.h"
#include "src/gc/flow_control/state_distributer.h"
#include "src/allocator/i_context_manager.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/logger/logger.h"
#include "src/array_models/dto/partition_logical_size.h"
#include "assert.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

namespace pos
{

FlowControl::FlowControl(IArrayInfo* arrayInfo)
: FlowControl(arrayInfo,
            nullptr,
            new SystemTimeoutChecker(),
            FlowControlServiceSingleton::Instance(),
            nullptr,
            new FlowControlConfiguration(arrayInfo))
{
}

FlowControl::FlowControl(IArrayInfo* arrayInfo,
                        IContextManager* inputIContextManager,
                        SystemTimeoutChecker* inputSystemTimeoutChecker,
                        FlowControlService* inputFlowControlService,
                        TokenDistributer* inputTokenDistributer,
                        FlowControlConfiguration* inputFlowControlConfiguration)
: arrayInfo(arrayInfo),
  iContextManager(inputIContextManager),
  systemTimeoutChecker(inputSystemTimeoutChecker),
  flowControlService(inputFlowControlService),
  tokenDistributer(inputTokenDistributer),
  flowControlConfiguration(inputFlowControlConfiguration)
{
}

FlowControl::~FlowControl(void)
{
    if (nullptr != tokenDistributer)
    {
        delete tokenDistributer;
    }
    if (nullptr != systemTimeoutChecker)
    {
        delete systemTimeoutChecker;
    }
    if (nullptr != flowControlConfiguration)
    {
        delete flowControlConfiguration;
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
    gcThreshold = iContextManager->GetGcThreshold(GcMode::MODE_NORMAL_GC);
    gcUrgentThreshold = iContextManager->GetGcThreshold(GcMode::MODE_URGENT_GC);

    sizeInfo = arrayInfo->GetSizeInfo(PartitionType::USER_DATA);
    blksPerStripe = sizeInfo->blksPerStripe;

    flowControlConfiguration->ReadConfig();
    totalToken = flowControlConfiguration->GetTotalToken();
    totalTokenInStripe = flowControlConfiguration->GetTotalTokenInStripe();
    targetSegment = flowControlConfiguration->GetTargetSegment();
    targetPercent = flowControlConfiguration->GetTargetPercent();
    urgentSegment = flowControlConfiguration->GetUrgentSegment();
    urgentPercent = flowControlConfiguration->GetUrgentPercent();
    flowControlStrategy = flowControlConfiguration->GetFlowControlStrategy();

    if (nullptr == tokenDistributer)
    {
        if (FlowControlStrategy::LINEAR == flowControlStrategy)
        {
            tokenDistributer = new LinearDistributer(arrayInfo, flowControlConfiguration);
        }
        else if (FlowControlStrategy::STATE == flowControlStrategy)
        {
            tokenDistributer = new StateDistributer(arrayInfo, flowControlConfiguration);
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

void
FlowControl::Flush(void)
{
    // no-op
}

int
FlowControl::GetToken(FlowControlType type, int token)
{
    if (token < 0)
    {
        POS_TRACE_DEBUG(EID(FC_NEGATIVE_TOKEN), "FlowControl GetToken should be greater than or equal to zero token:{}", token);
        return 0;
    }

    FlowControlStrategy strategy = flowControlConfiguration->GetFlowControlStrategy();
    if (FlowControlStrategy::DISABLE == strategy)
    {
        return token;
    }

    SegmentCtx* segmentCtx = iContextManager->GetSegmentCtx();
    uint32_t freeSegments = segmentCtx->GetNumOfFreeSegmentWoLock();
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
                if (0 >= bucket[type].load())
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
        return;
    }

    if (FlowControlStrategy::DISABLE == flowControlConfiguration->GetFlowControlStrategy())
    {
        return;
    }

    bucket[type].fetch_add(token);
}

void
FlowControl::InitDistributer(void)
{
    tokenDistributer->Init();
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
            POS_TRACE_INFO(EID(FC_RESET_FORCERESET), "_reset isForceReset, userBucket:{}, gcBucket:{}, userPrevBucket:{}, gcPrevBucket:{}",
                            bucket[FlowControlType::USER], bucket[FlowControlType::GC],
                            previousBucket[FlowControlType::USER], previousBucket[FlowControlType::GC]);
            isForceReset = false;
        }
        return false;
    }

    if (false == isForceReset)
    {
        POS_TRACE_INFO(EID(FC_SET_FORCERESET), "_set isForceReset, userBucket:{}, gcBucket:{}",
                        bucket[FlowControlType::USER], bucket[FlowControlType::GC]);
        systemTimeoutChecker->SetTimeout(flowControlConfiguration->GetForceResetTimeout());
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
    POS_TRACE_INFO(EID(FC_FORCERESET_DONE), "_force Reset, userPrevBucket:{}, gcPrevBucket:{}",
                        previousBucket[FlowControlType::USER], previousBucket[FlowControlType::GC]);
    return true;
}

std::tuple<uint32_t, uint32_t>
FlowControl::_DistributeToken(void)
{
    SegmentCtx* segmentCtx = iContextManager->GetSegmentCtx();
    return tokenDistributer->Distribute(segmentCtx->GetNumOfFreeSegmentWoLock());
}
} // namespace pos
