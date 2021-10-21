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

#include "src/gc/flow_control/token_distributer.h"
#include "src/allocator/i_context_manager.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/array_models/dto/partition_logical_size.h"
#include "src/allocator_service/allocator_service.h"
#include "src/gc/flow_control/flow_control_configuration.h"

namespace pos
{
TokenDistributer::TokenDistributer(IArrayInfo* iArrayInfo, FlowControlConfiguration* flowControlConfiguration,
                                IContextManager* inputIContextManager)
: iArrayInfo(iArrayInfo),
  flowControlConfiguration(flowControlConfiguration),
  iContextManager(inputIContextManager)
{
    Init();
}

TokenDistributer::~TokenDistributer(void)
{
}

void
TokenDistributer::Init(void)
{
}

std::tuple<uint32_t, uint32_t>
TokenDistributer::Distribute(uint32_t freeSegments)
{
    uint32_t totalToken = flowControlConfiguration->GetTotalToken();
    uint32_t gcThreshold = iContextManager->GetGcThreshold(GcMode::MODE_NORMAL_GC);

    if (freeSegments > gcThreshold)
    {
        return std::make_tuple(totalToken, 0);
    }
    else
    {
        return std::make_tuple(totalToken / 2, totalToken / 2);
    }
}

}; // namespace pos
