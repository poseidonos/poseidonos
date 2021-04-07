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

#include "scalable_meta_io_worker.h"

ScalableMetaIoWorker::ScalableMetaIoWorker(int threadId, int coreId, int coreCount)
: MetaFsIoHandlerBase(threadId, coreId)
{
    tophalfHandler = new MioHandler(threadId, coreId, coreCount);
    bottomhalfHandler = new MpioHandler(threadId, coreId);

    tophalfHandler->BindPartialMpioHandler(bottomhalfHandler);
}

ScalableMetaIoWorker::~ScalableMetaIoWorker(void)
{
    delete tophalfHandler;
    delete bottomhalfHandler;
}

void
ScalableMetaIoWorker::StartThread(void)
{
    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "mio_handler:: threadId={}, coreId={}",
        threadId, coreId);

    th = new std::thread(AsEntryPointNoParam(&ScalableMetaIoWorker::Execute, this));

    MFS_TRACE_DEBUG((int)IBOF_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Thread(metafs-mio-handler) joined. thread id={}",
        std::hash<std::thread::id>{}(th->get_id()));
}

void
ScalableMetaIoWorker::Execute(void)
{
    PrepareThread("MioHandler");

    while (false == threadExit)
    {
        tophalfHandler->TophalfMioProcessing();
        bottomhalfHandler->BottomhalfMioProcessing();
    }
}

bool
ScalableMetaIoWorker::EnqueueNewReq(MetaFsIoReqMsg& reqMsg)
{
    return tophalfHandler->EnqueueNewReq(reqMsg);
}
