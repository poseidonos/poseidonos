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

#include "src/gc/flow_control/flow_control.h"

#include "assert.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"
#include "src/allocator/i_context_manager.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_models/dto/partition_logical_size.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/gc/flow_control/flow_control_configuration.h"
#include "src/gc/flow_control/flow_control_service.h"
#include "src/gc/flow_control/linear_distributer.h"
#include "src/gc/flow_control/state_distributer.h"
#include "src/gc/flow_control/token_distributer.h"
#include "src/include/pos_event_id.h"
#include "src/lib/system_timeout_checker.h"
#include "src/logger/logger.h"

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
    if (arrayInfo != nullptr)
    {
        RegisterDebugInfo("GC_FlowControl_Array" + std::to_string(arrayInfo->GetIndex()), 10000, true);
    }
    else
    {
        RegisterDebugInfo("GC_FlowControl", 10000, true);
    }
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

void
FlowControl::MakeDebugInfo(FlowControlDebugInfo& obj)
{
    obj.bucket[FlowControlType::USER] = bucket[FlowControlType::USER].load();
    obj.bucket[FlowControlType::GC] = bucket[FlowControlType::GC].load();

    obj.totalToken = totalToken;
    obj.totalTokenInStripe = totalTokenInStripe;
    obj.totalSegments = totalSegments;
    obj.gcThreshold = gcThreshold;
    obj.gcUrgentThreshold = gcUrgentThreshold;
    obj.freeSegments = freeSegments;
    obj.targetPercent = targetPercent;
    obj.urgentPercent = urgentPercent;
    obj.targetSegment = targetSegment;
    obj.urgentSegment = urgentSegment;
    obj.forceResetTimeout = forceResetTimeout;
    if (arrayInfo != nullptr)
    {
        obj.arrayId = arrayInfo->GetIndex();
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

    InitDistributer();

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
        POS_TRACE_DEBUG(EID(FLOW_CONTROL_REQUESTED_TOKEN_NEGATIVE), "FlowControl GetToken should be greater than or equal to zero token:{}", token);
        return 0;
    }

    FlowControlStrategy strategy = flowControlConfiguration->GetFlowControlStrategy();
    if (FlowControlStrategy::DISABLE == strategy)
    {
        return token;
    }

    SegmentCtx* segmentCtx = iContextManager->GetSegmentCtx();
    freeSegments = segmentCtx->GetNumOfFreeSegmentWoLock();
    if (freeSegments > gcThreshold)
    {
        if(true == systemTimeoutChecker->IsActive())
        {
            systemTimeoutChecker->Reset();
        }
        return token;
    }
    int oldBucket = bucket[type].load();
    int counterType = type ^ 0x1;
    if (false == systemTimeoutChecker->IsActive())
    {
        systemTimeoutChecker->SetTimeout(flowControlConfiguration->GetForceResetTimeout());
    }
    do
    {
        if (oldBucket <= 0)
        {
            if (refillTokenMutex.try_lock())
            {
                if (bucket[counterType] <= 0 || systemTimeoutChecker->CheckTimeout() == true)
                {
                    _ResetAndRefillToken();
                    systemTimeoutChecker->SetTimeout(flowControlConfiguration->GetForceResetTimeout());
                }
                refillTokenMutex.unlock();

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

        if (true == systemTimeoutChecker->CheckTimeout())
        {
            if (refillTokenMutex.try_lock())
            {
                _ResetAndRefillToken();
                refillTokenMutex.unlock();
                systemTimeoutChecker->SetTimeout(flowControlConfiguration->GetForceResetTimeout());
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
        POS_TRACE_DEBUG(EID(FLOW_CONTROL_REQUESTED_TOKEN_NEGATIVE), "FlowControl ReturnToken should be greater than or equal to zero token:{}", token);
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
    POS_TRACE_DEBUG(EID(FLOW_CONTROL_TOKEN_DISTRIBUTED_INIT), "IN InitDistributer");

    if (nullptr != tokenDistributer)
    {
        tokenDistributer->Init();
        POS_TRACE_DEBUG(EID(FLOW_CONTROL_TOKEN_DISTRIBUTED_TRY_INIT), "try to distributer init");
    }
    else
    {
        POS_TRACE_DEBUG(EID(FLOW_CONTROL_TOKEN_DISTRIBUTED_SKIP_INIT), "skip to distributer init");
    }
}

void
FlowControl::Reset(void)
{
    bucket[FlowControlType::USER] = 0;
    bucket[FlowControlType::GC] = 0;
}

void
FlowControl::_ResetAndRefillToken()
{
    uint32_t userToken;
    uint32_t gcToken;

    Reset();
    std::tie(userToken, gcToken) = _DistributeToken();
    AddDebugInfo();
    bucket[FlowControlType::USER].fetch_add(userToken);
    bucket[FlowControlType::GC].fetch_add(gcToken);
}

std::tuple<uint32_t, uint32_t>
FlowControl::_DistributeToken(void)
{
    SegmentCtx* segmentCtx = iContextManager->GetSegmentCtx();
    return tokenDistributer->Distribute(segmentCtx->GetNumOfFreeSegmentWoLock());
}
} // namespace pos
