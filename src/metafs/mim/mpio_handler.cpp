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

#include "mpio_handler.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"

#include <string>

namespace pos
{
MpioHandler::MpioHandler(int threadId, int coreId, TelemetryPublisher* tp, MetaFsIoQ<Mpio*>* doneQ)
: partialMpioDoneQ(doneQ),
  mpioPool(nullptr),
  coreId(coreId),
  telemetryPublisher(tp)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "threadId={}, coreId={}", threadId, coreId);

    if (nullptr == doneQ)
        partialMpioDoneQ = new MetaFsIoQ<Mpio*>();

    nameForTelemetry = "metafs_mpio_" + to_string(coreId);
    if (nullptr == telemetryPublisher)
        telemetryPublisher = new TelemetryPublisher(nameForTelemetry);
    TelemetryClientSingleton::Instance()->RegisterPublisher(nameForTelemetry, telemetryPublisher);
    lastTime = std::chrono::steady_clock::now();
}

MpioHandler::~MpioHandler(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "MpioHandler is desctructed");

    if (nullptr != partialMpioDoneQ)
        delete partialMpioDoneQ;

    if (nullptr != telemetryPublisher)
        TelemetryClientSingleton::Instance()->DeregisterPublisher(nameForTelemetry);
}

void
MpioHandler::EnqueuePartialMpio(Mpio* mpio)
{
    partialMpioDoneQ->Enqueue(mpio);
}

void
MpioHandler::BindMpioPool(MpioPool* mpioPool)
{
    assert(this->mpioPool == nullptr && mpioPool != nullptr);
    this->mpioPool = mpioPool;

    _InitPartialMpioDoneQ(mpioPool->GetPoolSize());
}

void
MpioHandler::_InitPartialMpioDoneQ(size_t mpioDoneQSize)
{
    std::string partialqName("PartialMpioDQ = " + std::to_string(coreId));
    partialMpioDoneQ->Init(partialqName.c_str(), mpioDoneQSize);
}

void
MpioHandler::BottomhalfMioProcessing(void)
{
    Mpio* mpio = partialMpioDoneQ->Dequeue();
    if (mpio)
    {
        mpio->ExecuteAsyncState();

        if (mpio->IsCompleted())
        {
            mpioPool->Release(mpio);
        }

        _SendMetric(partialMpioDoneQ->GetItemCnt());
    }

#if MPIO_CACHE_EN
    mpioPool->ReleaseCache();
#endif
}

void
MpioHandler::_SendMetric(uint32_t size)
{
    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();

    if (elapsedTime >= MetaFsConfig::INTERVAL_IN_MILLISECOND_FOR_SENDING_METRIC)
    {
        POSMetric metric(TEL40101_METAFS_PENDING_MPIO_CNT, POSMetricTypes::MT_COUNT);
        metric.AddLabel("thread_name", nameForTelemetry);
        metric.SetCountValue(size);
        telemetryPublisher->PublishMetric(metric);
        lastTime = currentTime;
    }
}
} // namespace pos
