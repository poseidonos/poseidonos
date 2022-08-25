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

/* 
 * PoseidonOS - Meta Filesystem Layer
 * 
 * Meta File I/O Manager
*/

#include "meta_io_manager.h"

#include <numa.h>

#include <string>

#include "metafs_control_request.h"
#include "metafs_log.h"
#include "metafs_mem_lib.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/logger/logger.h"
#include "src/metafs/include/metafs_service.h"
#include "src/metafs/mim/metafs_io_scheduler.h"
#include "src/metafs/msc/metafs_mbr_mgr.h"
#include "src/metafs/storage/mss.h"

namespace pos
{
MetaIoManager::MetaIoManager(const bool supportNumaDedicated, MetaStorageSubsystem* storage)
: MetaIoManager(supportNumaDedicated, MetaFsServiceSingleton::Instance()->GetScheduler(), storage)
{
}

MetaIoManager::MetaIoManager(const bool supportNumaDedicated, const SchedulerMap& ioScheduler,
    MetaStorageSubsystem* storage)
: IS_SINGLE_SCHEDULER(ioScheduler.size() == 1),
  SUPPORT_NUMA_DEDICATED_SCHEDULING(supportNumaDedicated),
  ioScheduler(ioScheduler),
  totalMetaIoCoreCnt(0),
  mioHandlerCount(0),
  finalized(false),
  metaStorage(storage)
{
    _InitReqHandler();
}

MetaIoManager::~MetaIoManager(void)
{
    Finalize();
}

POS_EVENT_ID
MetaIoManager::CheckReqSanity(MetaFsRequestBase& reqMsg)
{
    POS_EVENT_ID rc = EID(SUCCESS);

    return rc;
}

bool
MetaIoManager::IsSuccess(POS_EVENT_ID rc)
{
    return rc == EID(SUCCESS);
}

void
MetaIoManager::_InitReqHandler(void)
{
    reqHandler[static_cast<uint32_t>(MetaIoRequestType::Read)] = &MetaIoManager::_ProcessNewIoReq;
    reqHandler[static_cast<uint32_t>(MetaIoRequestType::Write)] = &MetaIoManager::_ProcessNewIoReq;

    finalized = false;
}

void
MetaIoManager::Init(void)
{
    totalMetaIoCoreCnt =
        pos::AffinityManagerSingleton::Instance()->GetCoreCount(
            pos::CoreType::META_IO);

    MetaFsMemLib::Init();

    finalized = false;
}

void
MetaIoManager::Finalize(void)
{
    if (finalized == false)
    {
        MetaFsMemLib::Finalize();

        finalized = true;
    }
}

bool
MetaIoManager::AddArrayInfo(const int arrayId, const MaxMetaLpnMapPerMetaStorage& map)
{
    bool result = true;
    for (const auto& scheduler : ioScheduler)
    {
        if (!scheduler.second->AddArrayInfo(arrayId, map))
        {
            result = false;
            POS_TRACE_ERROR(EID(MFS_ARRAY_ADD_FAILED),
                "Adding array has been failed, arrayId:{}", arrayId);
        }
        else
        {
            POS_TRACE_INFO(EID(MFS_ARRAY_ADD_SUCCEEDED),
                "Adding array has been succeeded, arrayId:{}", arrayId);
        }
    }
    return result;
}

bool
MetaIoManager::RemoveArrayInfo(int arrayId)
{
    bool result = true;
    for (const auto& scheduler : ioScheduler)
    {
        if (!scheduler.second->RemoveArrayInfo(arrayId))
        {
            result = false;
            POS_TRACE_ERROR(EID(MFS_ARRAY_REMOVE_FAILED),
                "Removing array has been failed, arrayId:{}", arrayId);
        }
        else
        {
            POS_TRACE_INFO(EID(MFS_ARRAY_REMOVE_SUCCEEDED),
                "Removing array has been succeeded, arrayId:{}", arrayId);
        }
    }
    return result;
}

POS_EVENT_ID
MetaIoManager::ProcessNewReq(MetaFsRequestBase& reqMsg)
{
    POS_EVENT_ID rc;
    MetaFsIoRequest* msg = static_cast<MetaFsIoRequest*>(&reqMsg);
    rc = (this->*(reqHandler[(uint32_t)(msg->reqType)]))(*msg);
    return rc;
}

POS_EVENT_ID
MetaIoManager::_ProcessNewIoReq(MetaFsIoRequest& reqMsg)
{
    POS_EVENT_ID rc = EID(SUCCESS);

    // reqMsg     : original message, used only this thread
    // cloneReqMsg: new copy, only for meta scheduler, not meta handler thread
    MetaFsIoRequest* cloneReqMsg = new MetaFsIoRequest(reqMsg);

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "[MSG ][EnqueueReq ] type={}, req.TagId={}, mediaType={}, io_mode={}, fileOffset={}, Size={}, Lpn={}",
        (int)reqMsg.reqType, (int)reqMsg.tagId, (int)reqMsg.targetMediaType,
        (int)reqMsg.ioMode, reqMsg.byteOffsetInFile, reqMsg.byteSize,
        reqMsg.byteOffsetInFile / MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE);

    _IssueToScheduler(cloneReqMsg);

    if (reqMsg.IsSyncIO())
    {
        _WaitForDone(reqMsg);

        int error = reqMsg.GetError();
        if (error)
        {
            MFS_TRACE_ERROR(EID(MFS_IO_FAILED_DUE_TO_ERROR),
                "[MSG ] Sync I/O failed. req.tagId={}, fd={}", reqMsg.tagId, reqMsg.fd);
            rc = EID(MFS_IO_FAILED_DUE_TO_ERROR);
        }
    }

    return rc;
}

void
MetaIoManager::_IssueToScheduler(MetaFsIoRequest* reqMsg)
{
    if (IS_SINGLE_SCHEDULER)
    {
        ioScheduler[0]->EnqueueNewReq(reqMsg);
    }
    else
    {
        uint32_t numaId = SUPPORT_NUMA_DEDICATED_SCHEDULING ? reqMsg->numaId : 0;
        ioScheduler[numaId]->EnqueueNewReq(reqMsg);
    }
}

void
MetaIoManager::_WaitForDone(MetaFsIoRequest& reqMsg)
{
    if (false == reqMsg.IsIoCompleted())
    {
        reqMsg.SuspendUntilIoCompletion();
    }
}
} // namespace pos
