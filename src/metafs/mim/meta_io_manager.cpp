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

/* 
 * PoseidonOS - Meta Filesystem Layer
 * 
 * Meta File I/O Manager
*/

#include <string>
#include "metafs_mbr_mgr.h"
#include "meta_io_manager.h"
#include "metafs_aio_completer.h"
#include "metafs_log.h"
#include "metafs_mem_lib.h"
#include "metafs_control_request.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/logger/logger.h"

namespace pos
{
MetaIoManager metaIoMgr;
MetaIoManager& mimTopMgr = metaIoMgr;

MetaIoManager::MetaIoManager(void)
: ioScheduler(nullptr),
  totalMetaIoCoreCnt(0),
  mioHandlerCount(0)
{
    _InitReqHandler();
}

MetaIoManager::~MetaIoManager(void)
{
    Finalize();
}

const char*
MetaIoManager::GetModuleName(void)
{
    return "Meta IO Manager";
}

POS_EVENT_ID
MetaIoManager::CheckReqSanity(MetaFsIoRequest& reqMsg)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;

    if (false == reqMsg.IsValid())
    {
        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    POS_EVENT_ID mgmtSC;
    mgmtSC = mvmTopMgr.CheckFileAccessible(reqMsg.fd, reqMsg.arrayName);
    if (POS_EVENT_ID::SUCCESS != mgmtSC)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_NOT_FOUND,
            "File not found...(given fd={})", reqMsg.fd);
        return POS_EVENT_ID::MFS_FILE_NOT_FOUND;
    }

    switch (reqMsg.reqType)
    {
        case MetaIoRequestType::Read: // go thru
        case MetaIoRequestType::Write:
        {
            if (MetaIoMode::Async == reqMsg.ioMode)
            {
                rc = _CheckAIOReqSanity(reqMsg);
                if (!IsSuccess(rc))
                {
                    return rc;
                }
            }
        }
        break;
        default:
        {
            MFS_TRACE_CRITICAL((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
                "MetaIoManager::CheckReqSanity - Invalid OPcode");
            assert(false);
        }
    }
    return rc;
}

POS_EVENT_ID
MetaIoManager::_CheckAIOReqSanity(MetaFsIoRequest& reqMsg)
{
    return POS_EVENT_ID::SUCCESS;
}

bool
MetaIoManager::_IsSiblingModuleReady(void)
{
    // explicitly check. better to know which friends are associated to MIM
    if (false == mvmTopMgr.IsModuleReady())
    {
        return false;
    }
    // any other friends?

    return true;
}

bool
MetaIoManager::IsSuccess(POS_EVENT_ID rc)
{
    return rc == POS_EVENT_ID::SUCCESS;
}

void
MetaIoManager::_InitReqHandler(void)
{
    reqHandler[static_cast<uint32_t>(MetaIoRequestType::Read)] = &MetaIoManager::_ProcessNewIoReq;
    reqHandler[static_cast<uint32_t>(MetaIoRequestType::Write)] = &MetaIoManager::_ProcessNewIoReq;

    finalized = false;
}

MetaIoManager&
MetaIoManager::GetInstance(void)
{
    return metaIoMgr;
}

void
MetaIoManager::Init(void)
{
    totalMetaIoCoreCnt =
        pos::AffinityManagerSingleton::Instance()->GetCoreCount(
            pos::CoreType::META_IO);

    MetaFsMemLib::Init();

    finalized = false;
    SetModuleInit();
}

bool
MetaIoManager::Bringup(void)
{
    if (IsModuleReady())
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_MODULE_ALREADY_READY,
            "You attempt to bringup metaIoMgr again. Ignore it");
        return true;
    }
    _PrepareIoThreads();
    SetModuleReady();

    return true;
}

void
MetaIoManager::Close(void)
{
    if (IsModuleReady())
    {
        Finalize();

        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Meta Io Mgr has been closed");
    }
}

void
MetaIoManager::Finalize(void)
{
    if (IsModuleReady() && (finalized == false))
    {
        ioScheduler->ClearHandlerThread(); // exit mioHandler thread
        ioScheduler->ExitThread();         // exit scheduler thread
        ioScheduler->ClearQ();             // clear MultQ

        delete ioScheduler;
        ioScheduler = nullptr;

        MetaFsMemLib::Finalize();

        finalized = true;
    }
}

bool
MetaIoManager::AddArrayInfo(std::string arrayName)
{
    return ioScheduler->AddArrayInfo(arrayName);
}

bool
MetaIoManager::RemoveArrayInfo(std::string arrayName)
{
    return ioScheduler->RemoveArrayInfo(arrayName);
}

void
MetaIoManager::_PrepareIoThreads(void)
{
    pos::AffinityManager& affinityManager =
        *pos::AffinityManagerSingleton::Instance();
    cpu_set_t metaIoSchedulerList = affinityManager.GetCpuSet(pos::CoreType::META_SCHEDULER);
    cpu_set_t metaIoCoreList = affinityManager.GetCpuSet(pos::CoreType::META_IO);
    uint32_t maxCoreCount = affinityManager.GetTotalCore();
    uint32_t availableMetaIoCoreCnt = CPU_COUNT(&metaIoCoreList);
    uint32_t handlerId = 0;
    uint32_t numCPUsInSystem = std::thread::hardware_concurrency();

    // meta io scheduler
    for (uint32_t coreId = 0; coreId < numCPUsInSystem; ++coreId)
    {
        if (CPU_ISSET(coreId, &metaIoSchedulerList))
        {
            ioScheduler = new MetaFsIoScheduler(0, coreId, maxCoreCount);
            ioScheduler->StartThread();
            break;
        }
    }

    // meta io handler
    for (uint32_t coreId = 0; coreId < numCPUsInSystem; ++coreId)
    {
        if (CPU_ISSET(coreId, &metaIoCoreList))
        {
            _InitiateMioHandler(handlerId++, coreId, maxCoreCount);
            availableMetaIoCoreCnt--;

            if (availableMetaIoCoreCnt == 0)
            {
                break;
            }
        }
    }

    mioHandlerCount = ioScheduler->GetMioHandlerCount();
}

ScalableMetaIoWorker*
MetaIoManager::_InitiateMioHandler(int handlerId, int coreId, int coreCount)
{
    ScalableMetaIoWorker* mioHandler =
        new ScalableMetaIoWorker(handlerId, coreId, coreCount);
    mioHandler->StartThread();
    ioScheduler->RegisterMioHandler(mioHandler);

    return mioHandler;
}

POS_EVENT_ID
MetaIoManager::ProcessNewReq(MetaFsIoRequest& reqMsg)
{
    POS_EVENT_ID rc;

    rc = (this->*(reqHandler[static_cast<uint32_t>(reqMsg.reqType)]))(reqMsg);

    return rc;
}

POS_EVENT_ID
MetaIoManager::_ProcessNewIoReq(MetaFsIoRequest& reqMsg)
{
    _AddExtraIoReqInfo(reqMsg);

    POS_EVENT_ID rc;
    rc = _CheckFileIoBoundary(reqMsg);
    if (false == IsSuccess(rc))
    {
        MFS_TRACE_ERROR((int)rc, "File I/O boundary error. rc={}, offset={}, size={}",
            (int)rc, reqMsg.byteOffsetInFile, reqMsg.byteSize);
        return rc;
    }

    // reqMsg     : original message, used only this thread
    // cloneReqMsg: new copy, only for meta scheduler, not meta handler thread
    MetaFsIoRequest* cloneReqMsg = new MetaFsIoRequest();
    cloneReqMsg->CopyUserReqMsg(reqMsg);

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[MSG ][EnqueueReq ] type={}, req.TagId={}, mediaType={}, io_mode={}, fileOffset={}, Size={}, Lpn={}",
        (int)reqMsg.reqType, (int)reqMsg.tagId, (int)reqMsg.targetMediaType,
        (int)reqMsg.ioMode, reqMsg.byteOffsetInFile, reqMsg.byteSize,
        reqMsg.byteOffsetInFile / MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE);

    bool reqQueued = ioScheduler->EnqueueNewReq(cloneReqMsg);

    if (!reqQueued)
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR,
            "Failed to enqueue new request...");
        return POS_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
    }

    if (reqMsg.IsSyncIO())
    {
        _WaitForDone(reqMsg);

        int error = reqMsg.GetError();
        if (error)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR,
                "[MSG ] Sync I/O failed. req.tagId={}, fd={}", reqMsg.tagId, reqMsg.fd);
            rc = POS_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
        }
    }

    return rc;
}

void
MetaIoManager::_WaitForDone(MetaFsIoRequest& reqMsg)
{
    if (false == reqMsg.IsIoCompleted())
    {
        reqMsg.SuspendUntilIoCompletion();
    }
}

void
MetaIoManager::_SetByteRangeForFullFileIo(MetaFsIoRequest& reqMsg)
{
    reqMsg.byteOffsetInFile = 0;

    FileSizeType outfileByteSize;
    POS_EVENT_ID ret = mvmTopMgr.GetFileSize(reqMsg.fd, reqMsg.arrayName, outfileByteSize);
    if (ret == POS_EVENT_ID::SUCCESS)
    {
        reqMsg.byteSize = outfileByteSize;
    }
}

void
MetaIoManager::_SetTargetMediaType(MetaFsIoRequest& reqMsg)
{
    reqMsg.targetMediaType = MetaStorageType::Max;
    MetaStorageType outTargetMediaType;
    POS_EVENT_ID ret = mvmTopMgr.GetTargetMediaType(reqMsg.fd, reqMsg.arrayName, outTargetMediaType);
    if (ret == POS_EVENT_ID::SUCCESS)
    {
        reqMsg.targetMediaType = outTargetMediaType;
    }
}

POS_EVENT_ID
MetaIoManager::_CheckFileIoBoundary(MetaFsIoRequest& reqMsg)
{
    POS_EVENT_ID rc = POS_EVENT_ID::SUCCESS;
    FileSizeType fileByteSize;
    POS_EVENT_ID ret = mvmTopMgr.GetFileSize(reqMsg.fd, reqMsg.arrayName, fileByteSize);
    if (ret != POS_EVENT_ID::SUCCESS)
    {
        return POS_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    if (reqMsg.isFullFileIo)
    {
        if (reqMsg.byteOffsetInFile != 0 ||
            reqMsg.byteSize != fileByteSize)
        {
            rc = POS_EVENT_ID::MFS_INVALID_PARAMETER;
        }
    }
    else
    {
        if (reqMsg.byteOffsetInFile >= fileByteSize ||
            (reqMsg.byteOffsetInFile + reqMsg.byteSize) > fileByteSize)
        {
            rc = POS_EVENT_ID::MFS_INVALID_PARAMETER;
        }
    }

    return rc;
}

void
MetaIoManager::_AddExtraIoReqInfo(MetaFsIoRequest& reqMsg)
{
    if (true == reqMsg.isFullFileIo)
    {
        _SetByteRangeForFullFileIo(reqMsg);
    }
    _SetTargetMediaType(reqMsg);
}
} // namespace pos
