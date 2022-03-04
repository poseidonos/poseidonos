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

#include "get_gc_status_wbt_command.h"

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

#include "src/allocator_service/allocator_service.h"
#include "src/allocator/i_context_manager.h"
#include "src/array_mgmt/array_manager.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/gc/garbage_collector.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

namespace pos
{
GetGcStatusWbtCommand::GetGcStatusWbtCommand(void)
:   GcWbtCommand(GET_GC_STATUS, "get_gc_status")
{
}
// LCOV_EXCL_START
GetGcStatusWbtCommand::~GetGcStatusWbtCommand(void)
{
}
// LCOV_EXCL_STOP
int
GetGcStatusWbtCommand::Execute(Args &argv, JsonElement &elem)
{
    if (!argv.contains("array"))
    {
        return -1;
    }
    std::string arrayName = argv["array"].get<std::string>();
    GarbageCollector* gc = _GetGC(arrayName);

    if (gc == nullptr)
    {
        return -1;
    }
    int isEnabled = gc->IsEnabled();
    if (0 != isEnabled)
    {
        return -1;
    }

    bool gcRunning = gc->GetGcRunning();
    IContextManager* iContextManager = AllocatorServiceSingleton::Instance()->GetIContextManager(arrayName);
    SegmentCtx* segmentCtx = iContextManager->GetSegmentCtx();
    uint32_t freeSegments = segmentCtx->GetNumOfFreeSegment();
    uint32_t numGcThreshold = iContextManager->GetGcThreshold(MODE_NORMAL_GC);
    uint32_t numUrgentThreshold = iContextManager->GetGcThreshold(MODE_URGENT_GC);

    std::string gcActive;
    std::string gcMode;

    gcActive = (false == gcRunning) ? "done" : "active";

    if (freeSegments <= numUrgentThreshold)
    {
        gcMode = "urgent";
    }
    else if (freeSegments < numGcThreshold)
    {
        gcMode = "normal";
    }
    else
    {
        gcMode = "none";
    }

    JsonElement gcElem("gc");

    JsonElement statusElem("status");
    statusElem.SetAttribute(JsonAttribute("active", "\"" + gcActive + "\""));
    statusElem.SetAttribute(JsonAttribute("mode", "\"" + gcMode + "\""));
    gcElem.SetElement(statusElem);

    JsonElement timeElem("time");

    struct timeval startTv =
        gc->GetStartTime();

    std::ostringstream oss;

    if (startTv.tv_sec == 0 && startTv.tv_usec == 0)
    {
        oss << "N/A";
    }
    else
    {
        std::tm tm;
        localtime_r(&startTv.tv_sec, &tm);

        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    }

    timeElem.SetAttribute(JsonAttribute("start", "\"" + oss.str() + "\""));

    oss.str("");
    oss.clear();
    uint32_t elapsed = 0;

    if (false == gcRunning)
    {
        struct timeval endTv =
            gc->GetEndTime();
        if (endTv.tv_sec == 0 && endTv.tv_usec == 0)
        {
            oss << "N/A";
        }
        else
        {
            std::tm tm;
            localtime_r(&endTv.tv_sec, &tm);
            oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        }

        elapsed = ((endTv.tv_sec - startTv.tv_sec) * 1000) + ((endTv.tv_usec - startTv.tv_usec) / 1000);
    }

    timeElem.SetAttribute(JsonAttribute("end", "\"" + oss.str() + "\""));
    timeElem.SetAttribute(JsonAttribute("elapsed", elapsed));

    gcElem.SetElement(timeElem);

    const PartitionLogicalSize* udSize;
    IArrayInfo* info = ArrayMgr()->GetInfo(arrayName)->arrayInfo;
    udSize = info->GetSizeInfo(PartitionType::USER_DATA);
    uint32_t totalSegments = udSize->totalSegments;

    JsonElement segElem("segment");

    segElem.SetAttribute(JsonAttribute("total", totalSegments));
    segElem.SetAttribute(JsonAttribute("used", totalSegments - freeSegments));
    segElem.SetAttribute(JsonAttribute("free", freeSegments));
    gcElem.SetElement(segElem);

    elem.SetElement(gcElem);

    return 0;
}

} // namespace pos
