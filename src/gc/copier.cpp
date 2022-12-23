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

#include "src/gc/copier.h"

#include <air/Air.h>

#include <list>
#include <memory>

#include "src/allocator/context_manager/gc_ctx/gc_ctx.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"
#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_context_manager.h"
#include "src/allocator_service/allocator_service.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/gc/copier_read_completion.h"
#include "src/gc/reverse_map_load_completion.h"
#include "src/gc/stripe_copy_submission.h"
#include "src/include/backend_event.h"
#include "src/io/general_io/io_submit_handler.h"
#include "src/logger/logger.h"
#include "src/metadata/segment_context_updater.h"

#include <iostream>

namespace pos
{

Copier::Copier(SegmentId victimId, SegmentId targetId, GcStatus* gcStatus, IArrayInfo* array)
: Copier(victimId, targetId, gcStatus, array,
      array->GetSizeInfo(PartitionType::USER_DATA), new CopierMeta(array),
      AllocatorServiceSingleton::Instance()->GetIBlockAllocator(array->GetName()),
      AllocatorServiceSingleton::Instance()->GetIContextManager(array->GetName()),
      nullptr, nullptr)
{
}

void Copier::MakeDebugInfo(DebugCopier& obj)
{
    obj.arrayId = array->GetIndex();
    obj.userDataMaxStripes = userDataMaxStripes;
    obj.userDataMaxBlks = userDataMaxBlks;
    obj.blocksPerChunk = blocksPerChunk;
    obj.victimId = victimId;
    obj.targetId = targetId;
    obj.victimStripeId = victimStripeId;
    obj.copybackState = copybackState;
    obj.invalidBlkCnt = userDataMaxBlks - meta->GetDoneCopyBlks();
    obj.copyDoneCnt = meta->GetDoneCopyBlks();
    SegmentCtx* segmentCtx = iContextManager->GetSegmentCtx();
    obj.numFreeSegment = (uint32_t)(segmentCtx->GetNumOfFreeSegment());
}

Copier::Copier(SegmentId victimId, SegmentId targetId, GcStatus* gcStatus, IArrayInfo* array,
    const PartitionLogicalSize* udSize, CopierMeta* inputMeta,
    IBlockAllocator* inputIBlockAllocator,
    IContextManager* inputIContextManager,
    CallbackSmartPtr inputStripeCopySubmissionPtr, CallbackSmartPtr inputReverseMapLoadCompletionPtr)
: victimId(victimId),
  targetId(targetId),
  victimStripeId(UNMAP_STRIPE),
  copybackState(COPIER_THRESHOLD_CHECK_STATE),
  meta(inputMeta),
  array(array),
  iBlockAllocator(inputIBlockAllocator),
  iContextManager(inputIContextManager),
  gcStatus(gcStatus),
  stripeCopySubmissionPtr(inputStripeCopySubmissionPtr),
  reverseMapLoadCompletionPtr(inputReverseMapLoadCompletionPtr)
{
    userDataMaxStripes = udSize->stripesPerSegment;
    userDataMaxBlks = udSize->blksPerStripe * userDataMaxStripes;
    blocksPerChunk = udSize->blksPerChunk;
    SetEventType(BackendEvent_GC);

    debugCopier.RegisterDebugInfoInstance("GC_Copier_Array" + std::to_string(array->GetIndex()));
    copierQueue.RegisterDebugInfoQueue("History_GC_Copier_Array" + std::to_string(array->GetIndex()), 1000, true);
    RegisterDebugInfoMaker(&debugCopier, &copierQueue);
}

Copier::~Copier(void)
{
    if (nullptr != meta)
    {
        delete meta;
    }
}

bool
Copier::Execute(void)
{
    if (true == isStopped)
    {
        bool ret = _Stop();
        return ret;
    }
    else if (true == isPaused)
    {
        return false;
    }
    else
    {
        switch (copybackState)
        {
            case CopierStateType::COPIER_THRESHOLD_CHECK_STATE:
                _CompareThresholdState();
                break;

            case CopierStateType::COPIER_COPY_PREPARE_STATE:
                _CopyPrepareState();
                break;

            case CopierStateType::COPIER_COPY_COMPLETE_STATE:
            {
                bool ret = _CopyCompleteState();
                return ret;
            }

            case CopierStateType::COPIER_READY_TO_END_STATE:
                return readyToEnd;

            default:
                break;
        }
    }

    return false;
}

bool
Copier::_Stop(void)
{
    if (copybackState > CopierStateType::COPIER_COPY_PREPARE_STATE)
    {
        if (false == _IsAllVictimSegmentCopyDone())
        {
            return false;
        }
    }

    GcStripeManager* gcStripeManager = meta->GetGcStripeManager();
    if (false == gcStripeManager->IsAllFinished())
    {
        return false;
    }

    isStopped = false;
    _ChangeEventState(CopierStateType::COPIER_READY_TO_END_STATE);

    return false;
}

void
Copier::_InitVariables(void)
{
    AddDebugInfo();
    gcStatus->SetCopyInfo(false /*started*/, victimId,
        0 /*invalid cnt*/, 0 /*copy cnt*/);
}

void
Copier::_CompareThresholdState(void)
{
    uint64_t objAddr = reinterpret_cast<uint64_t>(this);
    SegmentCtx* segmentCtx = iContextManager->GetSegmentCtx();
    GcCtx* gcCtx = iContextManager->GetGcCtx();
    GcMode gcMode = gcCtx->GetCurrentGcMode();
    uint32_t arrayId = array->GetIndex();

    _CleanUpVictimSegments();

    if ((false == thresholdCheck) || (gcMode != MODE_NO_GC))
    {
        meta->GetGcStripeManager()->CheckTimeout();
        const uint32_t gcBusyThreshold = 20;
        uint32_t victimCnt = segmentCtx->GetVictimSegmentCount();
        uint32_t numFreeSegments = (uint32_t)segmentCtx->GetNumOfFreeSegment();
        uint32_t urgentThreshold = gcCtx->GetUrgentThreshold();
        uint32_t normalThreshold = gcCtx->GetNormalGcThreshold();
        if(victimCnt > gcBusyThreshold)
        {
            gcBusyRetryCnt++;
            if (gcBusyRetryCnt % 10000 == 0)
            {
                POS_TRACE_WARN(EID(GC_CONGESTED_VICTIM_SELECTION),
                    "victim_count:{}, free_segment_count:{}, urgent_threshold:{}, array_id:{}, gc_busy_retried:{}",
                    victimCnt, numFreeSegments, urgentThreshold, arrayId, gcBusyRetryCnt);
            }
            else if (gcBusyRetryCnt % 100 == 0)
            {
                POS_TRACE_DEBUG(EID(GC_RETRY_VICTIM_SELECTION),
                    "victim_count:{}, free_segment_count:{}, urgent_threshold:{}, array_id:{}, gc_busy_retried:{}",
                    victimCnt, numFreeSegments, urgentThreshold, arrayId, gcBusyRetryCnt);
            }
            return;
        }
        airlog("LAT_GetVictimSegment", "begin", 0, objAddr);
        victimId = iContextManager->AllocateGCVictimSegment();
        airlog("LAT_GetVictimSegment", "end", 0, objAddr);

        if (UNMAP_SEGMENT != victimId)
        {
            _InitVariables();
            _ChangeEventState(CopierStateType::COPIER_COPY_PREPARE_STATE);

            bool isUrgent = (numFreeSegments <= urgentThreshold);
            if (isUrgent == true)
            {
                POS_TRACE_WARN(EID(GC_VICTIM_SELECTED), "victim_segment_id:{}, free_segment_count:{}, urgent_threshold:{}, normal_threshold:{}, array_id:{}",
                    victimId, numFreeSegments, urgentThreshold, normalThreshold, arrayId);
            }
            else
            {
                POS_TRACE_DEBUG(EID(GC_VICTIM_SELECTED), "victim_segment_id:{}, free_segment_count:{}, urgent_threshold:{}, normal_threshold:{}, array_id:{}",
                    victimId, numFreeSegments, urgentThreshold, normalThreshold, arrayId);
            }
        }
    }
    gcBusyRetryCnt = 0;
}

void
Copier::_CopyPrepareState(void)
{
    uint32_t victimSegmentIndex = meta->SetInUseBitmap();
    uint32_t victimIndex = victimSegmentIndex;
    StripeId baseStripe = victimId * userDataMaxStripes;

    CallbackSmartPtr callee;
    if (nullptr == stripeCopySubmissionPtr)
    {
        callee = std::make_shared<StripeCopySubmission>(baseStripe, meta, victimIndex);
    }
    else
    {
        callee = stripeCopySubmissionPtr;
    }
    callee->SetWaitingCount(userDataMaxStripes);

    for (uint32_t index = 0; index < userDataMaxStripes; index++)
    {
        CallbackSmartPtr callback;
        if (nullptr == reverseMapLoadCompletionPtr)
        {
            callback = std::make_shared<ReverseMapLoadCompletion>();
        }
        else
        {
            callback = reverseMapLoadCompletionPtr;
        }
        callback->SetCallee(callee);
        meta->GetVictimStripe(victimIndex, index)->Load(baseStripe + index, callback);
    }
    _ChangeEventState(CopierStateType::COPIER_COPY_COMPLETE_STATE);
}

bool
Copier::_CopyCompleteState(void)
{
    bool ret = _IsSynchronized();
    if (false == ret)
    {
        return false;
    }

    POS_TRACE_DEBUG(EID(GC_COPY_COMPLETION), "victim_segment_id:{}", victimId);

    uint32_t invalidBlkCnt = userDataMaxBlks - meta->GetDoneCopyBlks();

    AddDebugInfo();
    gcStatus->SetCopyInfo(true /*started*/, victimId,
        invalidBlkCnt /*invalid cnt*/, meta->GetDoneCopyBlks() /*copy cnt*/);

    _ChangeEventState(CopierStateType::COPIER_THRESHOLD_CHECK_STATE);

    if (false == thresholdCheck)
    {
        thresholdCheck = true;
    }

    return false;
}

bool
Copier::_IsSynchronized(void)
{
    bool ret = meta->IsSynchronized();
    if (false == ret)
    {
        if (meta->IsCopyDone() == false)
        {
            ret = false;
        }
        else
        {
            ret = true;
        }
    }

    return ret;
}

bool
Copier::_IsAllVictimSegmentCopyDone(void)
{
    bool ret = meta->IsAllVictimSegmentCopyDone();

    return ret;
}

void
Copier::_CleanUpVictimSegments(void)
{
    // Clean up previous victim lists
    SegmentCtx* segmentCtx = iContextManager->GetSegmentCtx();
    std::set<SegmentId> victimSegments = segmentCtx->GetVictimSegmentList();
    for (auto victimSegId : victimSegments)
    {
        uint32_t validCount = segmentCtx->GetValidBlockCount(victimSegId);
        if (0 == validCount && UNMAP_SEGMENT != victimSegId)
        {
            // Push to free list among the victim lists
            segmentCtx->MoveToFreeState(victimSegId);
            SegmentContextUpdater* segmentCtxUpdater = (SegmentContextUpdater*)iContextManager->GetSegmentContextUpdaterPtr();
            segmentCtxUpdater->ResetInfos(victimSegId);
            POS_TRACE_INFO(EID(GC_RELEASE_VICTIM_SEGMENT),
                "victim_segment_id:{}, count:{}", victimSegId, validCount);
        }
    }
}

} // namespace pos
