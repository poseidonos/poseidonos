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
    const int coreCount, MetaFsConfigManager* configManager, TelemetryPublisher* tp)
: MetaFsIoHandlerBase(threadId, coreId)
{
    telemetryPublisher = tp;
    if (nullptr == telemetryPublisher)
    {
        telemetryPublisher = new TelemetryPublisher("metafs_io_" + to_string(coreId));
        needToDeleteTelemetryPublisher = true;
    }
    TelemetryClientSingleton::Instance()->RegisterPublisher(telemetryPublisher);

    tophalfHandler = new MioHandler(threadId, coreId, coreCount, configManager,
        telemetryPublisher);
    bottomhalfHandler = new MpioHandler(threadId, coreId, configManager,
        telemetryPublisher);

    tophalfHandler->BindPartialMpioHandler(bottomhalfHandler);
}

ScalableMetaIoWorker::~ScalableMetaIoWorker(void)
{
    if (nullptr != telemetryPublisher)
    {
        TelemetryClientSingleton::Instance()->DeregisterPublisher(telemetryPublisher->GetName());

        if (true == needToDeleteTelemetryPublisher)
        {
            delete telemetryPublisher;
            telemetryPublisher = nullptr;
        }
    }

    delete tophalfHandler;
    delete bottomhalfHandler;
}

void
ScalableMetaIoWorker::StartThread(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "mio_handler:: threadId={}, coreId={}",
        threadId, coreId);

    th = new std::thread(AsEntryPointNoParam(&ScalableMetaIoWorker::Execute, this));

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Thread(metafs-mio-handler) joined. thread id={}",
        std::hash<std::thread::id>{}(th->get_id()));
}

bool
ScalableMetaIoWorker::AddArrayInfo(int arrayId)
{
    return tophalfHandler->AddArrayInfo(arrayId);
}

bool
ScalableMetaIoWorker::RemoveArrayInfo(int arrayId)
{
    return tophalfHandler->RemoveArrayInfo(arrayId);
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

void
ScalableMetaIoWorker::EnqueueNewReq(MetaFsIoRequest* reqMsg)
{
    tophalfHandler->EnqueueNewReq(reqMsg);
}
} // namespace pos
