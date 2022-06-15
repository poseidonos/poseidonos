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

#include <string>

#include "src/metafs/config/metafs_config_manager.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"

namespace pos
{
MpioHandler::MpioHandler(const int threadId, const int coreId,
    MetaFsConfigManager* configManager, TelemetryPublisher* tp,
    MetaFsIoMultilevelQ<Mpio*, RequestPriority>* doneQ)
: partialMpioDoneQ(doneQ),
  mpioAllocator(nullptr),
  coreId(coreId),
  telemetryPublisher(tp),
  sampledTimeSpentProcessingAllStages(0),
  sampledTimeSpentFromWriteToRelease(0),
  sampledTimeSpentFromPushToPop(0),
  totalProcessedMpioCount(0),
  sampledProcessedMpioCount(0),
  metaFsTimeInterval(configManager->GetTimeIntervalInMillisecondsForMetric()),
  skipCount(0),
  SAMPLING_SKIP_COUNT(configManager->GetSamplingSkipCount())
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "threadId={}, coreId={}", threadId, coreId);

    if (nullptr == doneQ)
        partialMpioDoneQ = new MetaFsIoMultilevelQ<Mpio*, RequestPriority>();
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
    mpio->StoreTimestamp(MpioTimestampStage::PushToDoneQ);
    partialMpioDoneQ->Enqueue(mpio, mpio->GetPriority());
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
        mpio->StoreTimestamp(MpioTimestampStage::PopFromDoneQ);

        mpio->ExecuteAsyncState();

        if (mpio->IsCompleted())
        {
            mpio->StoreTimestamp(MpioTimestampStage::Release);
            _UpdateMetricsConditionally(mpio);
            mpioAllocator->Release(mpio);
        }
    }

    mpioAllocator->TryReleaseTheOldestCache();

    _PublishPeriodicMetrics();
}

void
MpioHandler::_UpdateMetricsConditionally(Mpio* mpio)
{
    totalProcessedMpioCount++;

    if (mpio->GetType() == MpioType::Write)
    {
        if (skipCount != SAMPLING_SKIP_COUNT)
        {
            skipCount++;
        }
        else
        {
            sampledTimeSpentProcessingAllStages += mpio->GetElapsedInMilli(MpioTimestampStage::Allocate, MpioTimestampStage::Release).count();
            sampledTimeSpentFromWriteToRelease += mpio->GetElapsedInMilli(MpioTimestampStage::Write, MpioTimestampStage::Release).count();
            sampledTimeSpentFromPushToPop += mpio->GetElapsedInMilli(MpioTimestampStage::PushToDoneQ, MpioTimestampStage::PopFromDoneQ).count();
            sampledProcessedMpioCount++;
            skipCount = 0;
        }
    }
}

void
MpioHandler::_PublishPeriodicMetrics()
{
    if (telemetryPublisher && metaFsTimeInterval.CheckInterval())
    {
        POSMetricVector* metricVector = new POSMetricVector();

        POSMetric mFreeMpioCount(TEL40300_METAFS_FREE_MPIO_CNT, POSMetricTypes::MT_GAUGE);
        mFreeMpioCount.SetGaugeValue(mpioAllocator->GetFreeCount());
        metricVector->emplace_back(mFreeMpioCount);

        if (totalProcessedMpioCount)
        {
            POSMetric m(TEL40202_METAFS_PROCESSED_MPIO_COUNT, POSMetricTypes::MT_GAUGE);
            m.SetGaugeValue(totalProcessedMpioCount);
            metricVector->emplace_back(m);
            totalProcessedMpioCount = 0;
        }

        if (sampledProcessedMpioCount)
        {
            POSMetric mTimeSpentAllStage(TEL40201_METAFS_MPIO_TIME_SPENT_PROCESSING_ALL_STAGES, POSMetricTypes::MT_GAUGE);
            mTimeSpentAllStage.SetGaugeValue(sampledTimeSpentProcessingAllStages);
            metricVector->emplace_back(mTimeSpentAllStage);

            POSMetric mTimeSpentWriteToRelease(TEL40303_METAFS_MPIO_TIME_FROM_WRITE_TO_RELEASE, POSMetricTypes::MT_GAUGE);
            mTimeSpentWriteToRelease.SetGaugeValue(sampledTimeSpentFromWriteToRelease);
            metricVector->emplace_back(mTimeSpentWriteToRelease);

            POSMetric mTimeSpentPushToPop(TEL40304_METAFS_MPIO_TIME_FROM_PUSH_TO_POP, POSMetricTypes::MT_GAUGE);
            mTimeSpentPushToPop.SetGaugeValue(sampledTimeSpentFromPushToPop);
            metricVector->emplace_back(mTimeSpentPushToPop);

            POSMetric m(TEL40305_METAFS_MPIO_SAMPLED_COUNT, POSMetricTypes::MT_GAUGE);
            m.SetGaugeValue(sampledProcessedMpioCount);
            metricVector->emplace_back(m);

            sampledTimeSpentProcessingAllStages = 0;
            sampledTimeSpentFromPushToPop = 0;
            sampledTimeSpentFromWriteToRelease = 0;
            sampledProcessedMpioCount = 0;
        }

        for (auto& item : *metricVector)
        {
            item.AddLabel("thread_name", std::to_string(coreId));
        }

        telemetryPublisher->PublishMetricList(metricVector);
    }
}
} // namespace pos
