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

#include "set_gc_threshold_wbt_command.h"

#include <exception>
#include <string>

#include "src/cli/command.h"
#include "src/array_mgmt/array_manager.h"
#include "src/allocator_service/allocator_service.h"
#include "src/allocator/i_allocator_wbt.h"
#include "src/gc/garbage_collector.h"
#include "src/logger/logger.h"

namespace pos
{
SetGcThresholdWbtCommand::SetGcThresholdWbtCommand(void)
:   GcWbtCommand(SET_GC_THRESHOLD, "set_gc_threshold")
{
}
// LCOV_EXCL_START
SetGcThresholdWbtCommand::~SetGcThresholdWbtCommand(void)
{
}
// LCOV_EXCL_STOP
int
SetGcThresholdWbtCommand::Execute(Args &argv, JsonElement &elem)
{
    int returnValue = -1;

    if (!argv.contains("array"))
    {
        return returnValue;
    }
    std::string arrayName = argv["array"].get<std::string>();
    GarbageCollector* gc = _GetGC(arrayName);

    if (gc == nullptr)
    {
        return returnValue;
    }

    int isEnabled = gc->IsEnabled();
    if (0 != isEnabled)
    {
        return returnValue;
    }

    if (!argv.contains("normal") || !argv.contains("urgent"))
    {
        return returnValue;
    }

    std::string gcThreshold = argv["normal"].get<std::string>();
    std::string urgentThreshold = argv["urgent"].get<std::string>();

    if (gcThreshold.at(0) == '-' || urgentThreshold.at(0) == '-')
    {
        return returnValue;
    }

    try
    {
        uint32_t numGcThreshold = std::stoul(gcThreshold);
        uint32_t numUrgentThreshold = std::stoul(urgentThreshold);

        if (numGcThreshold <= numUrgentThreshold)
        {
            return returnValue;
        }

        IAllocatorWbt* iAllocatorWbt = AllocatorServiceSingleton::Instance()->GetIAllocatorWbt(arrayName);
        iAllocatorWbt->SetNormalGcThreshold(numGcThreshold);
        iAllocatorWbt->SetUrgentThreshold(numUrgentThreshold);

        IContextManager* iContextManager = AllocatorServiceSingleton::Instance()->GetIContextManager(arrayName);
        SegmentCtx* segmentCtx = iContextManager->GetSegmentCtx();
        GcCtx* gcCtx = iContextManager->GetGcCtx();

        ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
        if (info == nullptr)
        {
            POS_TRACE_ERROR(EID(ARRAY_MGR_NO_ARRAY_MATCHING_REQ_NAME), "failed to get array info.!");
            return returnValue;
        }

        uint32_t arrayId = iContextManager->GetArrayId();
        segmentCtx->UpdateGcFreeSegment(arrayId);
        int numOfFreeSegments = segmentCtx->GetNumOfFreeSegment();

        if (MODE_URGENT_GC != gcCtx->GetCurrentGcMode(numOfFreeSegments))
        {
            IBlockAllocator* iBlockAllocator =
                AllocatorServiceSingleton::Instance()->GetIBlockAllocator(arrayName);
            iBlockAllocator->PermitUserBlkAlloc();
        }

        returnValue = 0;
    }
    catch (const std::exception& e)
    {
        POS_TRACE_ERROR(returnValue, e.what());
    }

    return returnValue;
}

} // namespace pos
