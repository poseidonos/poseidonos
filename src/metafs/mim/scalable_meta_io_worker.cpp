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

#include "scalable_meta_io_worker.h"

#include "src/metafs/include/metafs_service.h"

namespace pos
{
ScalableMetaIoWorker::ScalableMetaIoWorker(const int threadId, const int coreId,
    const std::string& threadName, MetaFsConfigManager* config,
    TelemetryPublisher* tp)
: MetaFsIoHandlerBase(threadId, coreId, threadName),
  topHalf_(nullptr),
  bottomHalf_(nullptr),
  tp_(tp),
  needToDeleteTp_(false)
{
    if (!tp_)
    {
        tp_ = new TelemetryPublisher("metafs_io_" + to_string(coreId_));
        TelemetryClientSingleton::Instance()->RegisterPublisher(tp_);
        needToDeleteTp_ = true;
    }

    topHalf_ = new MioHandler(threadId_, coreId_, config, tp_);
    bottomHalf_ = new MpioHandler(threadId_, coreId_, config, tp_);

    topHalf_->BindPartialMpioHandler(bottomHalf_);
}

ScalableMetaIoWorker::~ScalableMetaIoWorker(void)
{
    if (needToDeleteTp_)
    {
        TelemetryClientSingleton::Instance()->DeregisterPublisher(tp_->GetName());

        delete tp_;
        tp_ = nullptr;
    }

    delete topHalf_;
    delete bottomHalf_;
}

void
ScalableMetaIoWorker::StartThread(void)
{
    th_ = new std::thread(AsEntryPointNoParam(&ScalableMetaIoWorker::Execute, this));

    POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "Start MioHandler, " + GetLogString() + ", thread id: {}",
        std::hash<std::thread::id>{}(th_->get_id()));
}

bool
ScalableMetaIoWorker::AddArrayInfo(const int arrayId)
{
    return topHalf_->AddArrayInfo(arrayId);
}

bool
ScalableMetaIoWorker::RemoveArrayInfo(const int arrayId)
{
    return topHalf_->RemoveArrayInfo(arrayId);
}

void
ScalableMetaIoWorker::Execute(void)
{
    PrepareThread();

    while (!threadExit_)
    {
        topHalf_->TophalfMioProcessing();
        bottomHalf_->BottomhalfMioProcessing();
    }
}

void
ScalableMetaIoWorker::EnqueueNewReq(MetaFsIoRequest* reqMsg)
{
    topHalf_->EnqueueNewReq(reqMsg);
}
} // namespace pos
