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
#include "stripe_based_race_rebuild.h"

#include <typeinfo>

#include "rebuilder.h"
#include "rebuild_completed.h"
#include "update_data_handler.h"
#include "update_data_complete_handler.h"
#include "src/include/array_config.h"
#include "src/include/pos_event_id.h"
#include "src/include/branch_prediction.h"
#include "src/include/backend_event.h"
#include "src/array_models/dto/partition_physical_size.h"
#include "src/array/service/array_service_layer.h"
#include "src/bio/ubio.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/io_completer.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/io/backend_io/rebuild_io/rebuild_read.h"
#include "src/resource_manager/buffer_pool.h"

namespace pos
{
StripeBasedRaceRebuild::StripeBasedRaceRebuild(unique_ptr<RebuildContext> c)
: RebuildBehavior(move(c))
{
    POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG), "StripeBasedRaceRebuild");
    locker = ArrayService::Instance()->Getter()->GetIOLocker(ctx->part);
    assert(locker != nullptr);
}

StripeBasedRaceRebuild::~StripeBasedRaceRebuild(void)
{
}

string
StripeBasedRaceRebuild::_GetClassName(void)
{
    return typeid(this).name();
}

bool
StripeBasedRaceRebuild::Init(void)
{
    if (isInitialized == false)
    {
        bool ret = _InitBuffers();
        if (ret == false)
        {
            if (initRebuildRetryCnt >= INIT_REBUILD_MAX_RETRY)
            {
                POS_TRACE_ERROR(EID(REBUILD_INIT_FAILED), "part_type:{}, retried:{}",
                    PARTITION_TYPE_STR[ctx->part], initRebuildRetryCnt);
                ctx->SetResult(RebuildState::FAIL);
                return Read();
            }
            return false;
        }
        POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG), "StripeBasedRaceRebuild Initialized successfully {}", PARTITION_TYPE_STR[ctx->part]);
        isInitialized = true;
    }
    return Read();
}

bool
StripeBasedRaceRebuild::Read(void)
{
    uint32_t strPerSeg = ctx->size->stripesPerSegment;
    uint32_t maxStripeId = ctx->size->totalSegments * strPerSeg - 1;

    UpdateProgress(baseStripe);

    uint32_t targetIndex = ctx->faultIdx;
    ArrayDevice* targetDev = ctx->faultDev;

    if (baseStripe >= maxStripeId ||
        ctx->GetResult() >= RebuildState::CANCELLED)
    {
        if (locker->ResetBusyLock(targetDev) == false)
        {
            int logInterval = TRY_LOCK_MAX_RETRY / 10;
            if (resetLockRetryCnt % logInterval == 0)
            {
                locker->WriteBusyLog(targetDev);
                POS_TRACE_WARN(EID(REBUILD_DEBUG_MSG),
                    "Partition {} rebuild done, but waiting lock release, retried:{}",
                    PARTITION_TYPE_STR[ctx->part], resetLockRetryCnt);
                int sleep = resetLockRetryCnt * 10;
                usleep(sleep);
            }
            resetLockRetryCnt++;
            if (resetLockRetryCnt >= TRY_LOCK_MAX_RETRY)
            {
                POS_TRACE_ERROR(EID(REBUILD_FORCED_RESET_LOCK), "part:{}, retried:{}",
                    PARTITION_TYPE_STR[ctx->part], resetLockRetryCnt);
                bool forceReset = true;
                locker->ResetBusyLock(targetDev, forceReset);
                resetLockRetryCnt = 0;
                return true;
            }
            return false;
        }
        else
        {
             POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG), "Busy lock successfully reset, rebuild_result:{}", ctx->GetResult());
        }
        if (ctx->GetResult() == RebuildState::CANCELLED)
        {
            POS_TRACE_WARN((int)POS_EVENT_ID::REBUILD_STOPPED,
                "Partition {} ({}) rebuilding stopped",
                PARTITION_TYPE_STR[ctx->part], ctx->raidType.ToString());
        }
        else if (ctx->GetResult() == RebuildState::FAIL)
        {
            POS_TRACE_WARN((int)POS_EVENT_ID::REBUILD_FAILED,
                "Partition {} ({}) rebuilding failed",
                PARTITION_TYPE_STR[ctx->part], ctx->raidType.ToString());
        }
        else
        {
            POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG),
                "Partition {} ({}) rebuilding done",
                PARTITION_TYPE_STR[ctx->part], ctx->raidType.ToString());
            ctx->SetResult(RebuildState::PASS);
            UpdateProgress(ctx->stripeCnt);
        }

        EventSmartPtr complete(new RebuildCompleted(this));
        complete->SetEventType(BackendEvent_MetadataRebuild);
        EventSchedulerSingleton::Instance()->EnqueueEvent(complete);
        baseStripe = 0;
        return true;
    }

    POS_TRACE_DEBUG(EID(REBUILD_DEBUG_MSG),
        "Trying to read in rebuild, {}", PARTITION_TYPE_STR[ctx->part]);
    uint32_t blkCnt = ctx->size->blksPerChunk;
    uint32_t from = baseStripe;
    uint32_t to = baseStripe + strPerSeg - 1;
    if (to > maxStripeId)
    {
        to = maxStripeId;
    }

    uint32_t currWorkload = to - from + 1;

    if (locker->TryBusyLock(targetDev, from, to) == false)
    {
        int logInterval = TRY_LOCK_MAX_RETRY / 10;
        if (tryLockRetryCnt % logInterval == 0)
        {
            locker->WriteBusyLog(targetDev);
            POS_TRACE_WARN(EID(REBUILD_TRY_LOCK_RETRY),
                "Failed to acquire rebuild lock, array_name:{}, part:{}, dev_name:{}, stripe_from:{}, stripe_to:{}, retried:{}",
                ctx->array, PARTITION_TYPE_STR[ctx->part], targetDev->GetName(), from, to, tryLockRetryCnt);
            int sleep = tryLockRetryCnt * 10;
            usleep(sleep);
        }

        tryLockRetryCnt++;
        if (tryLockRetryCnt >= TRY_LOCK_MAX_RETRY)
        {
            locker->WriteBusyLog(targetDev);
            POS_TRACE_ERROR(EID(REBUILD_TRY_LOCK_FAILED),
                "array_name:{}, part:{}, dev_name:{}, stripe_from:{}, stripe_to:{}, retried:{}",
                ctx->array, PARTITION_TYPE_STR[ctx->part], targetDev->GetName(), from, to, tryLockRetryCnt);
            ctx->SetResult(RebuildState::FAIL);
            EventSmartPtr complete(new RebuildCompleted(this));
            complete->SetEventType(BackendEvent_MetadataRebuild);
            EventSchedulerSingleton::Instance()->EnqueueEvent(complete);
            baseStripe = 0;
            return true;
        }
        return false;
    }

    tryLockRetryCnt = 0;
    ctx->taskCnt = currWorkload;
    POS_TRACE_INFO(EID(REBUILD_DEBUG_MSG),
        "Trying to recover in rebuild - from:{}, to:{}", from, to);
    for (uint32_t offset = 0; offset < currWorkload; offset++)
    {
        uint32_t stripeId = baseStripe + offset;
        void* buffer = recovery->GetDestBuffer()->TryGetBuffer();
        assert(buffer != nullptr);

        UbioSmartPtr ubio(new Ubio(buffer, blkCnt * Ubio::UNITS_PER_BLOCK, ctx->arrayIndex));
        ubio->dir = UbioDir::Write;
        FtBlkAddr fta = {.stripeId = stripeId,
            .offset = targetIndex * blkCnt};
        PhysicalBlkAddr addr = ctx->translate(fta);
        ubio->SetPba(addr);
        CallbackSmartPtr callback(new UpdateDataHandler(stripeId, ubio, this));
        callback->SetEventType(BackendEvent_MetadataRebuild);
        ubio->SetEventType(BackendEvent_MetadataRebuild);
        ubio->SetCallback(callback);
        
        int res = recovery->Recover(ubio);
        if (res != 0)
        {
            POS_TRACE_ERROR((int)POS_EVENT_ID::REBUILD_FAILED,
                "Failed to recover stripe {} in Partition {} ({}), maxStripes:{}",
                stripeId, PARTITION_TYPE_STR[ctx->part], ctx->raidType.ToString(), maxStripeId);
            ctx->SetResult(RebuildState::FAIL);
        }
    }
    baseStripe += currWorkload;
    return true;
}

bool StripeBasedRaceRebuild::Write(uint32_t targetId, UbioSmartPtr ubio)
{
    CallbackSmartPtr event(
        new UpdateDataCompleteHandler(targetId, ubio, this));
    event->SetEventType(BackendEvent_MetadataRebuild);
    IODispatcher* ioDisp = IODispatcherSingleton::Instance();

    ubio->ClearCallback();
    ubio->SetCallback(event);

    if (likely(ctx->GetResult() == RebuildState::REBUILDING))
    {
        ioDisp->Submit(ubio);
    }
    else
    {
        IoCompleter ioCompleter(ubio);
        ioCompleter.CompleteUbio(IOErrorType::GENERIC_ERROR, true);
    }

    ubio = nullptr;
    return true;
}

bool StripeBasedRaceRebuild::Complete(uint32_t targetId, UbioSmartPtr ubio)
{
    ArrayDevice* targetDev = ctx->faultDev;
    locker->Unlock(targetDev, targetId);
    recovery->GetDestBuffer()->ReturnBuffer(ubio->GetBuffer());

    uint32_t currentTaskCnt = ctx->taskCnt -= 1;
    if (currentTaskCnt == 0)
    {
        EventSmartPtr nextEvent(new Rebuilder(this));
        nextEvent->SetEventType(BackendEvent_MetadataRebuild);
        EventSchedulerSingleton::Instance()->EnqueueEvent(nextEvent);
    }
    ubio = nullptr;

    return true;
}

void StripeBasedRaceRebuild::UpdateProgress(uint32_t val)
{
    ctx->prog->Update(PARTITION_TYPE_STR[ctx->part], val, ctx->stripeCnt);
}

} // namespace pos
