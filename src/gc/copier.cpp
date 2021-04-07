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

#include "src/array/array.h"
#include "src/gc/copier_read_completion.h"
#include "src/gc/reverse_map_load_completion.h"
#include "src/gc/stripe_copy_submission.h"
#include "src/io/general_io/io_submit_handler.h"
#include "src/logger/logger.h"

namespace ibofos
{
Copier::Copier(SegmentId victimId, SegmentId targetId, GcStatus* gcStatus)
: currentStripeOffset(0),
  victimId(victimId),
  targetId(targetId),
  victimStripeId(UNMAP_STRIPE),
  copybackState(COPIER_THRESHOLD_CHECK_STATE),
  meta(nullptr),
  allocator(AllocatorSingleton::Instance()),
  gcStatus(gcStatus),
  loadedValidBlock(false)
{
    const PartitionLogicalSize* udSize;
    udSize = ArraySingleton::Instance()->GetSizeInfo(PartitionType::USER_DATA);
    userDataMaxStripes = udSize->stripesPerSegment;
    userDataMaxBlks = udSize->blksPerStripe * userDataMaxStripes;
    blocksPerChunk = udSize->blksPerChunk;

    meta = new CopierMeta(udSize->chunksPerStripe, CHUNK_SIZE);

    victimStripe = new VictimStripe[userDataMaxStripes];

#if defined QOS_ENABLED_BE
    SetEventType(BackendEvent_GC);
#endif
}

Copier::~Copier(void)
{
    delete meta;
    delete[] victimStripe;
}

bool
Copier::Execute(void)
{
    if (true == isStop)
    {
        bool ret = _Stop();
        return ret;
    }
    else if (true == isPause)
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
        for (uint32_t index = 0; index < userDataMaxStripes; index++)
        {
            if (0 != victimStripe[index].IsAsyncIoDone())
            {
                return false;
            }
        }

        if (false == _Sync())
        {
            return false;
        }
    }

    isStop = false;
    _ChangeEventState(CopierStateType::COPIER_READY_TO_END_STATE);

    return false;
}

void
Copier::_InitVariables(void)
{
    meta->InitProgressCount();

    currentStripeOffset = 0;

    loadedValidBlock = false;

    gcStatus->SetCopyInfo(false /*started*/, victimId,
        0 /*invalid cnt*/, 0 /*copy cnt*/);
}

void
Copier::_CompareThresholdState(void)
{
    uint32_t freeSegments = allocator->GetNumOfFreeUserDataSegment();

    if ((false == thresholdCheck) || (allocator->GetGcThreshold() >= freeSegments))
    {
        victimId = allocator->GetMostInvalidatedSegment();
        if (UNMAP_SEGMENT != victimId)
        {
            _InitVariables();
            _ChangeEventState(CopierStateType::COPIER_COPY_PREPARE_STATE);

            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::GC_GET_VICTIM_SEGMENT,
                "trigger start, cnt:{}, victimId:{}",
                freeSegments, victimId);

            if (allocator->GetUrgentThreshold() < freeSegments)
            {
                allocator->SetBlockingSegmentAllocation(false);
            }
        }
    }
}

void
Copier::_CopyPrepareState(void)
{
    StripeId baseStripe = victimId * userDataMaxStripes;

    loadedValidBlock = false;
    validStripes.clear();
    CallbackSmartPtr callee(
        new StripeCopySubmission(baseStripe,
            victimStripe,
            meta));
    callee->SetWaitingCount(userDataMaxStripes);

    for (uint32_t index = 0; index < userDataMaxStripes; index++)
    {
        CallbackSmartPtr callback(new ReverseMapLoadCompletion());
        callback->SetCallee(callee);

        victimStripe[index].Load(baseStripe + index, callback);
    }

    _ChangeEventState(CopierStateType::COPIER_COPY_COMPLETE_STATE);
}

bool
Copier::_CopyCompleteState(void)
{
    bool ret = _Sync();
    if (false == ret)
    {
        return false;
    }

    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::GC_COPY_COMPLETE,
        "copy complete, id:{}", victimId);

    uint32_t invalidBlkCnt = userDataMaxBlks - meta->GetDoneCopyBlks();

    gcStatus->SetCopyInfo(true /*started*/, victimId,
        invalidBlkCnt /*invalid cnt*/, meta->GetDoneCopyBlks() /*copy cnt*/);

    _ReleaseSegment();
    _ChangeEventState(CopierStateType::COPIER_THRESHOLD_CHECK_STATE);

    if (false == thresholdCheck)
    {
        thresholdCheck = true;
    }

    return false;
}

bool
Copier::_Sync(void)
{
    bool ret = meta->IsSync();

    return ret;
}

int
Copier::_ReleaseSegment(void)
{
    allocator->FreeUserDataSegmentId(victimId);
    return 0;
}

} // namespace ibofos
