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
 * iBoFOS - Meta Filesystem Layer
 * 
 * Meta File I/O Manager
*/

#include "meta_io_manager.h"

#include "mfs_aio_completer.h"
#include "mfs_log.h"
#include "mfs_mem_lib.h"
#include "mfs_mvm_top.h"
#include "src/io/general_io/affinity_manager.h"

MetaIoMgr metaIoMgr;
MetaFsMIMTopMgrClass& mimTopMgr = metaIoMgr;

MetaIoMgr::MetaIoMgr(void)
: ioScheduler(nullptr),
  epochSignature(0),
  totalMetaIoCoreCnt(0),
  mioHandlerCount(0)
{
    _InitReqHandler();
}

MetaIoMgr::~MetaIoMgr(void)
{
    Finalize();
}

void
MetaIoMgr::_InitReqHandler(void)
{
    reqHandler[static_cast<uint32_t>(MetaIoReqTypeEnum::Read)] = &MetaIoMgr::_ProcessNewIoReq;
    reqHandler[static_cast<uint32_t>(MetaIoReqTypeEnum::Write)] = &MetaIoMgr::_ProcessNewIoReq;

    finalized = false;
}

MetaIoMgr&
MetaIoMgr::GetInstance(void)
{
    return metaIoMgr;
}

void
MetaIoMgr::Init(void)
{
    totalMetaIoCoreCnt =
        ibofos::AffinityManagerSingleton::Instance()->GetCoreCount(
            ibofos::CoreType::META_IO);

    MetaFsSysHwMemLib::Init();

    finalized = false;
    SetModuleInit();
}

bool
MetaIoMgr::Bringup(void)
{
    if (IsModuleReady())
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_MODULE_ALREADY_READY,
            "You attempt to bringup metaIoMgr again. Ignore it");
        return true;
    }
    _PrepareIoThreads();
    SetModuleReady();

    return true;
}

void
MetaIoMgr::Close(void)
{
    if (IsModuleReady())
    {
        Finalize();

        MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Meta Io Mgr has been closed");
    }
}

void
MetaIoMgr::Finalize(void)
{
    if (IsModuleReady() && (finalized == false))
    {
        ioScheduler->ClearHandlerThread(); // exit mioHandler thread
        ioScheduler->ExitThread();         // exit scheduler thread
        ioScheduler->ClearQ();             // clear MultQ

        delete ioScheduler;
        ioScheduler = nullptr;

        MetaFsSysHwMemLib::Finalize();

        finalized = true;
    }
}

void
MetaIoMgr::_PrepareIoThreads(void)
{
    ibofos::AffinityManager& affinityManager =
        *ibofos::AffinityManagerSingleton::Instance();
    cpu_set_t metaIoSchedulerList = affinityManager.GetCpuSet(ibofos::CoreType::META_SCHEDULER);
    cpu_set_t metaIoCoreList = affinityManager.GetCpuSet(ibofos::CoreType::META_IO);
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
MetaIoMgr::_InitiateMioHandler(int handlerId, int coreId, int coreCount)
{
    ScalableMetaIoWorker* mioHandler =
        new ScalableMetaIoWorker(handlerId, coreId, coreCount);
    mioHandler->StartThread();
    ioScheduler->RegisterMioHandler(mioHandler);

    return mioHandler;
}

void
MetaIoMgr::SetMDpageEpochSignature(uint64_t mbrEpochSignature)
{
    epochSignature = mbrEpochSignature;
}

uint64_t
MetaIoMgr::GetMDpageEpochSignature(void)
{
    return epochSignature;
}

IBOF_EVENT_ID
MetaIoMgr::ProcessNewReq(MetaFsIoReqMsg& reqMsg)
{
    IBOF_EVENT_ID rc;

    rc = (this->*(reqHandler[static_cast<uint32_t>(reqMsg.reqType)]))(reqMsg);

    return rc;
}

IBOF_EVENT_ID
MetaIoMgr::_ProcessNewIoReq(MetaFsIoReqMsg& reqMsg)
{
    _AddExtraIoReqInfo(reqMsg);

    IBOF_EVENT_ID rc;
    rc = _CheckFileIoBoundary(reqMsg);
    if (false == IsSuccess(rc))
    {
        MFS_TRACE_ERROR((int)rc, "File I/O boundary error. rc={}", (int)rc);
        return rc;
    }

    // reqMsg     : original message, used only this thread
    // cloneReqMsg: new copy, only for meta scheduler, not meta handler thread
    MetaFsIoReqMsg* cloneReqMsg = new MetaFsIoReqMsg();
    cloneReqMsg->CopyUserReqMsg(reqMsg);

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[MSG ][EnqueueReq ] type={}, req.TagId={}, mediaType={}, io_mode={}, fileOffset={}, Size={}, Lpn={}",
        (int)reqMsg.reqType, (int)reqMsg.tagId, (int)reqMsg.targetMediaType,
        (int)reqMsg.ioMode, reqMsg.byteOffsetInFile, reqMsg.byteSize,
        reqMsg.byteOffsetInFile / MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE);

    bool reqQueued = ioScheduler->EnqueueNewReq(*cloneReqMsg);

    if (!reqQueued)
    {
        MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR,
            "Failed to enqueue new request...");
        return IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
    }

    if (reqMsg.IsSyncIO())
    {
        _WaitForDone(reqMsg);

        int error = reqMsg.GetError();
        if (error)
        {
            MFS_TRACE_ERROR((int)IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR,
                "[MSG ] Sync I/O failed. req.tagId={}, fd={}", reqMsg.tagId, reqMsg.fd);
            rc = IBOF_EVENT_ID::MFS_IO_FAILED_DUE_TO_ERROR;
        }
    }

    return rc;
}

void
MetaIoMgr::_WaitForDone(MetaFsIoReqMsg& reqMsg)
{
    if (false == reqMsg.IsIoCompleted())
    {
        reqMsg.SuspendUntilIoCompletion();
    }
}

void
MetaIoMgr::_SetByteRangeForFullFileIo(MetaFsIoReqMsg& reqMsg)
{
    reqMsg.byteOffsetInFile = 0;

    FileSizeType outfileByteSize;
    IBOF_EVENT_ID ret = mvmTopMgr.GetFileSize(reqMsg.fd, outfileByteSize);
    if (ret == IBOF_EVENT_ID::SUCCESS)
    {
        reqMsg.byteSize = outfileByteSize;
    }
}

void
MetaIoMgr::_SetTargetMediaType(MetaFsIoReqMsg& reqMsg)
{
    reqMsg.targetMediaType = MetaStorageType::Max;
    MetaStorageType outTargetMediaType;
    IBOF_EVENT_ID ret = mvmTopMgr.GetTargetMediaType(reqMsg.fd, outTargetMediaType);
    if (ret == IBOF_EVENT_ID::SUCCESS)
    {
        reqMsg.targetMediaType = outTargetMediaType;
    }
}

IBOF_EVENT_ID
MetaIoMgr::_CheckFileIoBoundary(MetaFsIoReqMsg& reqMsg)
{
    IBOF_EVENT_ID rc = IBOF_EVENT_ID::SUCCESS;
    FileSizeType fileByteSize;
    IBOF_EVENT_ID ret = mvmTopMgr.GetFileSize(reqMsg.fd, fileByteSize);
    if (ret != IBOF_EVENT_ID::SUCCESS)
    {
        return IBOF_EVENT_ID::MFS_INVALID_PARAMETER;
    }

    if (reqMsg.isFullFileIo)
    {
        if (reqMsg.byteOffsetInFile != 0 ||
            reqMsg.byteSize != fileByteSize)
        {
            rc = IBOF_EVENT_ID::MFS_INVALID_PARAMETER;
        }
    }
    else
    {
        if (reqMsg.byteOffsetInFile >= fileByteSize ||
            (reqMsg.byteOffsetInFile + reqMsg.byteSize) > fileByteSize)
        {
            rc = IBOF_EVENT_ID::MFS_INVALID_PARAMETER;
        }
    }

    return rc;
}

void
MetaIoMgr::_AddExtraIoReqInfo(MetaFsIoReqMsg& reqMsg)
{
    if (true == reqMsg.isFullFileIo)
    {
        _SetByteRangeForFullFileIo(reqMsg);
    }
    _SetTargetMediaType(reqMsg);
}
