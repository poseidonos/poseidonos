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

#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <vector>

#include "victim_stripe.h"
#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_context_manager.h"
#include "src/gc/copier_meta.h"
#include "src/gc/gc_status.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/event_scheduler/event.h"

namespace pos
{
enum CopierStateType
{
    COPIER_THRESHOLD_CHECK_STATE = 0,
    COPIER_COPY_PREPARE_STATE,
    COPIER_COPY_COMPLETE_STATE,
    COPIER_READY_TO_END_STATE
};

class StripeCopySubmission;
class ReverseMapLoadCompletion;

class Copier : public Event
{
public:
    explicit Copier(SegmentId victimId, SegmentId targetId, GcStatus* gcStatus, IArrayInfo* array);
    Copier(SegmentId victimId, SegmentId targetId, GcStatus* gcStatus, IArrayInfo* array,
            const PartitionLogicalSize* udSize, CopierMeta* inputMeta,
            IBlockAllocator* inputIBlockAllocator,
            IContextManager* inputIContextManager,
            CallbackSmartPtr inputStripeCopySubmissionPtr, CallbackSmartPtr inputReverseMapLoadCompletionPtr);

    virtual ~Copier(void);
    virtual bool Execute(void);

    virtual void
    Stop(void)
    {
        isStopped = true;
    }
    virtual bool
    IsStopped(void)
    {
        return isStopped;
    }
    virtual void
    ReadyToEnd(void)
    {
        readyToEnd = true;
    }
    virtual void
    Pause(void)
    {
        isPaused = true;
    }
    virtual void
    Resume(void)
    {
        isPaused = false;
    }
    virtual bool
    IsPaused(void)
    {
        return isPaused;
    }
    virtual void
    DisableThresholdCheck(void)
    {
        thresholdCheck = false;
    }
    virtual bool
    IsEnableThresholdCheck(void)
    {
        return thresholdCheck;
    }
    virtual CopierStateType
    GetCopybackState(void)
    {
        return copybackState;
    }

private:
    void _CompareThresholdState(void);
    void _CopyPrepareState(void);
    bool _CopyCompleteState(void);

    void _InitVariables(void);
    bool _Stop(void);
    bool _IsSynchronized(void);
    bool _IsAllVictimSegmentCopyDone(void);
    void _CleanUpVictimSegments(void);
    void
    _ChangeEventState(CopierStateType state)
    {
        copybackState = state;
    }

    uint32_t userDataMaxStripes;
    uint32_t userDataMaxBlks;
    uint32_t blocksPerChunk;
    StripeId currentStripeOffset;
    SegmentId victimId;
    SegmentId targetId;
    StripeId victimStripeId;
    uint32_t victimIndex;

    CopierStateType copybackState;
    CopierMeta* meta;

    IArrayInfo* array;
    IBlockAllocator* iBlockAllocator;
    IContextManager* iContextManager;
    GcStatus* gcStatus;
    std::map<uint32_t, BlkInfo> validStripes;
    bool loadedValidBlock;

    bool thresholdCheck = true;
    bool isStopped = false;
    bool isPaused = false;
    bool readyToEnd = false;

    CallbackSmartPtr stripeCopySubmissionPtr;
    CallbackSmartPtr reverseMapLoadCompletionPtr;
};

} // namespace pos
