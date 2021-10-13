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

#include "src/gc/flow_control/state_distributer.h"
#include "src/allocator/i_context_manager.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/array_models/dto/partition_logical_size.h"
#include "src/allocator_service/allocator_service.h"
#include "src/gc/flow_control/flow_control_configuration.h"

namespace pos
{
StateDistributer::StateDistributer(IArrayInfo* iArrayInfo, FlowControlConfiguration* flowControlConfiguration)
: StateDistributer(iArrayInfo, flowControlConfiguration,
                AllocatorServiceSingleton::Instance()->GetIContextManager(iArrayInfo->GetName()))
{
}

StateDistributer::StateDistributer(IArrayInfo* iArrayInfo, FlowControlConfiguration* flowControlConfiguration,
                                IContextManager* inputIContextManager)
: TokenDistributer(iArrayInfo, flowControlConfiguration, inputIContextManager),
  blksPerStripe(0),
  totalTokenInStripe(0),
  totalToken(0),
  gcThreshold(0),
  targetSegment(0),
  targetPercent(0),
  urgentSegment(0),
  urgentPercent(0)
{
    Init();
}

StateDistributer::~StateDistributer(void)
{
}

void
StateDistributer::Init(void)
{
    const PartitionLogicalSize* sizeInfo = iArrayInfo->GetSizeInfo(PartitionType::USER_DATA);
    blksPerStripe = sizeInfo->blksPerStripe;

    totalTokenInStripe = flowControlConfiguration->GetTotalTokenInStripe();
    totalToken = flowControlConfiguration->GetTotalToken();

    gcThreshold = iContextManager->GetGcThreshold(GcMode::MODE_NORMAL_GC);
    gcUrgentThreshold = iContextManager->GetGcThreshold(GcMode::MODE_URGENT_GC);

    targetSegment = flowControlConfiguration->GetTargetSegment();
    targetPercent = flowControlConfiguration->GetTargetPercent();
    urgentSegment = flowControlConfiguration->GetUrgentSegment();
    urgentPercent = flowControlConfiguration->GetUrgentPercent();
}

std::tuple<uint32_t, uint32_t>
StateDistributer::Distribute(uint32_t freeSegments)
{
    uint32_t userStripe;
    uint32_t userToken;
    uint32_t gcToken;

    if (freeSegments > gcThreshold)
    {
        userToken = totalToken;
    }
    else if (freeSegments > targetSegment)
    {
        userStripe = (totalTokenInStripe * targetPercent) / 100;
        userToken =  userStripe * blksPerStripe;
    }
    else if (freeSegments > urgentSegment)
    {
        userStripe = (totalTokenInStripe * urgentPercent) / 100;
        userToken =  userStripe * blksPerStripe;
    }
    else
    {
        userToken = 0;
    }

    gcToken = totalToken - userToken;

    return std::make_tuple(userToken, gcToken);
}

}; // namespace pos
