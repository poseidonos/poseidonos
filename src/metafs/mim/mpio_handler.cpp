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
MpioHandler::MpioHandler(int threadId, int coreId, TelemetryPublisher* tp, MetaFsIoMultilevelQ<Mpio*, RequestPriority>* doneQ)
: partialMpioDoneQ(doneQ),
  mpioAllocator(nullptr),
  coreId(coreId),
  telemetryPublisher(tp),
  metricSumOfSpendTime(0),
  metricSumOfMpioCount(0)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "threadId={}, coreId={}", threadId, coreId);

    if (nullptr == doneQ)
        partialMpioDoneQ = new MetaFsIoMultilevelQ<Mpio*, RequestPriority>();

    lastTime = std::chrono::steady_clock::now();
}

MpioHandler::~MpioHandler(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "MpioHandler is desctructed");

    if (nullptr != partialMpioDoneQ)
        delete partialMpioDoneQ;
}

void
MpioHandler::EnqueuePartialMpio(Mpio* mpio)
{
    partialMpioDoneQ->Enqueue(mpio, mpio->GetPriority());
    mpio->StoreTimestamp(MpioTimestampStage::Enqueue);
}

void
MpioHandler::BindMpioAllocator(MpioAllocator* mpioAllocator)
{
    assert(this->mpioAllocator == nullptr && mpioAllocator != nullptr);
    this->mpioAllocator = mpioAllocator;
}

void
MpioHandler::BottomhalfMioProcessing(void)
{
    Mpio* mpio = partialMpioDoneQ->Dequeue();
    if (mpio)
    {
        mpio->StoreTimestamp(MpioTimestampStage::Dequeue);

        mpio->ExecuteAsyncState();

        if (mpio->IsCompleted())
        {
            mpio->StoreTimestamp(MpioTimestampStage::Release);
            metricSumOfSpendTime += mpio->GetElapsedInMilli(MpioTimestampStage::Allocate, MpioTimestampStage::Release).count();
            metricSumOfMpioCount++;
            mpioAllocator->Release(mpio);
        }
    }

    mpioAllocator->TryReleaseTheOldestCache();

    _SendPeriodicMetrics();
}

void
MpioHandler::_SendPeriodicMetrics()
{
    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();

    if (elapsedTime >= MetaFsConfig::INTERVAL_IN_MILLISECOND_FOR_SENDING_METRIC)
    {
        std::string thread_name = to_string(coreId);
        POSMetric metricFreeMpioCnt(TEL40103_METAFS_FREE_MPIO_CNT, POSMetricTypes::MT_GAUGE);
        metricFreeMpioCnt.AddLabel("thread_name", thread_name);
        metricFreeMpioCnt.SetGaugeValue(mpioAllocator->GetFreeCount());
        telemetryPublisher->PublishMetric(metricFreeMpioCnt);

        if (metricSumOfMpioCount != 0)
        {
            POSMetric metricTime(TEL40104_METAFS_SUM_OF_ALL_THE_TIME_SPENT_BY_MPIO, POSMetricTypes::MT_GAUGE);
            metricTime.AddLabel("thread_name", thread_name);
            metricTime.SetGaugeValue(metricSumOfSpendTime);
            telemetryPublisher->PublishMetric(metricTime);

            POSMetric metricCount(TEL40105_METAFS_SUM_OF_MPIO_COUNT, POSMetricTypes::MT_GAUGE);
            metricCount.AddLabel("thread_name", thread_name);
            metricCount.SetGaugeValue(metricSumOfMpioCount);
            telemetryPublisher->PublishMetric(metricCount);

            metricSumOfSpendTime = 0;
            metricSumOfMpioCount = 0;
        }

        lastTime = currentTime;
    }
}
} // namespace pos
