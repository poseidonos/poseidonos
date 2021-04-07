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
#pragma once

#include "mfs_io_q.h"
#include "mfs_io_scheduler.h"
#include "mfs_mim_top.h"

class MetaIoMgr;
extern MetaIoMgr metaIoMgr;
using MetaIoReqHandler = IBOF_EVENT_ID (MetaIoMgr::*)(MetaFsIoReqMsg& reqMsg);

class MetaIoMgr : public MetaFsMIMTopMgrClass
{
public:
    MetaIoMgr(void);
    virtual ~MetaIoMgr(void);
    static MetaIoMgr& GetInstance(void);

    virtual void Init(void) override;
    virtual bool Bringup(void) override;
    virtual void Close(void) override;
    virtual IBOF_EVENT_ID ProcessNewReq(MetaFsIoReqMsg& reqMsg) override;
    virtual void SetMDpageEpochSignature(uint64_t mbrEpochSignature) override;
    uint64_t GetMDpageEpochSignature(void); // FIXME: Need to move other place
    void Finalize(void);

private:
    void _InitReqHandler(void);
    void _PrepareIoThreads(void);

    ScalableMetaIoWorker* _InitiateMioHandler(int handlerId, int coreId, int coreCount);
    IBOF_EVENT_ID _ProcessNewIoReq(MetaFsIoReqMsg& reqMsg);
    IBOF_EVENT_ID _CheckFileIoBoundary(MetaFsIoReqMsg& reqMsg);
    void _AddExtraIoReqInfo(MetaFsIoReqMsg& reqMsg);
    void _SetByteRangeForFullFileIo(MetaFsIoReqMsg& reqMsg);
    void _SetTargetMediaType(MetaFsIoReqMsg& reqMsg);
    void _WaitForDone(MetaFsIoReqMsg& reqMsg);

    static const uint32_t NUM_IO_TYPE = static_cast<uint32_t>(MetaIoReqTypeEnum::Max);
    MetaIoReqHandler reqHandler[NUM_IO_TYPE];
    MetaFsIoScheduler* ioScheduler;

    uint64_t epochSignature;
    uint32_t totalMetaIoCoreCnt;
    uint32_t mioHandlerCount;
    bool finalized;
};
