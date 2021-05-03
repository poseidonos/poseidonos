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

#include "src/gc/copier.h"

#include <list>

#include "Air.h"
#include "src/allocator_service/allocator_service.h"
#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_context_manager.h"
#include "src/gc/copier_read_completion.h"
#include "src/gc/reverse_map_load_completion.h"
#include "src/gc/stripe_copy_submission.h"
#include "src/io/general_io/io_submit_handler.h"
#include "src/include/backend_event.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/event_scheduler.h"

namespace pos
{

Copier::Copier(SegmentId victimId, SegmentId targetId, GcStatus* gcStatus,
               IArrayInfo* array, const PartitionLogicalSize* udSize, CopierMeta* meta_,
               IBlockAllocator* iBlockAllocator_, IContextManager* iContextManager_,
               CallbackSmartPtr stripeCopySubmissionPtr_, CallbackSmartPtr reverseMapLoadCompletionPtr_)
: currentStripeOffset(0),
  victimId(victimId),
  targetId(targetId),
  victimStripeId(UNMAP_STRIPE),
  copybackState(COPIER_THRESHOLD_CHECK_STATE),
  meta(nullptr),
  array(array),
  iBlockAllocator(AllocatorServiceSingleton::Instance()->GetIBlockAllocator(array->GetName())),
  iContextManager(AllocatorServiceSingleton::Instance()->GetIContextManager(array->GetName())),
  gcStatus(gcStatus),
  loadedValidBlock(false),
  stripeCopySubmissionPtr(stripeCopySubmissionPtr_),
  reverseMapLoadCompletionPtr(reverseMapLoadCompletionPtr_)
{
    if (nullptr == udSize)
    {
        udSize = array->GetSizeInfo(PartitionType::USER_DATA);
    }
    userDataMaxStripes = udSize->stripesPerSegment;
    userDataMaxBlks = udSize->blksPerStripe * userDataMaxStripes;
    blocksPerChunk = udSize->blksPerChunk;
    victimIndex = 0;
    if (nullptr == meta)
    {
        meta = new CopierMeta(array);
    }
    if (nullptr == iBlockAllocator)
    {
        iBlockAllocator =  AllocatorServiceSingleton::Instance()->GetIBlockAllocator(array->GetName());
    }
    if (nullptr == iContextManager)
    {
        iContextManager = AllocatorServiceSingleton::Instance()->GetIContextManager(array->GetName());
    }
    SetEventType(BackendEvent_GC);
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
    currentStripeOffset = 0;
    loadedValidBlock = false;
    gcStatus->SetCopyInfo(false /*started*/, victimId,
        0 /*invalid cnt*/, 0 /*copy cnt*/);
}

void
Copier::_CompareThresholdState(void)
{
    CurrentGcMode gcMode = iContextManager->GetCurrentGcMode();
    if ((false == thresholdCheck) || (gcMode != MODE_NO_GC))
    {
        victimId = iContextManager->AllocateGCVictimSegment();
        if (UNMAP_SEGMENT != victimId)
        {
            _InitVariables();
            _ChangeEventState(CopierStateType::COPIER_COPY_PREPARE_STATE);

            POS_TRACE_DEBUG((int)POS_EVENT_ID::GC_GET_VICTIM_SEGMENT,
                "trigger start, cnt:{}, victimId:{}",
                 iContextManager->GetNumFreeSegment(), victimId);

            if (gcMode == MODE_NORMAL_GC)
            {
                iBlockAllocator->PermitUserBlkAlloc();
            }
        }
    }
}

void
Copier::_CopyPrepareState(void)
{
    uint32_t victimSegmentIndex = meta->SetInUseBitmap();
    victimIndex = victimSegmentIndex;

    StripeId baseStripe = victimId * userDataMaxStripes;

    loadedValidBlock = false;
    validStripes.clear();

    CallbackSmartPtr callee = std::make_shared<StripeCopySubmission>(baseStripe, meta, victimIndex);
    callee->SetWaitingCount(userDataMaxStripes);

    for (uint32_t index = 0; index < userDataMaxStripes; index++)
    {
        CallbackSmartPtr callback;
        if (nullptr == reverseMapLoadCompletionPtr)
        {
            callback = std::make_shared<ReverseMapLoadCompletion>();
            callback->SetCallee(callee);
        }
        else
        {
            callback = reverseMapLoadCompletionPtr;
        }
        meta->GetVictimStripe(victimIndex, index)->Load(baseStripe + index, callback);
    }

    _ChangeEventState(CopierStateType::COPIER_COPY_COMPLETE_STATE);
}

bool
Copier::_CopyCompleteState(void)
{
    bool ret = meta->IsSynchronized();
    if (false == ret)
    {
        if (meta->IsCopyDone() == false)
        {
            return false;
        }
    }

    POS_TRACE_DEBUG((int)POS_EVENT_ID::GC_COPY_COMPLETE,
        "copy complete, id:{}", victimId);

    uint32_t invalidBlkCnt = userDataMaxBlks - meta->GetDoneCopyBlks();

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

    return ret;
}

bool
Copier::_IsAllVictimSegmentCopyDone(void)
{
    bool ret = meta->IsAllVictimSegmentCopyDone();

    return ret;
}

} // namespace pos
